#include "bent.h"
#include "generator.h"

int PatchBlock(Bent *b, HOOY *start, HOOY *end, HOOY *with) {
	int res = 0;

	DWORD blockSize = 0;
	for (HOOY *h = start; h != NULL; h = h->next) {
		blockSize += h->datalen;
		if (h == end) {
			break;
		}
	}

	if (with->datalen > blockSize) {
		return 1;
	}

	HOOY *h = end->next;
	b->hooyList.remove_block(start, end);
	b->hooyList.insert_before(with, h);
	with->oldofs = start->oldofs;

	DWORD padLen = blockSize - with->datalen;

	if (padLen) {
		HOOY *padding = GenHData(NULL, padLen);
		padding->oldofs = with->oldofs + with->datalen;
		b->hooyList.insert_before(padding, h);
	}

	//xrefs..
	return 0;
}

int EasyAssemble(Bent *b, HOOY *start, HOOY *end, DWORD va, DWORD pa) {
	DWORD v = va, p = pa;
	for (HOOY *h = start; h != NULL; h = h->next) {
		h->newrva = v;
		h->newofs = p;

		if (h->flags & FL_SECTALIGN) {
			v = ALIGN(v, b->nt->OptionalHeader.SectionAlignment);
			p = ALIGN(p, b->nt->OptionalHeader.FileAlignment);
		} else {
			if (h->flags & FL_VPRESENT) {
				v += h->datalen;
			}

			if (h->flags & FL_PRESENT) {
				p += h->datalen;
			}
		}
	}
	/*
	for (HOOY *h = start; h != NULL; h = h->next) {
		h->newofs = h->newrva = v;
		if (h->dataptr && h->datalen) {
			v += h->datalen;
		}
	}
	for (HOOY *h = start; h != NULL; h = h->next) {
		if (h->flags & FL_OPCODE) {
			if (h->flags & FL_HAVEREL) {
				HOOY *dest = FindHooyByOldRVA(h->arg1);
				if (dest == NULL) {
					//dest = m->FindHooyByOldRVA(h->arg1);
				}
				*(DWORD*)(&h->dataptr[h->datalen - 4]) = dest->newrva - (h->newrva + h->datalen);
				ReDisasm(h);
			}
		}
	}
	for (HOOY *h = start; h != NULL; h = h->next) {
		if (h->dataptr && h->datalen) {
			memcpy(&output[h->newofs], h->dataptr, h->datalen);
		}
	}
	*/

	for (HOOY *h = start; h != NULL; h = h->next) {
		if (h->flags & FL_FIXUP) {
		}
		
		if (h->flags & FL_RVA) {
		}
		
		if (h->flags & FL_DELTA) {
		}

		if (h->flags & FL_OPCODE) {
			if (h->flags & FL_HAVEREL) {
				HOOY *dest = b->GetHOOYByOldRVA(h->arg1, FL_ALL);
				if (dest == NULL) {
					for (HOOY *j = start; j != NULL; j = j->next) {
						if (j->oldrva == h->arg1) {
							dest = j;
							break;
						}
						if (j == end) {
							break;
						}
					}
				}
				if (dest) {
					*(DWORD*)(&h->dataptr[h->datalen - 4]) = dest->newrva - (h->newrva + h->datalen);
					ReDisasm(h);
				} else {
					return 2;
				}
			}
		}
	}
	return 0;
}

int PatchX86(Bent *b) {
	//1. copy old binary to vector
	Vector<BYTE> outVector;
	outVector.reserve(b->physSize + 1);
	b->outputMem  = outVector.base;
	b->outputSize = b->physSize;
	memcpy(b->outputMem, b->physMem, b->physSize);

	//2. patch headers & code/data
	// this patches even h->dataptr since h->dataptr = &b->physMem[]
	memcpy(b->outputMem, b->memb, b->physSize);
	// maybe a pass to check if any hooy's have oldrva && custom data alloc ( >=physMem+physSize)

	//3. resize vector for new sect data
	IMAGE_SECTION_HEADER *oe = IMAGE_FIRST_SECTION(b->nt);
	size_t i = b->nt->FileHeader.NumberOfSections - 1;
	b->outputSize = oe[i].PointerToRawData + oe[i].SizeOfRawData + b->overlaySize;
	outVector.reserve(b->outputSize + 1);
	outVector.usedCapacity = b->outputSize;
	//3. calculate v,p from last sect

	//4. calculate new size(compiling FL_GENERATED HOOYs to new RVA), resize vector and write new data
	DWORD v = 0, p = 0;
	
	for (HOOY *h = (HOOY*)b->hooyList.root; h != NULL; h = h->next) {
		if (h->flags & FL_GENERATED) {
			if (v != 0) {
				// update v,p
			} else {
				// set v,p
			}
		}
	}

	// FIXME: output size doesn't take account of any generated sizes...
	b->outputSize = oe[i].PointerToRawData + oe[i].SizeOfRawData + b->overlaySize;
	b->outputMem  = (BYTE*)calloc(1, b->outputSize + 1);

	//memcpy(b->outputMem, b->physMem, b->physSize);
	memcpy(b->outputMem, b->physMem, oe[i].PointerToRawData + oe[i].SizeOfRawData);
	memcpy(b->outputMem, b->mz, b->nt->OptionalHeader.SizeOfHeaders);

	int hasFixups = 0;

	for (HOOY *h = (HOOY*)b->hooyList.root; h != NULL; h = h->next) {
		if (h->flags & FL_GENERATED) {
			if (h->flags & FL_PRESENT) {
				if (h->flags & FL_FIXUP) {
					hasFixups = 1;
				}
				if (h->oldofs) {
					memcpy(&b->outputMem[h->oldofs], h->dataptr, h->datalen);
				} else {
					memcpy(&b->outputMem[h->newofs], h->dataptr, h->datalen);
				}
			}
		}
	}

	if (hasFixups) { // null old fixup data, point to new at EOF or new sect
	}
	
	//4. replace IAT, RELOC as nessecary

	return 0;
}
