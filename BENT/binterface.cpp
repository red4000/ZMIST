#include <stdio.h>
#include "bent.h"
#include "log.h"
#include "mutate.h"
#include "main.h"

int ParseArgs(int argc, char *argv[]) {
	Bent  b;
	char *inFile  = NULL, *outFile = NULL, *logFile = "BentLog.txt", *inHooy = NULL, *outHooy = NULL;
	int disasm = 1, makeList = 1, mutate = 1, reasm = 1, dumpBin = 0, dumpList = 1;

	if (argc <= 2) {
		printf("usage: fent [-i inputFile] [-o outputFile] [-l logFile] [-lh hooyFile] [-oh hooyFile] [-n<d,l,m,a>disasm,list,mutate,asm] [-d<b,l> dump list,binary]\n");
		return 1;
	}

	for (int i = 1; i < argc; i++) {
		switch (argv[i][1]) {
		case 'i':
			inFile = argv[i + 1];
			break;

		case 'o':
			if (argv[i][2] == 'h') {
				outHooy = argv[i + 1];
			} else {
				outFile = argv[i + 1];
			}
			break;

		case 'l':
			if (argv[i][2] == 'h') {
				inHooy = argv[i + 1];
			} else {
				logFile = argv[i + 1];
				strcpy_s(lPath, sizeof(lPath), logFile);
			}
			break;

		case 'n':
			switch (argv[i][2]) {
			case 'd':
				disasm = 0;
				break;

			case 'l':
				makeList = 0;
				break;

			case 'm':
				mutate = 0;
				break;
				
			case 'a':
				reasm = 0;
				break;
			}
			break;

		case 'd':
			switch (argv[i][2]) {
			case 'b':
				dumpBin = 1;
				break;

			case 'l':
				dumpList = 1;
				break;
			}
			break;
		}
	}

#ifdef _X86_
	DEBUG_LOG("Bent x86\n");
#else
	SYSTEMTIME lt;
	GetLocalTime(&lt);
	DEBUG_LOG("Bent x64 %02i:%02i:%04i %02i:%02i:%02i.%i\n", lt.wMonth, lt.wDay, lt.wYear, lt.wHour, lt.wMinute, lt.wSecond, lt.wMilliseconds);
#endif

	int res = 0;
	if (inFile) {
		b.LoadFile(inFile);
		CHECKRES("Bent::LoadFile");
	}

	if (inHooy) {
		res = LoadHooy(&b.hooyList, inHooy);
		CHECKRES("LoadHooy");
	}
	
	if (disasm) {
		res = b.Analyze();
		CHECKRES("Bent::Analyze");
	}

	if (makeList) {
		res = b.MakeList();
		CHECKRES("Bent::MakeList");
	}

	if (mutate) {
		res = Mutate86(&b);
		CHECKRES("Mutate");
	}

	if (reasm) {
		res = b.Assemble();
		CHECKRES("Assemble");
	}

	if (outHooy) {
		res = SaveHooy(&b.hooyList, outHooy);
		CHECKRES("SaveHooy");
	}

	if (outFile) {
		res = b.SaveFile(outFile);
		CHECKRES("SaveFile");
	}

	if (dumpBin) {
		b.DumpBinary();
	}

	if (dumpList) {
		b.DumpList();
	}

	return 0;
}
