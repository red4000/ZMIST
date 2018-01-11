#ifndef XDA_H
#define XDA_H

#define BEA_ENGINE_STATIC
#define BEA_USE_STDCALL
#include "BeaEngine.h"

#define C_ERROR      0x00000400
#define C_BAD        0x00000800  /* "bad", i.e. rarely used instruction     */
#define C_REL        0x00001000  /* it is jxx/call/...                      */
#define C_STOP       0x00002000  /* it is ret/jmp/...                       */
#define C_JMP        0x00010000  /* it is jmp                               */
#define C_ANALYZED   0x00004000

typedef struct {
	DISASM d;
	int    len;
	DWORD  flag, srcSet, dstSet;
} XDA;

int XDisasm(XDA *da, BYTE *p, size_t va);
int XDisasm64(XDA *da, BYTE *p, size_t va);

#endif
