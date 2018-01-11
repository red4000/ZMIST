#define _CRT_SECURE_NO_WARNINGS
#include "bent.h"
#include "dbginfo.h"
#define _NO_CVCONST_H
#include <dbghelp.h>
#include <stdio.h>
#include "log.h"

typedef struct {
	Bent *m;
} SymbolInfo;

BOOL CALLBACK EnumSymbolsCallback(PSYMBOL_INFO pSymInfo, ULONG SymbolSize, PVOID UserContext) {
	if (0 == SymbolSize) {
		return TRUE;
	}

	Symbol sym;
	size_t i = 0;
	char *sName;

	switch (pSymInfo->Tag) {
	case SymTagFunction:
	case SymTagThunk:
	case SymTagBlock:
		sym.rva  = (size_t)(pSymInfo->Address - pSymInfo->ModBase);
		sym.size = (size_t)pSymInfo->Size;
		if (pSymInfo->NameLen) {
			sym.name = (char*)malloc(pSymInfo->NameLen + 1);
			strcpy(sym.name, pSymInfo->Name);
		} else {
			sym.name = "";
		}
		sym.type = SYM_CODE;

		((SymbolInfo*)UserContext)->m->flag[sym.rva] |= FL_SIGNATURE;
		for (size_t i = 0; i < sym.size; i++) {
			//((SymbolInfo*)UserContext)->m->flag[sym.rva + i] |= FL_KNOWNCODE;
		}

		((SymbolInfo*)UserContext)->m->symbols.push_back(sym);
		//Log("%08X: %04i %s\n", sym.rva, sym.size, sym.name);
		break;

	case SymTagData:
		if (pSymInfo->Address < pSymInfo->ModBase) {
			return TRUE;
		}
		sym.rva  = (size_t)(pSymInfo->Address - pSymInfo->ModBase);
		sym.size = (size_t)pSymInfo->Size;
		if (pSymInfo->NameLen) {
			sym.name = (char*)malloc(pSymInfo->NameLen + 1);
			strcpy(sym.name, pSymInfo->Name);
		} else {
			sym.name = "";
		}
		sym.type = SYM_DATA;

		((SymbolInfo*)UserContext)->m->symbols.push_back(sym);

		for (size_t i = 0; i < SymbolSize; i++) {
			((SymbolInfo*)UserContext)->m->flag[(pSymInfo->Address - pSymInfo->ModBase) + i] |= FL_DATA;
		}

		break;

	default:
		if (pSymInfo->NameLen) {
			sName = &pSymInfo->Name[0];
		} else {
			sName = "";
		}
		DEBUG_LOG("Unknown symbol %i %s\n", pSymInfo->Tag, sName);
		break;
	}

	return TRUE;
}

int LoadSymbols(Bent *b) {
	int res = 0;
	HANDLE     hProcess = GetCurrentProcess();
	DWORD64    BaseOfDll;
	BOOL       status;
	SymbolInfo info;

	info.m = b;

	status = SymInitialize(hProcess, NULL, FALSE);

	if (status == FALSE) {
		return 1;
	}

	BaseOfDll = SymLoadModuleEx(hProcess, NULL, b->path, NULL, 0, 0, NULL, 0);

	if (BaseOfDll == 0) {
		SymCleanup(hProcess);
		return 2;
	}

	if (SymEnumSymbols(hProcess, BaseOfDll, 0, EnumSymbolsCallback, (PVOID)&info)) {
		DEBUG_LOG("Loaded %i symbols\n", b->symbols.size());
	} else {
		DEBUG_LOG("Error: SymEnumSymbols failed: %d\n", GetLastError());
		SymCleanup(hProcess);
		return 3;
	}

	SymCleanup(hProcess);
	return res;
}
