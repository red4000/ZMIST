#include "bent.h"
#include "dbginfo.h"
#include "log.h"
#include "pedump.h"

int ProcessImport(Bent *m, size_t rva) {
	if (*(DWORD*)&m->memb[rva] == 0) {
		return 1;
	}

	m->MarkRVAData(rva);
	rva = *(DWORD*)&m->memb[rva];

	while (*(DWORD*)(&m->memb[rva])) {
		DWORD i = *(DWORD*)&m->memb[rva];
		if (i < m->size) {
			m->MarkData(i, 2);
			m->MarkRVAData(rva);
		}
		m->flag[rva] |= FL_IMPORT;

		rva += 4;
	}
	return 0;
}

void ProcessResource(Bent *m, size_t rva) {
	DWORD c = ((WORD*)m->memb)[rva + 0x0C] + ((WORD*)m->memb)[rva + 0x0E];
	DWORD t = rva + 0x10;

	while (c--) {
		if (*(DWORD*)&m->memb[t] & 0x80000000) {
			m->flag[t] |= FL_RES8;
			DWORD q = *(DWORD*)&m->memb[t] & 0x7FFFFFFF;
			m->MarkDelta(t, m->nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress, q);
		}

		t += 4;

		if (*(DWORD*)&m->memb[t] & 0x80000000) {
			m->flag[t] |= FL_RES8;
			DWORD q = *(DWORD*)&m->memb[t] & 0x7FFFFFFF;
			m->MarkDelta(t, m->nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress, q);
			ProcessResource(m, m->nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress + q);
		} else {
			m->MarkDelta(t, m->nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress, *(DWORD*)&m->memb[t]);
			m->MarkRVAData(m->nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress + *(DWORD*)&m->memb[t]);
		}

		t += 4;
	}
}

int LoadPE(Bent *b) {
	int res = 0;

	if (0 != b->AllocBuffers(b->nt->OptionalHeader.SizeOfImage)) {
		return 1;
	}

	b->fileType = FT_PE;

	IMAGE_SECTION_HEADER *oe = IMAGE_FIRST_SECTION(b->nt);

	/*
	for (size_t i = 0; i < b->nt->FileHeader.NumberOfSections; i++) {
		oe[i].Misc.VirtualSize = ALIGN(oe[i].Misc.VirtualSize, b->nt->OptionalHeader.SectionAlignment);
		oe[i].SizeOfRawData    = ALIGN(oe[i].SizeOfRawData, b->nt->OptionalHeader.FileAlignment);
	}
	*/

	b->imageBase = (void*)b->nt->OptionalHeader.ImageBase;

	b->MarkData(0, MakeDelta(size_t, b->nt, b->mz));

	memcpy(b->memb, b->physMem, b->nt->OptionalHeader.SizeOfHeaders);

	//b->mz = (IMAGE_DOS_HEADER*)b->memb;
	//b->nt = MakePtr(IMAGE_NT_HEADERS*, b->mz, b->mz->e_lfanew);

	for (size_t i = 0; i < b->nt->OptionalHeader.SizeOfHeaders; i++) {
		b->flag[i] |= FL_PRESENT | FL_VPRESENT;
	}

	size_t t;

	t              = b->nt->FileHeader.NumberOfSections - 1;
	b->overlayRVA  = oe[t].PointerToRawData + oe[t].SizeOfRawData;
	b->overlaySize = b->physSize - b->overlayRVA;

	if ((signed)b->overlaySize < 0) {
		b->errorCount++;
		return 2;
	}

	t = 0;

	for (size_t i = 0; i < b->overlaySize; i++) {
		t |= b->memb[b->overlayRVA + i];
	}

	if (t == 0) {
		b->overlaySize = 0;
	}

	DEBUG_LOG("Phys size: %i / 0x%X\nVirt size: %i / 0x%X\n", b->physSize, b->physSize, b->nt->OptionalHeader.SizeOfImage, b->nt->OptionalHeader.SizeOfImage);

	for (size_t i = 0; i < b->nt->FileHeader.NumberOfSections; i++) {
		memcpy(&b->memb[oe[i].VirtualAddress], MakePtr(void*, b->physMem, oe[i].PointerToRawData), MIN(oe[i].SizeOfRawData, oe[i].Misc.VirtualSize));

		DWORD sectionFlag = FL_VPRESENT;

		if (oe[i].Characteristics & 0x20000020) {
			sectionFlag |= FL_EXECUTABLE;
		}

		for (size_t j = 0; j < oe[i].SizeOfRawData; j++) {
			b->flag[oe[i].VirtualAddress + j] |= FL_PRESENT;
		}

		for (size_t j = 0; j < oe[i].Misc.VirtualSize; j++) {
			b->flag[oe[i].VirtualAddress + j] |= sectionFlag;
		}
	}

	t = b->mz->e_lfanew + 0xF8;

	for (size_t n = 0; n < (DWORD)b->nt->FileHeader.NumberOfSections; n++) {
		DEBUG_LOG("section %-8s at %04X, %08X..%08X vsize %08X psize %08X\n", oe[n].Name, t, oe[n].VirtualAddress, oe[n].VirtualAddress + oe[n].Misc.VirtualSize, oe[n].Misc.VirtualSize, oe[n].SizeOfRawData);

		b->SetComment(oe[n].VirtualAddress, (char*)&oe[n].Name[0]);
		b->flag[oe[n].VirtualAddress] |= FL_SECTALIGN;

		b->MarkData(t, IMAGE_SIZEOF_SHORT_NAME);
		b->MarkDelta(t + 0x08, oe[n].VirtualAddress, oe[n].Misc.VirtualSize); // 08=virtsize
		b->MarkRVA(t + 0x0C); // 0C=virtrva
		b->MarkDelta(t + 0x10, oe[n].VirtualAddress, oe[n].SizeOfRawData); // 10=physsize
		b->flag[t + 0x10] |= FL_PHYS | FL_FORCEFILEALIGN; // 10=physsize

		b->flag[t + 0x14] |= FL_PHYS | FL_RVA; // 14=physoffs
		b->arg1[t + 0x14]  = oe[n].VirtualAddress; // 14=physoffs

		t += sizeof(_IMAGE_SECTION_HEADER);
	}

	b->SetComment(b->nt->OptionalHeader.SizeOfImage, "|SizeOfImage");
	b->flag[b->nt->OptionalHeader.SizeOfImage] |= FL_SECTALIGN;

	t = b->mz->e_lfanew;

	if (b->nt->OptionalHeader.AddressOfEntryPoint) {
		b->MarkRVA(t + 0x28);
		b->SetComment(b->nt->OptionalHeader.AddressOfEntryPoint, "|OEP");
		b->flag[b->nt->OptionalHeader.AddressOfEntryPoint] |= FL_ENTRY | FL_CREF;
	}

	b->MarkRVA(t + 0x2C); // 2C=baseofcode
	b->MarkRVA(t + 0x30); // 30=baseofdata

	b->MarkDelta(t + 0x1C, b->nt->OptionalHeader.BaseOfCode, b->nt->OptionalHeader.SizeOfCode); // 1C=sizeofcode
	b->flag[t + 0x1C] |= FL_FORCEOBJALIGN;

	//b->MarkDelta(t + 0x20, b->nt->OptionalHeader.BaseOfData, b->nt->OptionalHeader.SizeOfInitializedData); // 20=sizeofidata
	//b->flag[t + 0x20] |= FL_FORCEOBJALIGN;

	b->MarkRVA(t + 0x50);     // 50=imagesize
	b->MarkRVAData(t + 0x54); // 54=headersize

	for (size_t i = 0; i < MIN(16, b->nt->OptionalHeader.NumberOfRvaAndSizes); i++) {
		t = b->mz->e_lfanew + 0x78 + (i * 8); // rva/size #i

		if (*((DWORD*)&b->memb[t])) {
			DEBUG_LOG("data descriptor %02X: %02i / %02X, %08X..%08X size %04X %s\n", t, i, i, *((DWORD*)&b->memb[t]), *((DWORD*)&b->memb[t]) + *((DWORD*)&b->memb[t + 4]), *((DWORD*)&b->memb[t + 4]), dataDirectoryString[i]);
			b->MarkRVAData(t);
			b->MarkDelta(t + 4, *(DWORD*)(&b->memb[t]), *(DWORD*)(&b->memb[t + 4]));

			for (DWORD j = *((DWORD*)&b->memb[t]); j < (*((DWORD*)&b->memb[t]) + *((DWORD*)&b->memb[t + 4])); j++) {
				if (j < b->nt->OptionalHeader.SizeOfImage) {
					b->flag[i] |= FL_DATA;
				} else {
					DEBUG_LOG("Error: data descriptor at %X [%i/%X] out of bounds\n", t, i, i);
					b->errorCount++;
					break;
				}
			}
		}
	}

	if (b->nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress) {
		DWORD imp0 = b->nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
		b->flag[imp0] |= FL_LABEL;
		IMAGE_IMPORT_DESCRIPTOR *iid;

		while (*(DWORD*)&b->memb[imp0 + 0x10]) { // 10=addresstable
			iid = MakePtr(IMAGE_IMPORT_DESCRIPTOR*, b->memb, imp0);
			char *cmt = (char*)calloc(1, 256);
			//sprintf_s(cmt, 256, "%s.%s %08X", MakePtr(char*, b->memb, iid->Name), fName, fAddr);
			//MarkComment

			ProcessImport(b, imp0 + 0x00); // 00=lookup
			ProcessImport(b, imp0 + 0x10); // 10=addresstable
			b->MarkRVAData(imp0 + 0x0C);    // 0C=name
			// TODO: cache import data
			imp0 += sizeof(IMAGE_IMPORT_DESCRIPTOR);
		}
	}

	if (b->nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress) {
		DWORD exp0 = b->nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
		b->MarkRVAData(exp0 + 0x0C); // 0C=namerva
		b->MarkRVAData(exp0 + 0x1C); // 1C=addresstablerva
		b->MarkRVAData(exp0 + 0x20); // 20=namepointersrva
		b->MarkRVAData(exp0 + 0x24); // 24=ordinaltablerva
		IMAGE_EXPORT_DIRECTORY *exp = (IMAGE_EXPORT_DIRECTORY*)&b->memb[exp0];
		exp0 = exp->AddressOfFunctions;

		for (size_t i = 0; i < exp->NumberOfFunctions; i++) {
			DWORD x = exp0 + i * 4;
			b->MarkRVA(x);
			if (b->flag[*(DWORD*)&b->memb[x]] & FL_EXECUTABLE) {
				b->flag[*(DWORD*)&b->memb[x]] |= FL_SIGNATURE;
			}
		}

		exp0 = exp->AddressOfNames;

		for (size_t i = 0; i < exp->NumberOfNames; i++) {
			b->MarkRVAData(exp0 + i * 4);
		}
	}

	if ((b->nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress) && ((b->nt->FileHeader.Characteristics & 1) == 0)) {
		DWORD     fixupcount = 0;

		IMAGE_BASE_RELOCATION *fx = MakePtr(IMAGE_BASE_RELOCATION*, b->memb, b->nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
		DWORD     k  = 0;

		while (k < b->nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size) {
			for (size_t i = 0; i < (fx->SizeOfBlock - 8) / 2; i++) {
				WORD *offs = MakePtr(WORD*, MakePtr(DWORD, fx, sizeof(IMAGE_BASE_RELOCATION)), i * 2);
				DWORD fxtynt = (*offs) >> 12;

				if (fxtynt == 3) {
					fixupcount++;
					DWORD j = fx->VirtualAddress + ((*offs) & 0x0FFF);

					if (j > b->nt->OptionalHeader.SizeOfImage) {
						b->errorCount++;
						return 3;
					}

					b->fixups.push_back(j);
					b->MarkFixup(j);
				} else if (fxtynt != 0) {
					DEBUG_LOG("error: unknown fixup type %i idx %i base %X\n", fxtynt, i, fx->VirtualAddress);
					b->errorCount++;
					return 3;
				}
			}

			k            += fx->SizeOfBlock;
			*(DWORD*)&fx += fx->SizeOfBlock;
		}

		if (fixupcount == 0) {
			b->errorCount++;
			return 3; // # fixups == 0
		}
	}

	if (b->nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress) {
		ProcessResource(b, b->nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress);
	}

	if (b->nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress) {
		IMAGE_TLS_DIRECTORY32 *tls             = (IMAGE_TLS_DIRECTORY32*)&b->memb[b->nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress];
		DWORD                  tls_callbackrva = tls->AddressOfCallBacks - b->nt->OptionalHeader.ImageBase;

		if (tls->StartAddressOfRawData) {
			b->MarkARVA(MakeDelta(size_t, &tls->StartAddressOfRawData, b->memb));
		}
		if (tls->EndAddressOfRawData) {
			b->MarkARVA(MakeDelta(size_t, &tls->EndAddressOfRawData, b->memb));
		}
		if (tls->AddressOfIndex) {
			b->MarkARVA(MakeDelta(size_t, &tls->AddressOfIndex, b->memb));
		}
		if (tls->AddressOfCallBacks) {
			b->MarkARVA(MakeDelta(size_t, &tls->AddressOfCallBacks, b->memb));
		}

		while (*(DWORD*)&b->memb[tls_callbackrva]) {
			DWORD          x = (*(DWORD*)&b->memb[tls_callbackrva]) - b->nt->OptionalHeader.ImageBase;
			if (x < b->size) {
				b->MarkFixup(tls_callbackrva);
				b->flag[x]      |= FL_ENTRY | FL_CREF | FL_LABEL;
				tls_callbackrva += 4;
			} else {
				DEBUG_LOG("error: tls callback %08X rva %08X out of bounds\n", x, tls_callbackrva);
				b->errorCount++;
			}
		}
	}

	if (b->nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress) {
		LoadSymbols(b);
	}

	if (b->nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG].VirtualAddress) {
		IMAGE_LOAD_CONFIG_DIRECTORY32 *cfg = (IMAGE_LOAD_CONFIG_DIRECTORY32*)&b->memb[b->nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG].VirtualAddress];

		if (cfg->SecurityCookie) {
			b->MarkARVA(MakeDelta(size_t, &cfg->SecurityCookie, b->memb));
		}
		if (cfg->SEHandlerTable) {
			DWORD table = MakeDelta(DWORD, cfg->SEHandlerTable, b->imageBase);
			b->MarkARVA(MakeDelta(DWORD, &cfg->SEHandlerTable, b->memb));
			for (size_t i = 0; i < cfg->SEHandlerCount; i++) {
				b->MarkRVA(table + (i * 4));
			}
		}
		/*
		if (cfg->GuardCFCheckFunctionPointer) {
			b->MarkARVA(MakeDelta(size_t, &cfg->GuardCFCheckFunctionPointer, b->memb));
		}
		if (cfg->Reserved2) {
			b->MarkARVA(MakeDelta(size_t, &cfg->Reserved2, b->memb)); // GuardCFDispatchFunctionPointer
		}
		if (cfg->GuardCFFunctionTable) {
			DWORD table = MakeDelta(DWORD, cfg->GuardCFFunctionTable, b->imageBase);
			b->MarkARVA(MakeDelta(DWORD, &cfg->GuardCFFunctionTable, b->memb));
			for (size_t i = 0; i < cfg->GuardCFFunctionCount; i++) {
				b->MarkRVA(table + (i * 4));
			}
		}
		*/
	}

	//free(physMem);
	//physMem = NULL;

	return res;
}
