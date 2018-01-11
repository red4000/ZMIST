#include "bent.h"
#include "dbginfo.h"
#include "log.h"

#include <stdlib.h>
#include "peloader64.h"
#include <assert.h>

void ProcessImport64(Bent *m, size_t rva) {
	if (*(DWORD*)&m->memb[rva] == 0) {
		return;
	}

	m->MarkRVAData64(rva);
	rva = *(DWORD*)&m->memb[rva];

	while (*(DWORD*)&m->memb[rva]) {
		if (*(DWORD*)&m->memb[rva] < m->size) {
			m->flag[(*(DWORD*)&m->memb[rva]) + 0] |= FL_DATA;
			m->flag[(*(DWORD*)&m->memb[rva]) + 1] |= FL_DATA;
			m->MarkRVAData64(rva);
		}

		rva += 4;
	}
}

void ProcessResource64(Bent *m, size_t rva) {
	DWORD c = ((WORD*)m->memb)[rva + 0x0C] + ((WORD*)m->memb)[rva + 0x0E];
	DWORD t = rva + 0x10;

	while (c--) {
		if (((DWORD*)m->memb)[t] & 0x80000000) {
			m->flag[t] |= FL_RES8;
			DWORD q = ((DWORD*)m->memb)[t] & 0x7FFFFFFF;
			m->MarkDelta(t, m->nt64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress, q);
		}

		t += 4;

		if (((DWORD*)m->memb)[t] & 0x80000000) {
			m->flag[t] |= FL_RES8;
			DWORD q = ((DWORD*)m->memb)[t] & 0x7FFFFFFF;
			m->MarkDelta(t, m->nt64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress, q);
			ProcessResource64(m, m->nt64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress + q);
		} else {
			m->MarkDelta(t, m->nt64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress, ((DWORD*)m->memb)[t]);
			m->MarkRVAData(m->nt64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress + ((DWORD*)m->memb)[t]);
		}

		t += 4;
	}
}

int LoadPE64(void *p, size_t len, Bent *m) {
	unsigned char    *c  = (unsigned char*)p;
	IMAGE_DOS_HEADER *mz = (IMAGE_DOS_HEADER*)c;

	if (mz->e_magic != IMAGE_DOS_SIGNATURE) {
		return 1;
	}

	m->nt64 = MakePtr(IMAGE_NT_HEADERS64*, mz, mz->e_lfanew);
	if (m->nt64->Signature != IMAGE_NT_SIGNATURE) {
		return 2;
	}

	if (m->nt64->FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64) {
		return 3;
	}

	m->AllocBuffers(m->nt64->OptionalHeader.SizeOfImage + 1);

	m->imageBase = (void*)m->nt64->OptionalHeader.ImageBase;

	_IMAGE_SECTION_HEADER *oe = IMAGE_FIRST_SECTION(m->nt64);

	size_t i, t, n;

	for (i = 0; i < m->nt64->OptionalHeader.SizeOfImage; i++) {
		m->flag[i] |= FL_PRESENT | FL_VPRESENT;
	}

	for (n = 0; n < (size_t)m->nt64->FileHeader.NumberOfSections; n++) {
		if ((oe[n].PointerToRawData == 0) || (oe[n].SizeOfRawData == 0) ||
			(oe[n].VirtualAddress == 0) || (oe[n].Misc.VirtualSize == 0)) {
			m->errorCount++;
			return 4;
		}

		//oe[n].Misc.VirtualSize = ALIGN(oe[n].Misc.VirtualSize, m->nt64->OptionalHeader.FileAlignment);
		//oe[n].SizeOfRawData    = ALIGN(oe[n].PointerToRawData, m->nt64->OptionalHeader.FileAlignment);
	}

	t              = m->nt64->FileHeader.NumberOfSections - 1;
	m->overlayRVA  = oe[t].PointerToRawData + oe[t].SizeOfRawData;
	m->overlaySize = m->physSize - m->overlayRVA;

	if ((signed)m->overlaySize < 0) {
		m->errorCount++;
		return 4;
	}

	t = 0;

	for (i = 0; i < m->overlaySize; i++) {
		t |= m->memb[m->overlayRVA + i];
	}

	if (t == 0) {
		m->overlaySize = 0;
	}

	t = m->nt64->OptionalHeader.SizeOfImage + 1; // +1 because we may have last virtual entry

	memcpy(m->memb, p, m->nt64->OptionalHeader.SizeOfHeaders);

	for (i = 0; i < m->nt64->OptionalHeader.SizeOfHeaders; i++) {
		m->flag[i] |= FL_PRESENT | FL_VPRESENT;
	}

	for (n = 0; n < (DWORD)m->nt64->FileHeader.NumberOfSections; n++) {
		memcpy(&m->memb[oe[n].VirtualAddress], MakePtr(void*, p, oe[n].PointerToRawData), MIN(oe[n].SizeOfRawData, oe[n].Misc.VirtualSize));

		DWORD sectionFlag = FL_VPRESENT;

		if (oe[n].Characteristics & 0x20000020) {
			sectionFlag |= FL_EXECUTABLE;
		}

		for (i = 0; i < oe[n].SizeOfRawData; i++) {
			m->flag[oe[n].VirtualAddress + i] |= FL_PRESENT;
		}

		for (i = 0; i < oe[n].Misc.VirtualSize; i++) {
			m->flag[oe[n].VirtualAddress + i] |= sectionFlag;
		}
	}

	t = mz->e_lfanew + MakeDelta(size_t, oe, m->nt);

	for (n = 0; n < (DWORD)m->nt64->FileHeader.NumberOfSections; n++) {
		m->flag[oe[n].VirtualAddress] |= FL_SECTALIGN;

		m->MarkDelta(t + 0x08, oe[n].VirtualAddress, oe[n].Misc.VirtualSize); // 08=virtsize

		m->MarkRVA(t + 0x0C); // 0C=virtrva

		m->MarkDelta(t + 0x10, oe[n].VirtualAddress, oe[n].SizeOfRawData); // 10=physsize
		m->flag[t + 0x10] |= FL_PHYS | FL_FORCEFILEALIGN; // 10=physsize

		m->flag[t + 0x14] |= FL_PHYS | FL_RVA; // 14=physoffs
		m->arg1[t + 0x14]  = oe[n].VirtualAddress; // 14=physoffs

		t += sizeof(_IMAGE_SECTION_HEADER);
	}

	m->flag[m->nt64->OptionalHeader.SizeOfImage] |= FL_SECTALIGN;
	 
	t = mz->e_lfanew;

	if (m->nt64->OptionalHeader.AddressOfEntryPoint) {
		m->MarkRVA(t + FIELD_OFFSET(IMAGE_NT_HEADERS64, OptionalHeader.AddressOfEntryPoint)); // EntryPointRVA
	}

	m->MarkRVA(t + FIELD_OFFSET(IMAGE_NT_HEADERS64, OptionalHeader.BaseOfCode)); // 2C=baseofcode
	
	m->MarkDelta(t + FIELD_OFFSET(IMAGE_NT_HEADERS64, OptionalHeader.SizeOfCode), m->nt64->OptionalHeader.BaseOfCode, m->nt64->OptionalHeader.SizeOfCode); // 1C=sizeofcode
	m->flag[t + FIELD_OFFSET(IMAGE_NT_HEADERS64, OptionalHeader.SizeOfCode)] |= FL_FORCEOBJALIGN;

	//m->MarkDelta(t + FIELD_OFFSET(IMAGE_NT_HEADERS64, OptionalHeader.SizeOfInitializedData), m->nt64->OptionalHeader., m->nt64->OptionalHeader.SizeOfInitializedData); // 20=sizeofidata
	//m->flag[t + FIELD_OFFSET(IMAGE_NT_HEADERS64, OptionalHeader.SizeOfInitializedData)] |= FL_FORCEOBJALIGN;

	m->MarkRVA(t + FIELD_OFFSET(IMAGE_NT_HEADERS64, OptionalHeader.SizeOfImage));     // 50=imagesize
	m->MarkRVAData(t + FIELD_OFFSET(IMAGE_NT_HEADERS64, OptionalHeader.SizeOfHeaders)); // 54=headersize

	for (i = 0; i < MIN(16, m->nt64->OptionalHeader.NumberOfRvaAndSizes); i++) {
		t = mz->e_lfanew + FIELD_OFFSET(IMAGE_NT_HEADERS64, OptionalHeader.DataDirectory[0]) + (i * 8); // rva/size #i

		if (*(DWORD*)&m->memb[t]) {
			m->MarkRVAData(t);
			m->MarkDelta(t + 4, *(DWORD*)&m->memb[t], *(DWORD*)&m->memb[t + 4]);

			for (DWORD i = *(DWORD*)&m->memb[t]; i < *(DWORD*)&m->memb[t] + *(DWORD*)&m->memb[t + 4]; i++) {
				if (i < m->nt64->OptionalHeader.SizeOfImage) {
					m->flag[i] |= FL_DATA;
				} else {
					//log("error: section out of bounds\n");
					//m->errorCount++;
				}
			}
		}
	}

	if (m->nt64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress) {
		DWORD imp0 = m->nt64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
		m->flag[imp0] |= FL_LABEL;

		while (*(DWORD*)&m->memb[imp0 + 0x10]) { // 10=addresstable
			ProcessImport64(m, imp0 + 0x00); // 00=lookup
			ProcessImport64(m, imp0 + 0x10); // 10=addresstable
			m->MarkRVAData(imp0 + 0x0C);    // 0C=name
			// TODO: cache import data
			imp0 += sizeof(IMAGE_IMPORT_DESCRIPTOR);
			if (0 == m->memb[imp0]) {
				break;
			}
		}
	}

	if (m->nt64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress) {
		DWORD exp0 = m->nt64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
		m->MarkRVAData(exp0 + 0x0C); // 0C=namerva
		m->MarkRVAData(exp0 + 0x1C); // 1C=addresstablerva
		m->MarkRVAData(exp0 + 0x20); // 20=namepointersrva
		m->MarkRVAData(exp0 + 0x24); // 24=ordinaltablerva
		IMAGE_EXPORT_DIRECTORY *exp = (IMAGE_EXPORT_DIRECTORY*)&m->memb[exp0];
		exp0 = exp->AddressOfFunctions; //?

		for (i = 0; i < exp->NumberOfFunctions; i++) {
			DWORD x = exp0 + i * 4;
			m->MarkRVA(x);
			if (m->flag[*(DWORD*)&m->memb[x]] & FL_EXECUTABLE) {
				m->flag[*(DWORD*)&m->memb[x]] |= FL_SIGNATURE;
			}
		}

		exp0 = exp->AddressOfNames;

		for (i = 0; i < exp->NumberOfNames; i++) {
			m->MarkRVAData(exp0 + i * 4);
		}
	}

	if ((m->nt64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress) && ((m->nt64->FileHeader.Characteristics & 1) == 0)) {
		DWORD     fixupcount = 0;

		IMAGE_BASE_RELOCATION *fx = MakePtr(IMAGE_BASE_RELOCATION*, m->memb, m->nt64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
		DWORD                  k  = 0;

		while (k < m->nt64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size) {
			for (i = 0; i < (fx->SizeOfBlock - 8) / 2; i++) {
				WORD *offs   = MakePtr(WORD*, MakePtr(DWORD, fx, sizeof(IMAGE_BASE_RELOCATION)), i * 2);
				DWORD fxtynt = (*offs) >> 12;

				if (fxtynt & IMAGE_REL_BASED_DIR64) {
					fixupcount++;
					DWORD j = fx->VirtualAddress + ((*offs) & 0x0FFF);

					if (j > m->nt64->OptionalHeader.SizeOfImage) {
						m->errorCount++;
						return 5;
					}

					m->MarkFixup64(j);
				} else if (fxtynt != 0) {
					m->errorCount++;
					return 5;
				}
			}

			k            += fx->SizeOfBlock;
			*(DWORD*)&fx += fx->SizeOfBlock;
		}

		if (fixupcount == 0) {
			m->errorCount++;
			return 6; // # fixups == 0
		}
	}

	if (m->nt64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress) {
		assert(0);
		ProcessResource64(m, m->nt64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress);
	}

	if (m->nt64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress) {
		IMAGE_TLS_DIRECTORY64 *tls             = (IMAGE_TLS_DIRECTORY64*)&m->memb[m->nt64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress];
		DWORD                  tls_callbackrva = tls->AddressOfCallBacks - m->nt64->OptionalHeader.ImageBase;

		while (*(DWORD*)&m->memb[tls_callbackrva]) {
			DWORD          x = (*(DWORD*)&m->memb[tls_callbackrva]) - m->nt64->OptionalHeader.ImageBase;
			m->flag[x]      |= FL_ENTRY;
			tls_callbackrva += 4;
		}
	}

	if (m->nt64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress) {
		LoadSymbols(m);
	}

	for (size_t i = 0; i < m->nt64->FileHeader.NumberOfSections; i++) {
		if (0 == strcmp((char*)&oe[i].Name[0], ".text")) {
		}
		if (0 == strcmp((char*)&oe[i].Name[0], "code")) {
		}
	}
	//_IMAGE_SECTION_HEADER *text = GetSectionByName(".text");
	//_IMAGE_SECTION_HEADER *code = GetSectionByName("code");

	//code_start = text->VirtualAddress;
	//code_end   = text->VirtualAddress + text->SizeOfRawData;

	/*if (NULL != code &&
		code->VirtualAddress == (text->VirtualAddress + text->Misc.VirtualSize) &&
		code->VirtualAddress > text->VirtualAddress) {
		//code_end                = code->VirtualAddress + code->SizeOfRawData;
		//m->flag[code->VirtualAddress] |= FL_SIGNATURE;
	}*/

	if (m->nt64->OptionalHeader.AddressOfEntryPoint) {
		m->flag[m->nt64->OptionalHeader.AddressOfEntryPoint] |= FL_ENTRY;
	}

	return 0;
}

int SavePE64(char *file, Bent *m) {
	return 0;
}


int LoadPE64(Bent *b) {
	return LoadPE64(b->physMem, b->physSize, b);
}
