#ifndef HOOY_H
#define HOOY_H

#define FL_RVA               0x00000001
#define FL_DELTA             0x00000002
#define FL_FIXUP             0x00000004
#define FL_LABEL             0x00000008
#define FL_OPCODE            0x00000010
#define FL_CODE              0x00000020
#define FL_HAVEREL           0x00000040
#define FL_CREF              0x00000080
#define FL_DREF              0x00000100
#define FL_ENTRY             0x00000200
#define FL_ANALYZED          0x00000400
#define FL_SECTALIGN         0x00000800 // new section
#define FL_PHYS              0x00001000 // FL_RVA/FL_DELTA modifier, use offs instead of rva
#define FL_PRESENT           0x00002000 // physically present
#define FL_VPRESENT          0x00004000 // virtually present
#define FL_FORCEOBJALIGN     0x00008000 // used for sizeofcode & sizeofidata
#define FL_FORCEFILEALIGN    0x00010000 // used for section[i].physsize
#define FL_DATA              0x00020000
#define FL_STOP              0x00040000 // JMP/RET-alike
#define FL_SIGNATURE         0x00080000 // should be set by sigman()
#define FL_EXECUTABLE        0x00100000 // executable
#define FL_GENERATED         0x00200000 // meta-generated
#define FL_IMPORT            0x00400000 // pointer to imported func
#define FL_SWITCH            0x00800000 // switch/jmp table
#define FL_SWITCHC           0x01000000 // switch condition table
#define FL_FALIGN            0x02000000 // 0xF align
#define FL_ARVA              0x04000000 // absolute (+imagebase)
#define FL_COMMENT           0x08000000 // has text comment
#define FL_ERROR             0x10000000 // intersected objects or bad ptr
#define FL_rsrv1             0x20000000 // this and prev entries are parts of the same instruction
#define FL_HEADER            0x40000000 // dataptr is in header, not alloc()
#define FL_RES8              0x80000000 // high bit set, ==0x80000000
#define FL_ALL               0xFFFFFFFF

class HOOY {
public:
	HOOY *prev;
	HOOY *next;

	DWORD  flags;
	DWORD  datalen;
	BYTE  *dataptr;
	size_t oldrva;
	size_t oldofs;
	size_t arg1;
	size_t arg2;
	size_t arg3;
	size_t arg4;
	size_t newrva;
	size_t newofs;
	char  *cmt;
	Vector<HOOY*, 1> *xref;

	XDA *diza;
};

class CHooyList : public list {
public:
	CHooyList() : list(sizeof(HOOY)) {
		//on_free = CHooyListOnFree;
	}
};

int LoadHooy(CHooyList *l, void *p, size_t len);
int LoadHooy(CHooyList *l, char *fName);
int SaveHooy(CHooyList *l, void **p, size_t *len);
int SaveHooy(CHooyList *l, char *fName);

#endif
