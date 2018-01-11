#include "bent.h"
#include "generator.h"
#include "log.h"

int AssembleX86(Bent *m) {
	HOOY *h;

	/*
	memcpy(mz, edit_mz, sizeof(IMAGE_DOS_HEADER));
	memcpy(pe, edit_pe, sizeof(IMAGE_NT_HEADERS32));
	memcpy(oe, edit_oe, sizeof(IMAGE_SECTION_HEADER) * pe->pe_numofobjects);
	*/

	size_t maxRVA = m->size + 3 + m->nrva;
	HOOY **fasthooy = (HOOY**)malloc(maxRVA * sizeof(HOOY*));

	if (fasthooy == NULL) {
		m->errorCount++;
		return 1;
	}

re:
	// recalculate addresses
	memset(fasthooy, 0x00, maxRVA * sizeof(HOOY*));

	DWORD v = 0, p = 0;

	ForEachInList(m->hooyList, HOOY, h) {
		if (h->flags & FL_LABEL) {
			fasthooy[h->oldrva] = h;
		}

		/*if ((h->flags & FL_FALIGN) {
		// TODO: pad/unpad correctly
		//  would probably be best to remove all padding, then just repad
		v = ALIGN(v, 16);
		p = ALIGN(p, 16);
		}*/

		h->newrva = v;
		h->newofs = p;

		if (h->flags & FL_SECTALIGN) {
			v = ALIGN(v, m->nt->OptionalHeader.SectionAlignment);
			p = ALIGN(p, m->nt->OptionalHeader.FileAlignment);
		} else {
			if (h->flags & FL_VPRESENT) {
				v += h->datalen;
			}

			if (h->flags & FL_PRESENT) {
				p += h->datalen;
			}
		}
	}

	int hasFixups;
	if (m->nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress) {
		hasFixups = 1;
	} else {
		hasFixups = 0;
	}

	HOOY *fxrva, *fxsize;

	if (hasFixups) {
		fxrva = m->GetHOOYByOldRVA(m->nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress, FL_DATA);

		if (fxrva == NULL) {
			m->errorCount++;
			return 4;
		}

		//fxsize = m->GetHOOYByOldRVA(m->mz->e_lfanew + 0xA4, FL_DELTA); // A4=fixupsize
		fxsize = m->GetHOOYByOldRVA(m->mz->e_lfanew + 0xA4, FL_ALL); // A4=fixupsize

		if (fxsize == NULL) {
			m->errorCount++;
			return 3;
		}

		DWORD numBases = 0, numFixups = 0, fixBase = 0;
		m->fixups.clear();

		ForEachInList(m-> hooyList, HOOY, h) {
			if (h->flags & FL_FIXUP) {
				DWORD fBase = (h->newrva + h->arg2) & 0xFFFFF000;
				if (fBase != fixBase) {
					fixBase = fBase;
					numBases++;
				}
				m->fixups.push_back(h->newrva + h->arg2);
				numFixups++;
				if (h->arg3 && h->arg4) {
					fBase = (h->newrva + h->arg4) & 0xFFFFF000;
					if (fBase != fixBase) {
						fixBase = fBase;
						numBases++;
					}
					m->fixups.push_back(h->newrva + h->arg4);
					numFixups++;
				}
			}
		}
		
		DWORD fixSize = (numBases * 8) + (numFixups * 2), fixI = 0;

		if (fixSize > *(DWORD*)fxsize->dataptr) {
			*(DWORD*)fxsize->dataptr = fixSize;
			if (fxrva->dataptr < m->memb || fxrva->dataptr > (&m->memb[m->size])) {
				//free(fxrva->dataptr);
			}
			fxrva->dataptr = (BYTE*)calloc(1, fixSize + 1);
			fxrva->datalen = fixSize;
			goto re;
		}

		fxsize->flags &= ~FL_DELTA; // apparently the delta fixing fucks this size

		IMAGE_BASE_RELOCATION *base = (IMAGE_BASE_RELOCATION*)fxrva->dataptr;
		WORD *fixs = MakePtr(WORD*, base, 8);

		fixBase              = m->fixups[0] & 0xFFFFF000;
		base->VirtualAddress = fixBase;
		base->SizeOfBlock    = 8;

		while (m->fixups.size()) {
			DWORD f = m->fixups[0];

			if ((f & 0xFFFFF000) != fixBase) {
				base = (IMAGE_BASE_RELOCATION*)&fixs[fixI];
				fixs = MakePtr(WORD*, base, 8);
				base->VirtualAddress = f & 0xFFFFF000;
				base->SizeOfBlock    = 8;
				fixBase = base->VirtualAddress;
				fixI    = 0;
			}

			fixs[fixI]         = 3 << 12;
			fixs[fixI++]      |= m->fixups[0];
			base->SizeOfBlock += 2;
			m->fixups.remove(0);
		}
	}

	// recalc pointers
	int expanded = 0;

#define SETHOOY(x, y) HOOY *x = ((y) <= maxRVA) ? fasthooy[y] : m->GetHOOYByOldRVA(y, FL_ALL)

	ForEachInList(m->hooyList, HOOY, h) {
		if (h->flags & FL_ARVA) {
			SETHOOY(h1, h->arg1);

			if (h1) {
				*(DWORD*)h->dataptr = MakePtr(DWORD, h1->newrva, m->imageBase);
			} else {
				return 3;
			}
		}

		if (h->flags & FL_RVA) {
			SETHOOY(h1, h->arg1);

			if (h1) {
				if (h->flags & FL_PHYS) {
					*(DWORD*)h->dataptr = h1->newofs;
				} else {
					*(DWORD*)h->dataptr = h1->newrva;
				}
			} else {
				return 3;
			}
		}

		if (h->flags & FL_FIXUP) {
			SETHOOY(h1, h->arg1);

			if (h1) {
				*(DWORD*)&h->dataptr[h->arg2] = MakePtr(DWORD, h1->newrva, m->imageBase);
			} else {
				return 3;
			}

			SETHOOY(h2, h->arg3);

			if (h2 && (h->arg4 != 0)) {
				*(DWORD*)&h->dataptr[h->arg4] = MakePtr(DWORD, h2->newrva, m->imageBase);
			}
		}

		if (h->flags & FL_DELTA) {
			SETHOOY(h1, h->arg1);
			SETHOOY(h2, h->arg2);

			if (h1) {
				if (h2) {
					if (h->flags & FL_PHYS) {
						*(DWORD*)h->dataptr = h2->newofs - h1->newofs;
					} else {
						*(DWORD*)h->dataptr = h2->newrva - h1->newrva;
					}
				} else {
					return 3;
				}
			} else {
				return 3;
			}
		}

		if (h->flags & FL_RES8) {
			*(DWORD*)h->dataptr |= 0x80000000;
		}

		if (h->flags & FL_FORCEOBJALIGN) {
			*(DWORD*)h->dataptr = ALIGN(*(DWORD*)h->dataptr, m->nt->OptionalHeader.SectionAlignment);
		}

		if (h->flags & FL_FORCEFILEALIGN) {
			*(DWORD*)h->dataptr = ALIGN(*(DWORD*)h->dataptr, m->nt->OptionalHeader.FileAlignment);
		}

		if (h->flags & FL_HAVEREL) {
			SETHOOY(h1, h->arg1);

			if (h1) {
				DWORD t = h1->newrva - (h->newrva + h->datalen);

				if (h->arg2 == 1) {
					if ((long)t != (char)t) {
						if (h->dataptr[0] == 0xEB) {
							h->datalen    = 5;
							h->dataptr    = (BYTE*)calloc(1, h->datalen);
							h->dataptr[0] = 0xE9;
						} else if ((h->dataptr[0] & 0xF0) == 0x70) {
							h->datalen    = 6;
							h->dataptr    = (BYTE*)calloc(1, h->datalen);
							h->dataptr[1] = h->dataptr[0] ^ 0x70 ^ 0x80;
							h->dataptr[0] = 0x0F;
						} else if (h->dataptr[0] == 0xE3) {
							h->datalen    = 2 + 6;
							h->dataptr    = (BYTE*)calloc(1, h->datalen);
							h->dataptr[0] = 0x09; // or ecx, ecx
							h->dataptr[1] = 0xC9;
							h->dataptr[2] = 0x0F; // jz
							h->dataptr[3] = 0x84;
						} else if (h->dataptr[0] == 0xE2) {
							h->datalen    = 1 + 6;
							h->dataptr    = (BYTE*)calloc(1, h->datalen);
							h->dataptr[0] = 0x49; // dec ecx
							h->dataptr[1] = 0x0F; // jnz
							h->dataptr[2] = 0x85;
						} else {
							return 4;
						}

						h->arg2 = 4;
						expanded++;
					} else {
						h->dataptr[h->datalen - 1] = (BYTE)t;
					}
				} else if (h->arg2 == 4) {
					*(DWORD*)&(h->dataptr[h->datalen - 4]) = t;
				}
			}
		}
	}

	if (expanded) {
		goto re;
	}

	int shrunk = 0;
	/*
	ForEachInList(hooyList, HOOY, h) {
		if (h->flags & FL_HAVEREL) {
			if (h->arg2 == 4) {
				SETHOOY(h1, h->arg1);

				DWORD t = h1->newrva - (h->newrva + h->datalen);

				if ((long)t == (char)t) {
					if (h->dataptr[0] == 0x0F) {
						switch (h->dataptr[1]) {
						case 0x87:
							h->datalen    = 2;
							h->dataptr[0] = 0x87;
							h->dataptr[1] = t;
							h->arg2       = 1;
							shrunk++;
							break;

						case 0x83:
							h->datalen    = 2;
							h->dataptr[0] = 0x73;
							h->dataptr[1] = t;
							h->arg2       = 1;
							shrunk++;
							break;

						case 0x82:
							h->datalen    = 2;
							h->dataptr[0] = 0x72;
							h->dataptr[1] = t;
							h->arg2       = 1;
							shrunk++;
							break;

						case 0x86:
							h->datalen    = 2;
							h->dataptr[0] = 0x76;
							h->dataptr[1] = t;
							h->arg2       = 1;
							shrunk++;
							break;

						case 0x84:
							h->datalen    = 2;
							h->dataptr[0] = 0x74;
							h->dataptr[1] = t;
							h->arg2       = 1;
							shrunk++;
							break;

						case 0x85:
							h->datalen    = 2;
							h->dataptr[0] = 0x75;
							h->dataptr[1] = t;
							h->arg2       = 1;
							shrunk++;
							break;

						case 0x8B:
							h->datalen    = 2;
							h->dataptr[0] = 0x7B;
							h->dataptr[1] = t;
							h->arg2       = 1;
							shrunk++;
							break;

						case 0x8A:
							h->datalen    = 2;
							h->dataptr[0] = 0x7A;
							h->dataptr[1] = t;
							h->arg2       = 1;
							shrunk++;
							break;

						case 0x8F:
							h->datalen    = 2;
							h->dataptr[0] = 0x7F;
							h->dataptr[1] = t;
							h->arg2       = 1;
							shrunk++;
							break;

						case 0x8D:
							h->datalen    = 2;
							h->dataptr[0] = 0x7D;
							h->dataptr[1] = t;
							h->arg2       = 1;
							shrunk++;
							break;

						case 0x8C:
							h->datalen    = 2;
							h->dataptr[0] = 0x7C;
							h->dataptr[1] = t;
							h->arg2       = 1;
							shrunk++;
							break;

						case 0x8E:
							h->datalen    = 2;
							h->dataptr[0] = 0x7E;
							h->dataptr[1] = t;
							h->arg2       = 1;
							shrunk++;
							break;

						case 0x81:
							h->datalen    = 2;
							h->dataptr[0] = 0x71;
							h->dataptr[1] = t;
							h->arg2       = 1;
							shrunk++;
							break;

						case 0x89:
							h->datalen    = 2;
							h->dataptr[0] = 0x79;
							h->dataptr[1] = t;
							h->arg2       = 1;
							shrunk++;
							break;

						case 0x80:
							h->datalen    = 2;
							h->dataptr[0] = 0x70;
							h->dataptr[1] = t;
							h->arg2       = 1;
							shrunk++;
							break;

						case 0x88:
							h->datalen    = 2;
							h->dataptr[0] = 0x78;
							h->dataptr[1] = t;
							h->arg2       = 1;
							shrunk++;
							break;
						}
					} else if (h->dataptr[0] == 0xE9) {
						h->datalen = 2;
						h->dataptr[0] = 0xEB;
						h->dataptr[1] = t;
						h->arg2       = 1;
						shrunk++;
						break;
					}
				}
			}
		}
	}
	*/

	if (shrunk) {
		goto re;
	}

	p             = ALIGN(((HOOY*)m->hooyList.tail)->newofs + ((HOOY*)m->hooyList.tail)->datalen, m->nt->OptionalHeader.FileAlignment);
	m->outputSize = p + m->overlaySize;
	m->outputMem  = (BYTE*)calloc(1, m->outputSize + 1);

	if (m->outputMem == NULL) {
		m->errorCount++;
		return 1;
	}

	ForEachInList(m->hooyList, HOOY, h) {
		if (h->flags & FL_PRESENT) {
			memcpy(&m->outputMem[h->newofs], h->dataptr, h->datalen);
		}
		if (h->flags & FL_OPCODE) {
			if (h->diza) {
				memset(h->diza, 0, sizeof(XDA));
				int len = XDisasm(h->diza, h->dataptr, h->newrva);
			} else {
				h->diza = (XDA*)calloc(1, sizeof(XDA));
				int len = XDisasm(h->diza, h->dataptr, h->newrva);
			}
		}
	}

	if (m->overlaySize > 0) {
		memcpy(&m->outputMem[p], &m->physMem[m->overlayRVA], m->overlaySize);
	}

	IMAGE_DOS_HEADER   *tmz = (IMAGE_DOS_HEADER*)&m->outputMem[0];
	IMAGE_NT_HEADERS32 *tpe = (IMAGE_NT_HEADERS32*)&m->outputMem[tmz->e_lfanew];
	
	size_t newVS = ALIGN(((HOOY*)m->hooyList.tail)->newrva + ((HOOY*)m->hooyList.tail)->datalen, tpe->OptionalHeader.SectionAlignment);
	DEBUG_LOG("New phys size: %i / 0x%X\nNew virt size: %i / 0x%X : %i / 0x%X\n", m->outputSize, m->outputSize, newVS, newVS, tpe->OptionalHeader.SizeOfImage, tpe->OptionalHeader.SizeOfImage);

	if (tpe->OptionalHeader.CheckSum != 0) {
		tpe->OptionalHeader.CheckSum = 0;
		//tpe->OptionalHeader.CheckSum = calc_pe_csum(m->outputMem, m->outputSize);
	}

	//free((void*)fasthooy); // FIXME: crash?
	return 0;
}

int AssembleX86Bin(Bent *m) {
reBin:
	size_t v = (size_t)m->imageBase, p = 0;

	HOOY *start = (HOOY*)m->hooyList.root;
	HOOY *end = (HOOY*)m->hooyList.tail;

	for (HOOY *h = start; h != NULL; h = h->next) {
		h->newofs = p;
		h->newrva = v;
		if (h->dataptr && h->datalen) {
			p += h->datalen;
			v += h->datalen;
		}
	}

	m->outputMem  = (BYTE*)calloc(1, p + 1);
	m->outputSize = p;

	int expanded = 0;

	for (HOOY *h = start; h != NULL; h = h->next) {
		if (h->flags & FL_OPCODE) {
			if (h->flags & FL_HAVEREL) {
				HOOY *dest = m->GetHOOYByOldRVA(h->arg1, FL_LABEL);
				*(DWORD*)(&h->dataptr[h->datalen - 4]) = dest->newrva - (h->newrva + h->datalen);
				ReDisasm(h);
			}
		}
		if (h->flags & FL_FIXUP) {
		}
		if (h->flags & FL_DELTA) {
		}
		if (h->flags & FL_RVA) {
		}
	}

	if (expanded) {
		goto reBin;
	}

	for (HOOY *h = start; h != NULL; h = h->next) {
		if (h->dataptr && h->datalen) {
			memcpy(&m->outputMem[h->newofs], h->dataptr, h->datalen);
		}
	}

	return 0;
}
