#ifndef CGEN_H
#define CGEN_H

#include "libtcc.h"

class CCode {
public:
};

class CFunction {
public:
	char *name;
	DWORD rva;
	HOOY *start, *end;
	Vector<char*> codes;
	char *code;
};

class CGen {
public:
	CGen();
	~CGen();
	int AddCode(char *code); // immediately compile code into TCC
	int Compile(); // compile remaining codes, and load into Bent*
	int Reverse();
	int Optimize();
	DWORD FindSymbol(char *sym);
	void Insert(HOOY *at);
	//int ConvertToC(HOOY *start, HOOY *end);

	BYTE *output;
	DWORD outputSize;
	
	HOOY *start, *end;

	TCCState *s;
	Bent     *b;

	Vector<char*> codes;
};

void strcatf_s(char *buf, size_t len, char *fmt, ...);

#endif
