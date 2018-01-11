#ifndef BENT_H
#define BENT_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include "list.h"
#include "xda.h"
#include "_vector.h"
#include "hooy.h"

enum {
	FT_PE,
	FT_PE64,
	FT_BIN,
	FT_BIN64
	//FT_OBJ/COFF
	//FT_ELF
	//FT_LIB
};

#define MakePtr(cast, ptr, addValue)   (cast)((DWORD_PTR)(ptr) + (DWORD_PTR)(addValue))
#define MakeDelta(cast, ptr, subValue) (cast)((DWORD_PTR)(ptr) - (DWORD_PTR)(subValue))
#define ALIGN(x, y)                    (((x) + (y) - 1) & (~((y) - 1)))
//#define ALIGN(x, y)                    (((x) + (y) - 1) - ((x) + (y) - 1) % (y))
#define MIN(x, y)                      ((x) < (y) ? (x) : (y))
#define MAX(x, y)                      ((x) > (y) ? (x) : (y))
#define SAFE_FREE(x)                   if (x) { free(x); }

#define memw   *(WORD*)&memb
#define memd   *(DWORD*)&memb
#define memd64 *(size_t*)&memb

enum {
	SYM_DATA,
	SYM_CODE
};

typedef struct {
	size_t rva;
	size_t size;
	char  *name;
	int    type;
} Symbol;

typedef struct {
	char *moduleName, *importName;
	size_t moduleRVA, nameRVA, impRVA;
} Import;

typedef struct {
	// TODO: code block recognition/listing, as in old vers...
} Block;

#define M_PATCH     0x1 // patch mode, original PE data is only patched instead of rebuilt . . .

class Bent {
public:
	Bent();
	~Bent();
	
	int LoadMemory(BYTE *p, DWORD len, void *base = NULL);
	int LoadFile(char *_path);
	int Analyze();
	int MakeList();
	int Assemble();
	int SaveFile(char *_path);

	void DumpRVA(size_t rva);
	void DumpBinary();
	void DumpList();
	void DumpSymbols();

	XDA *DisassembleOpcode(size_t rva);
	XDA *DisassembleOpcode64(size_t rva);
	HOOY *GetHOOYByOldRVA(size_t rva, DWORD flags);
	Symbol *GetSymbolForRVA(size_t rva);
	DWORD GenRVA();

	int AllocBuffers(size_t _size);
	
	void MarkARVA(size_t rva);
	void MarkRVA(size_t rva);
	void MarkRVAData(size_t rva);
	void MarkFixup(size_t rva);
	
	void MarkARVA64(size_t rva);
	void MarkRVA64(size_t rva);
	void MarkRVAData64(size_t rva);
	void MarkFixup64(size_t rva);

	void MarkDelta(size_t x, size_t y, size_t z);
	void MarkData(size_t rva, size_t len);
	void SetComment(size_t rva, char *cmt);
	void SetCommentf(size_t rva, char *fmt, ...);

	DWORD flags; // engine options M_*
	char *path;
	union {
		BYTE             *physMem;
		IMAGE_DOS_HEADER *mz;
	};
	size_t physSize;
	size_t nrva;

	int    fileType;
	int    isX64, archWordSize, errorCount;

	union {
		IMAGE_NT_HEADERS32 *nt;
		IMAGE_NT_HEADERS64 *nt64;
	};

	BYTE  *memb; // virtual memory
	size_t size; // virtual size
	void  *imageBase;

	size_t overlayRVA, overlaySize;

	DWORD *flag;
	DWORD *arg1;
	DWORD *arg2;
	XDA  **insn;
	char **comment;

	CHooyList      hooyList;
	Vector<Symbol> symbols;
	Vector<Import> imports;
	Vector<size_t> fixups;

	BYTE  *outputMem;
	size_t outputSize;
};

#endif
