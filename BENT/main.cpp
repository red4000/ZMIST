#include <stdio.h>
#include "bent.h"
#include "log.h"
#include "mutate.h"
#include "patch.h"
#include "mt19937.h"
#include "main.h"
#include "binterface.h"

int main(int argc, char *argv[]) {
	InitializeLog();
	InitializePRNG();

	//return ParseArgs(argc, argv);

	Bent bent;

	/*
	int tRes = bent.LoadFile("_pcdll.dll");
	tRes = bent.Analyze();
	tRes = bent.MakeList();
	bent.DumpList();
	Sleep(1500);
	*/
	//bent.flags |= M_PATCH;

	//int res = bent.LoadFile("pcdll.dll");
	//int res = bent.LoadFile("dtest.dll");

	char *inputFile = "ctest.exe", *outputFile = "_out.exe";
	if (argc > 1) {
		inputFile = argv[1];
		if (argc > 2) {
			outputFile = argv[2];
		}
	}

	int res = bent.LoadFile(inputFile);
	//int res = bent.LoadFile("fent.exe");
	//int res = bent.LoadFile("hello.exe");
	CHECKRES("LoadFile");

	res = bent.Analyze();
	CHECKRES("Analyze");

	res = bent.MakeList();
	CHECKRES("MakeList");

	for (HOOY *h = (HOOY*)bent.hooyList.root; h != NULL; h = h->next) {
		if (h->flags & FL_STOP) {
			if (h->prev->flags & FL_HAVEREL) {
				// this int3 is after a call, so assume call is noreturn
			} else {
				if (h->dataptr[0] == 0xCC) {
					printf("Anomaly at %08X %02X\n", h->oldrva, h->dataptr[0]);
				}
			}
		}
		if (h->flags & FL_EXECUTABLE) {
			if (0 == (h->flags & (FL_FIXUP | FL_IMPORT | FL_SWITCH /*| FL_DATA*/ | FL_LABEL))) {
				if (0 == (h->flags & FL_OPCODE)) {
					int isAlignment = 1;
					for (size_t i = 0; i < h->datalen; i++) {
						if (h->dataptr[i] != 0xCC) {
							isAlignment = 0;
							break;
						}
					}
					if (!isAlignment) {
						char dText[32];
						for (size_t i = 0; i < 4; i++) {
							size_t o = i * 3;
							if (i < h->datalen) {
								sprintf_s(&dText[o], sizeof(dText) - o, "%02X ", h->dataptr[i]);
							} else {
								sprintf_s(&dText[o], sizeof(dText) - o, "?? ");
							}
						}
						DEBUG_LOG("Possibly unmarked code at %08X size %X (%i) : %s\n", h->oldrva, h->datalen, h->datalen, dText);
					}
				}
			}
		}
	}

	res = Mutate86(&bent);
	CHECKRES("Mutate");

	if (bent.flags & M_PATCH) {
		res = PatchX86(&bent);
		CHECKRES("Patch");
	} else {
		res = bent.Assemble();
		CHECKRES("Assemble");
	}

	//res = bent.SaveFile("_pcdll.dll");
	//res = bent.SaveFile("_dtest.dll");
	res = bent.SaveFile(outputFile);
	//res = bent.SaveFile("__hello.exe");
	CHECKRES("SaveFile");

	bent.DumpBinary();
	bent.DumpList();

	return 0;
}
