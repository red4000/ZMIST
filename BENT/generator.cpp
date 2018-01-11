#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <algorithm>
#include <assert.h>
#include "bent.h"
#include "log.h"
#include "assemble.h"
#include "generator.h"

void GenOpRC(DISASM *out, char *mnemonic, int reg, size_t imm) {
	memset((void*)out, 0, sizeof(DISASM));
	strcpy_s(out->Instruction.Mnemonic, sizeof(out->Instruction.Mnemonic), mnemonic);
	out->Argument1.ArgType    = REGISTER_TYPE | reg;
	out->Argument2.ArgType    = CONSTANT_TYPE;
	out->Instruction.Immediat = imm;
}

void GenOpRR(DISASM *out, char *mnemonic, int reg, int reg2) {
	memset((void*)out, 0, sizeof(DISASM));
	strcpy_s(out->Instruction.Mnemonic, sizeof(out->Instruction.Mnemonic), mnemonic);
	out->Argument1.ArgType = REGISTER_TYPE | reg;
	out->Argument2.ArgType = REGISTER_TYPE | reg2;
}

void GenOpRM(DISASM *out, char *mnemonic, int reg, int baseReg, int indexReg, int scale, size_t displacement) {
	memset((void*)out, 0, sizeof(DISASM));
	strcpy_s(out->Instruction.Mnemonic, sizeof(out->Instruction.Mnemonic), mnemonic);
	out->Argument1.ArgType              = REGISTER_TYPE | reg;
	out->Argument2.ArgType              = MEMORY_TYPE;
	out->Argument2.Memory.BaseRegister  = baseReg;
	out->Argument2.Memory.IndexRegister = indexReg;
	out->Argument2.Memory.Scale         = scale;
	out->Argument2.Memory.Displacement  = displacement;
}

void GenOpMR(DISASM *out, char *mnemonic, int baseReg, int indexReg, int scale, size_t displacement, int reg) {
	memset((void*)out, 0, sizeof(DISASM));
	strcpy_s(out->Instruction.Mnemonic, sizeof(out->Instruction.Mnemonic), mnemonic);
	out->Argument1.ArgType              = MEMORY_TYPE;
	out->Argument1.Memory.BaseRegister  = baseReg;
	out->Argument1.Memory.IndexRegister = indexReg;
	out->Argument1.Memory.Scale         = scale;
	out->Argument1.Memory.Displacement  = displacement;
	out->Argument2.ArgType              = REGISTER_TYPE | reg;
}

void GenOpMC(DISASM *out, char *mnemonic, int baseReg, int indexReg, int scale, size_t displacement, size_t imm) {
	memset((void*)out, 0, sizeof(DISASM));
	strcpy_s(out->Instruction.Mnemonic, sizeof(out->Instruction.Mnemonic), mnemonic);
	out->Argument1.ArgType              = MEMORY_TYPE;
	out->Argument1.Memory.BaseRegister  = baseReg;
	out->Argument1.Memory.IndexRegister = indexReg;
	out->Argument1.Memory.Scale         = scale;
	out->Argument1.Memory.Displacement  = displacement;
	out->Argument2.ArgType              = CONSTANT_TYPE;
	out->Instruction.Immediat           = imm;
}

void GenOpR(DISASM *out, char *mnemonic, int reg) {
	memset((void*)out, 0, sizeof(DISASM));
	strcpy_s(out->Instruction.Mnemonic, sizeof(out->Instruction.Mnemonic), mnemonic);
	out->Argument1.ArgType = REGISTER_TYPE | reg;
}

void GenOpM(DISASM *out, char *mnemonic, int baseReg, int indexReg, int scale, size_t displacement) {
	memset((void*)out, 0, sizeof(DISASM));
	strcpy_s(out->Instruction.Mnemonic, sizeof(out->Instruction.Mnemonic), mnemonic);
	out->Argument1.ArgType              = MEMORY_TYPE;
	out->Argument1.Memory.BaseRegister  = baseReg;
	out->Argument1.Memory.IndexRegister = indexReg;
	out->Argument1.Memory.Scale         = scale;
	out->Argument1.Memory.Displacement  = displacement;
}

void GenOpC(DISASM *out, char *mnemonic, size_t imm) {
	memset((void*)out, 0, sizeof(DISASM));
	strcpy_s(out->Instruction.Mnemonic, sizeof(out->Instruction.Mnemonic), mnemonic);
	out->Argument1.ArgType             = CONSTANT_TYPE;
	out->Instruction.Immediat          = imm;
}

void GenOp(DISASM *out, char *mnemonic) {
	memset((void*)out, 0, sizeof(DISASM));
	strcpy_s(out->Instruction.Mnemonic, sizeof(out->Instruction.Mnemonic), mnemonic);
}

void GenOpRA(DISASM *out, char *mnemonic, int reg, ARGTYPE *a, INSTRTYPE *i) {
	memset((void*)out, 0, sizeof(DISASM));
	strcpy_s(out->Instruction.Mnemonic, sizeof(out->Instruction.Mnemonic), mnemonic);
	out->Argument1.ArgType = REGISTER_TYPE | reg;
	out->Argument2         = *a;
	if (a->ArgType & CONSTANT_TYPE) {
		out->Instruction.Immediat = i->Immediat;
	}
}

void GenOpMA(DISASM *out, char *mnemonic, int baseReg, int indexReg, int scale, size_t displacement, ARGTYPE *a, INSTRTYPE *i) {
	memset((void*)out, 0, sizeof(DISASM));
	strcpy_s(out->Instruction.Mnemonic, sizeof(out->Instruction.Mnemonic), mnemonic);
	out->Argument1.ArgType              = MEMORY_TYPE;
	out->Argument1.Memory.BaseRegister  = baseReg;
	out->Argument1.Memory.IndexRegister = indexReg;
	out->Argument1.Memory.Scale         = scale;
	out->Argument1.Memory.Displacement  = displacement;
	out->Argument2                      = *a;
	if (a->ArgType & CONSTANT_TYPE) {
		out->Instruction.Immediat       = i->Immediat;
	}
}

void GenOpAC(DISASM *out, char *mnemonic, ARGTYPE *a, INSTRTYPE *i, size_t imm) {
	memset((void*)out, 0, sizeof(DISASM));
	strcpy_s(out->Instruction.Mnemonic, sizeof(out->Instruction.Mnemonic), mnemonic);
	out->Argument1                     = *a;
	out->Argument2.ArgType             = CONSTANT_TYPE;
	out->Instruction.Immediat          = imm;
}

void GenOpAR(DISASM *out, char *mnemonic, ARGTYPE *a, INSTRTYPE *i, int reg) {
	memset((void*)out, 0, sizeof(DISASM));
	strcpy_s(out->Instruction.Mnemonic, sizeof(out->Instruction.Mnemonic), mnemonic);
	out->Argument1         = *a;
	out->Argument2.ArgType = REGISTER_TYPE | reg;
}

void GenOpAM(DISASM *out, char *mnemonic, ARGTYPE *a, INSTRTYPE *i, int baseReg, int indexReg, int scale, size_t displacement) {
	memset((void*)out, 0, sizeof(DISASM));
	strcpy_s(out->Instruction.Mnemonic, sizeof(out->Instruction.Mnemonic), mnemonic);
	out->Argument1 = *a;
	out->Argument2.ArgType              = MEMORY_TYPE;
	out->Argument2.Memory.BaseRegister  = baseReg;
	out->Argument2.Memory.IndexRegister = indexReg;
	out->Argument2.Memory.Scale         = scale;
	out->Argument2.Memory.Displacement  = displacement;
}

void GenOpAA(DISASM *out, char *mnemonic, ARGTYPE *a1, INSTRTYPE *i1, ARGTYPE *a2, INSTRTYPE *i2) {
	memset((void*)out, 0, sizeof(DISASM));
	strcpy_s(out->Instruction.Mnemonic, sizeof(out->Instruction.Mnemonic), mnemonic);
	out->Argument1 = *a1;
	out->Argument2 = *a2;
	if (a2->ArgType & CONSTANT_TYPE) {
		out->Instruction.Immediat = i2->Immediat;
	}
}

void GenOpA(DISASM *out, char *mnemonic, ARGTYPE *a, INSTRTYPE *i) {
	memset((void*)out, 0, sizeof(DISASM));
	strcpy_s(out->Instruction.Mnemonic, sizeof(out->Instruction.Mnemonic), mnemonic);
	out->Argument1 = *a;
	if (a->ArgType & CONSTANT_TYPE) {
		out->Instruction.Immediat = i->Immediat;
	}
}

HOOY *GenHOpRC(char *mnemonic, int reg, size_t imm) {
	HOOY *result = (HOOY*)malloc(sizeof(HOOY));
	memset(result, 0, sizeof(HOOY));
	result->flags = FL_OPCODE | FL_CODE | FL_EXECUTABLE | FL_PRESENT | FL_VPRESENT | FL_GENERATED;
	result->diza  = (XDA*)malloc(sizeof(XDA));
	memset(result->diza, 0, sizeof(XDA));
	GenOpRC(&result->diza->d, mnemonic, reg, imm);
	void *buffer;
	result->datalen                = AssembleInstruction(&result->diza->d, &buffer);
	if (buffer != NULL) {
		result->dataptr                = (BYTE*)buffer;
		result->diza->d.SecurityBlock = result->datalen;
		int len = XDisasm(result->diza, result->dataptr, 0);
		return result;
	} else {
		free(result);
		return NULL;
	}
}

HOOY *GenHOpRR(char *mnemonic, int reg, int reg2) {
	HOOY *result = (HOOY*)malloc(sizeof(HOOY));
	memset(result, 0, sizeof(HOOY));
	result->flags = FL_OPCODE | FL_CODE | FL_EXECUTABLE | FL_PRESENT | FL_VPRESENT | FL_GENERATED;
	result->diza  = (XDA*)malloc(sizeof(XDA));
	memset(result->diza, 0, sizeof(XDA));
	GenOpRR(&result->diza->d, mnemonic, reg, reg2);
	void *buffer;
	result->datalen = AssembleInstruction(&result->diza->d, &buffer);
	if (buffer != NULL) {
		result->dataptr                = (BYTE*)buffer;
		result->diza->d.SecurityBlock = result->datalen;
		int len = XDisasm(result->diza, result->dataptr, 0);
		return result;
	} else {
		free(result);
		return NULL;
	}
}

HOOY *GenHOpRM(char *mnemonic, int reg, int baseReg, int indexReg, int scale, size_t displacement) {
	HOOY *result = (HOOY*)malloc(sizeof(HOOY));
	memset(result, 0, sizeof(HOOY));
	result->flags = FL_OPCODE | FL_CODE | FL_EXECUTABLE | FL_PRESENT | FL_VPRESENT | FL_GENERATED;
	result->diza  = (XDA*)malloc(sizeof(XDA));
	memset(result->diza, 0, sizeof(XDA));
	GenOpRM(&result->diza->d, mnemonic, reg, baseReg, indexReg, scale, displacement);
	void *buffer;
	result->datalen                = AssembleInstruction(&result->diza->d, &buffer);
	if (buffer != NULL) {
		result->dataptr                = (BYTE*)buffer;
		result->diza->d.SecurityBlock = result->datalen;
		int len = XDisasm(result->diza, result->dataptr, 0);
		return result;
	} else {
		free(result);
		return NULL;
	}
}

HOOY *GenHOpMR(char *mnemonic, int baseReg, int indexReg, int scale, size_t displacement, int reg) {
	HOOY *result = (HOOY*)malloc(sizeof(HOOY));
	memset(result, 0, sizeof(HOOY));
	result->flags = FL_OPCODE | FL_CODE | FL_EXECUTABLE | FL_PRESENT | FL_VPRESENT | FL_GENERATED;
	result->diza  = (XDA*)malloc(sizeof(XDA));
	memset(result->diza, 0, sizeof(XDA));
	GenOpMR(&result->diza->d, mnemonic, baseReg, indexReg, scale, displacement, reg);
	void *buffer;
	result->datalen                = AssembleInstruction(&result->diza->d, &buffer);
	if (buffer != NULL) {
		result->dataptr                = (BYTE*)buffer;
		result->diza->d.SecurityBlock = result->datalen;
		int len = XDisasm(result->diza, result->dataptr, 0);
		return result;
	} else {
		free(result);
		return NULL;
	}
}

HOOY *GenHOpMC(char *mnemonic, int baseReg, int indexReg, int scale, size_t displacement, size_t imm) {
	HOOY *result = (HOOY*)malloc(sizeof(HOOY));
	memset(result, 0, sizeof(HOOY));
	result->flags = FL_OPCODE | FL_CODE | FL_EXECUTABLE | FL_PRESENT | FL_VPRESENT | FL_GENERATED;
	result->diza  = (XDA*)malloc(sizeof(XDA));
	memset(result->diza, 0, sizeof(XDA));
	GenOpMC(&result->diza->d, mnemonic, baseReg, indexReg, scale, displacement, imm);
	void *buffer;
	result->datalen                = AssembleInstruction(&result->diza->d, &buffer);
	if (buffer != NULL) {
		result->dataptr                = (BYTE*)buffer;
		result->diza->d.SecurityBlock = result->datalen;
		int len = XDisasm(result->diza, result->dataptr, 0);
		return result;
	} else {
		free(result);
		return NULL;
	}
}

HOOY *GenHOpR(char *mnemonic, int reg) {
	HOOY *result = (HOOY*)malloc(sizeof(HOOY));
	memset(result, 0, sizeof(HOOY));
	result->flags = FL_OPCODE | FL_CODE | FL_EXECUTABLE | FL_PRESENT | FL_VPRESENT | FL_GENERATED;
	result->diza  = (XDA*)malloc(sizeof(XDA));
	memset(result->diza, 0, sizeof(XDA));
	GenOpR(&result->diza->d, mnemonic, reg);
	void *buffer;
	result->datalen                = AssembleInstruction(&result->diza->d, &buffer);
	if (buffer != NULL) {
		result->dataptr                = (BYTE*)buffer;
		result->diza->d.SecurityBlock = result->datalen;
		int len = XDisasm(result->diza, result->dataptr, 0);
		return result;
	} else {
		free(result);
		return NULL;
	}
}

HOOY *GenHOpM(char *mnemonic, int baseReg, int indexReg, int scale, size_t displacement) {
	HOOY *result = (HOOY*)malloc(sizeof(HOOY));

	memset(result, 0, sizeof(HOOY));
	result->flags = FL_OPCODE | FL_CODE | FL_EXECUTABLE | FL_PRESENT | FL_VPRESENT | FL_GENERATED;
	result->diza  = (XDA*)malloc(sizeof(XDA));
	memset(result->diza, 0, sizeof(XDA));
	GenOpM(&result->diza->d, mnemonic, baseReg, indexReg, scale, displacement);
	void *buffer;
	result->datalen                = AssembleInstruction(&result->diza->d, &buffer);
	if (buffer != NULL) {
		result->dataptr                = (BYTE*)buffer;
		result->diza->d.SecurityBlock = result->datalen;
		int len = XDisasm(result->diza, result->dataptr, 0);
		return result;
	} else {
		free(result);
		return NULL;
	}
}

HOOY *GenHOpC(char *mnemonic, size_t imm) {
	HOOY *result = (HOOY*)malloc(sizeof(HOOY));

	memset(result, 0, sizeof(HOOY));
	result->flags = FL_OPCODE | FL_CODE | FL_EXECUTABLE | FL_PRESENT | FL_VPRESENT | FL_GENERATED;
	result->diza  = (XDA*)malloc(sizeof(XDA));
	memset(result->diza, 0, sizeof(XDA));
	GenOpC(&result->diza->d, mnemonic, imm);
	void *buffer;
	result->datalen                = AssembleInstruction(&result->diza->d, &buffer);
	if (buffer != NULL) {
		result->dataptr                = (BYTE*)buffer;
		result->diza->d.SecurityBlock = result->datalen;
		int len = XDisasm(result->diza, result->dataptr, 0);
		return result;
	} else {
		free(result);
		return NULL;
	}
}

HOOY *GenHOp(char *mnemonic) {
	HOOY *result = (HOOY*)malloc(sizeof(HOOY));

	memset(result, 0, sizeof(HOOY));
	result->flags = FL_OPCODE | FL_CODE | FL_EXECUTABLE | FL_PRESENT | FL_VPRESENT | FL_GENERATED;
	result->diza  = (XDA*)malloc(sizeof(XDA));
	memset(result->diza, 0, sizeof(XDA));
	GenOp(&result->diza->d, mnemonic);
	void *buffer;
	result->datalen                = AssembleInstruction(&result->diza->d, &buffer);
	if (buffer != NULL) {
		result->dataptr                = (BYTE*)buffer;
		result->diza->d.SecurityBlock = result->datalen;
		int len = XDisasm(result->diza, result->dataptr, 0);
		return result;
	} else {
		free(result);
		return NULL;
	}
}

HOOY *GenHOpRA(char *mnemonic, int reg, ARGTYPE *a, INSTRTYPE *i) {
	HOOY *result = (HOOY*)malloc(sizeof(HOOY));

	memset(result, 0, sizeof(HOOY));
	result->flags = FL_OPCODE | FL_CODE | FL_EXECUTABLE | FL_PRESENT | FL_VPRESENT | FL_GENERATED;
	result->diza  = (XDA*)malloc(sizeof(XDA));
	memset(result->diza, 0, sizeof(XDA));
	GenOpRA(&result->diza->d, mnemonic, reg, a, i);
	void *buffer;
	result->datalen                = AssembleInstruction(&result->diza->d, &buffer);
	if (buffer != NULL) {
		result->dataptr                = (BYTE*)buffer;
		result->diza->d.SecurityBlock = result->datalen;
		int len = XDisasm(result->diza, result->dataptr, 0);
		return result;
	} else {
		free(result);
		return NULL;
	}
}

HOOY *GenHOpMA(char *mnemonic, int baseReg, int indexReg, int scale, size_t displacement, ARGTYPE *a, INSTRTYPE *i) {
	HOOY *result = (HOOY*)malloc(sizeof(HOOY));

	memset(result, 0, sizeof(HOOY));
	result->flags = FL_OPCODE | FL_CODE | FL_EXECUTABLE | FL_PRESENT | FL_VPRESENT | FL_GENERATED;
	result->diza  = (XDA*)malloc(sizeof(XDA));
	memset(result->diza, 0, sizeof(XDA));
	GenOpMA(&result->diza->d, mnemonic, baseReg, indexReg, scale, displacement, a, i);
	void *buffer;
	result->datalen                = AssembleInstruction(&result->diza->d, &buffer);
	if (buffer != NULL) {
		result->dataptr                = (BYTE*)buffer;
		result->diza->d.SecurityBlock = result->datalen;
		int len = XDisasm(result->diza, result->dataptr, 0);
		return result;
	} else {
		free(result);
		return NULL;
	}
}

HOOY *GenHOpAC(char *mnemonic, ARGTYPE *a, INSTRTYPE *i, size_t imm) {
	HOOY *result = (HOOY*)malloc(sizeof(HOOY));

	memset(result, 0, sizeof(HOOY));
	result->flags = FL_OPCODE | FL_CODE | FL_EXECUTABLE | FL_PRESENT | FL_VPRESENT | FL_GENERATED;
	result->diza  = (XDA*)malloc(sizeof(XDA));
	memset(result->diza, 0, sizeof(XDA));
	GenOpAC(&result->diza->d, mnemonic, a, i, imm);
	void *buffer;
	result->datalen                = AssembleInstruction(&result->diza->d, &buffer);
	if (buffer != NULL) {
		result->dataptr                = (BYTE*)buffer;
		result->diza->d.SecurityBlock = result->datalen;
		int len = XDisasm(result->diza, result->dataptr, 0);
		return result;
	} else {
		free(result);
		return NULL;
	}
}

HOOY *GenHOpAR(char *mnemonic, ARGTYPE *a, INSTRTYPE *i, int reg) {
	HOOY *result = (HOOY*)malloc(sizeof(HOOY));

	memset(result, 0, sizeof(HOOY));
	result->flags = FL_OPCODE | FL_CODE | FL_EXECUTABLE | FL_PRESENT | FL_VPRESENT | FL_GENERATED;
	result->diza  = (XDA*)malloc(sizeof(XDA));
	memset(result->diza, 0, sizeof(XDA));
	GenOpAR(&result->diza->d, mnemonic, a, i, reg);
	void *buffer;
	result->datalen                = AssembleInstruction(&result->diza->d, &buffer);
	if (buffer != NULL) {
		result->dataptr                = (BYTE*)buffer;
		result->diza->d.SecurityBlock = result->datalen;
		int len = XDisasm(result->diza, result->dataptr, 0);
		return result;
	} else {
		free(result);
		return NULL;
	}
}

HOOY *GenHOpAM(char *mnemonic, ARGTYPE *a, INSTRTYPE *i, int baseReg, int indexReg, int scale, size_t displacement) {
	HOOY *result = (HOOY*)malloc(sizeof(HOOY));

	memset(result, 0, sizeof(HOOY));
	result->flags = FL_OPCODE | FL_CODE | FL_EXECUTABLE | FL_PRESENT | FL_VPRESENT | FL_GENERATED;
	result->diza  = (XDA*)malloc(sizeof(XDA));
	memset(result->diza, 0, sizeof(XDA));
	GenOpAM(&result->diza->d, mnemonic, a, i, baseReg, indexReg, scale, displacement);
	void *buffer;
	result->datalen                = AssembleInstruction(&result->diza->d, &buffer);
	if (buffer != NULL) {
		result->dataptr                = (BYTE*)buffer;
		result->diza->d.SecurityBlock = result->datalen;
		int len = XDisasm(result->diza, result->dataptr, 0);
		return result;
	} else {
		free(result);
		return NULL;
	}
}

HOOY *GenHOpAA(char *mnemonic, ARGTYPE *a1, INSTRTYPE *i1, ARGTYPE *a2, INSTRTYPE *i2) {
	HOOY *result = (HOOY*)malloc(sizeof(HOOY));

	memset(result, 0, sizeof(HOOY));
	result->flags = FL_OPCODE | FL_CODE | FL_EXECUTABLE | FL_PRESENT | FL_VPRESENT | FL_GENERATED;
	result->diza  = (XDA*)malloc(sizeof(XDA));
	memset(result->diza, 0, sizeof(XDA));
	GenOpAA(&result->diza->d, mnemonic, a1, i1, a2, i2);
	void *buffer;
	result->datalen                = AssembleInstruction(&result->diza->d, &buffer);
	if (buffer != NULL) {
		result->dataptr                = (BYTE*)buffer;
		result->diza->d.SecurityBlock = result->datalen;
		int len = XDisasm(result->diza, result->dataptr, 0);
		return result;
	} else {
		free(result);
		return NULL;
	}
}

HOOY *GenHOpA(char *mnemonic, ARGTYPE *a, INSTRTYPE *i) {
	HOOY *result = (HOOY*)malloc(sizeof(HOOY));

	memset(result, 0, sizeof(HOOY));
	result->flags = FL_OPCODE | FL_CODE | FL_EXECUTABLE | FL_PRESENT | FL_VPRESENT | FL_GENERATED;
	result->diza  = (XDA*)malloc(sizeof(XDA));
	memset(result->diza, 0, sizeof(XDA));
	GenOpA(&result->diza->d, mnemonic, a, i);
	void *buffer;
	result->datalen                = AssembleInstruction(&result->diza->d, &buffer);
	if (buffer != NULL) {
		result->dataptr                = (BYTE*)buffer;
		result->diza->d.SecurityBlock = result->datalen;
		int len = XDisasm(result->diza, result->dataptr, 0);
		return result;
	} else {
		free(result);
		return NULL;
	}
}

HOOY *GenHLabel(Bent *m) {
	HOOY *result = (HOOY*)calloc(1, sizeof(HOOY));
	result->flags  = FL_LABEL | FL_GENERATED;
	result->oldrva = m->GenRVA();
	return result;
}

HOOY *GenHCall(Bent *m, HOOY *dest) {
	HOOY *h = (HOOY*)malloc(sizeof(HOOY));

	memset(h, 0, sizeof(HOOY));
	h->dataptr = (BYTE*)calloc(1, 5);

	if (NULL == h->dataptr) {
		return NULL;
	}

	if (0 == (dest->flags & FL_LABEL)) {
		if (dest->prev) {
			if (dest->prev->flags & FL_LABEL) {
				dest = dest->prev;
			} else {
				HOOY *h2 = (HOOY*)malloc(sizeof(HOOY));
				memset(h2, 0, sizeof(HOOY));
				h->flags  = FL_LABEL | FL_GENERATED;
				h->oldrva = m->GenRVA();
				m->hooyList.insert_before((void*)h2, (void*)dest);
				dest = h2;
			}
		}
	}

	dest->flags |= FL_CREF;
	
	// TODO: if (m->isX64) {
	h->dataptr[0] = 0xE8;
	h->datalen    = 5;
	*(DWORD*)(&h->dataptr[1]) = dest->oldrva - (h->oldrva + h->datalen);
	h->arg1       = dest->oldrva;
	h->arg2       = 4;
	h->flags      = FL_OPCODE | FL_CODE | FL_EXECUTABLE | FL_PRESENT | FL_VPRESENT | FL_HAVEREL | FL_GENERATED;

	h->diza  = (XDA*)calloc(1, sizeof(XDA));
	h->diza->d.SecurityBlock = h->datalen;
	XDisasm(h->diza, h->dataptr, h->oldrva);

	return h;
}

HOOY *GenHCall(Bent *m, int reg) {
	HOOY *result = (HOOY*)malloc(sizeof(HOOY));
	memset(result, 0, sizeof(HOOY));
	result->flags = FL_OPCODE | FL_CODE | FL_EXECUTABLE | FL_PRESENT | FL_VPRESENT | FL_GENERATED;
	result->diza  = (XDA*)malloc(sizeof(XDA));
	memset(result->diza, 0, sizeof(XDA));
	GenOpR(&result->diza->d, "call ", reg);
	void *buffer;
	result->datalen                = AssembleInstruction(&result->diza->d, &buffer);
	if (buffer != NULL) {
		result->dataptr                = (BYTE*)buffer;
		result->diza->d.SecurityBlock = result->datalen;
		int len = XDisasm(result->diza, result->dataptr, 0);
		return result;
	} else {
		free(result);
		return NULL;
	}
}

HOOY *GenHJMP(Bent *m, HOOY *dest) {
	HOOY *h = (HOOY*)calloc(1, sizeof(HOOY));

	h->dataptr = (BYTE*)calloc(1, 5);
	h->oldrva  = m->GenRVA();

	if (NULL == h->dataptr) {
		return NULL;
	}

	if (0 == (dest->flags & FL_LABEL)) {
		if (dest->prev) {
			if (dest->prev->flags & FL_LABEL) {
				dest = dest->prev;
			} else {
				HOOY *h2  = (HOOY*)calloc(1, sizeof(HOOY));
				h->flags  = FL_LABEL | FL_GENERATED;
				h->oldrva = m->GenRVA();
				m->hooyList.insert_before((void*)h2, (void*)dest);
				dest = h2;
			}
		}
	}

	dest->flags |= FL_CREF;

	// TODO: if (m->isX64) {
	h->dataptr[0] = 0xE9;
	h->datalen    = 5;
	*(DWORD*)(&h->dataptr[1]) = dest->oldrva - (h->oldrva + h->datalen);
	h->arg1       = dest->oldrva;
	h->arg2       = 4;
	h->flags      = FL_OPCODE | FL_CODE | FL_EXECUTABLE | FL_PRESENT | FL_VPRESENT | FL_HAVEREL | FL_GENERATED;

	h->diza  = (XDA*)calloc(1, sizeof(XDA));
	h->diza->d.SecurityBlock = h->datalen;
	XDisasm(h->diza, h->dataptr, h->oldrva);

	return h;
}

HOOY *GenHJMP(Bent *m, int reg) {
	HOOY *result = (HOOY*)malloc(sizeof(HOOY));
	memset(result, 0, sizeof(HOOY));
	result->flags = FL_OPCODE | FL_CODE | FL_EXECUTABLE | FL_PRESENT | FL_VPRESENT | FL_GENERATED;
	result->diza  = (XDA*)malloc(sizeof(XDA));
	memset(result->diza, 0, sizeof(XDA));
	GenOpR(&result->diza->d, "jmp ", reg);
	void *buffer;
	result->datalen                = AssembleInstruction(&result->diza->d, &buffer);
	if (buffer != NULL) {
		result->dataptr                = (BYTE*)buffer;
		result->diza->d.SecurityBlock = result->datalen;
		int len = XDisasm(result->diza, result->dataptr, 0);
		return result;
	} else {
		free(result);
		return NULL;
	}
}

HOOY *GenHJCC(Bent *b, int cond, HOOY *dest) {
	HOOY *h = (HOOY*)calloc(1, sizeof(HOOY));
	h->datalen = 6;
	h->dataptr = (BYTE*)calloc(1, h->datalen);

	// TODO: if (m->isX64) {
	h->dataptr[0] = 0x0F;

	switch (cond) {
	case C_JA: // 0F 87 / 77
		h->dataptr[1] = 0x87;
		break;

	case C_JAE: // 0F 83 / 73
		h->dataptr[1] = 0x83;
		break;

	case C_JB: // 0F 82 / 72
		h->dataptr[1] = 0x82;
		break;

	case C_JBE: // 0F 86 / 76
		h->dataptr[1] = 0x86;
		break;

	case C_JC:
		break;

	case C_JE: // 0F 84 / 74
		h->dataptr[1] = 0x84;
		break;

	case C_JNC:
		break;

	case C_JNE: // 0F 85 / 75
		h->dataptr[1] = 0x85;
		break;

	case C_JNP: // 0F 8B / 7B
		h->dataptr[1] = 0x8B;
		break;

	case C_JP: // 0F 8A / 7A
		h->dataptr[1] = 0x8A;
		break;

	case C_JG: // 0F 8F / 7F
		h->dataptr[1] = 0x8F;
		break;

	case C_JGE: // 0F 8D / 7D
		h->dataptr[1] = 0x8D;
		break;

	case C_JL: // 0F 8C / 7C
		h->dataptr[1] = 0x8C;
		break;

	case C_JLE: // 0F 8E / 7E
		h->dataptr[1] = 0x8E;
		break;

	case C_JNO: // 0F 81 / 71
		h->dataptr[1] = 0x81;
		break;

	case C_JNS: // 0F 89 / 79
		h->dataptr[1] = 0x89;
		break;

	case C_JO: // 0F 80 / 70
		h->dataptr[1] = 0x80;
		break;

	case C_JS: // 0F 88 / 78
		h->dataptr[1] = 0x88;
		break;
	}
	h->flags = FL_OPCODE | FL_CODE | FL_EXECUTABLE | FL_PRESENT | FL_VPRESENT | FL_HAVEREL | FL_GENERATED;
	if (dest) {
		dest->flags |= FL_CREF;
		h->arg1      = dest->oldrva;
		h->oldrva    = b->GenRVA();
		*(DWORD*)(&h->dataptr[h->datalen - 4]) = dest->oldrva - (h->oldrva + h->datalen);
	}
	h->arg2 = 4;

	h->diza  = (XDA*)calloc(1, sizeof(XDA));
	h->diza->d.SecurityBlock = h->datalen;
	XDisasm(h->diza, h->dataptr, h->oldrva);
	return h;
}

HOOY *GenHData(BYTE *p, size_t len) {
	HOOY *h    = (HOOY*)calloc(1, sizeof(HOOY));
	h->dataptr = p;
	h->datalen = len;
	h->flags  |= FL_PRESENT | FL_VPRESENT | FL_DATA | FL_GENERATED;
	return h;
}

int ReDisasm(HOOY *h) {
	int res = XDisasm(h->diza, h->dataptr, h->newrva);
	h->datalen = h->diza->len = res;
	return res;
}

GeneratedBlock::GeneratedBlock(Bent *f) {
	Initialize(f);
}

GeneratedBlock::GeneratedBlock() {

}

void GeneratedBlock::Initialize(Bent *f) {
	error = 0;
	start = end = cursor = NULL;
	m     = f;
}

bool GeneratedBlock::Attach(HOOY *h) {
	if (0 == error) {
		if (h) {
			if (NULL == start) {
				start = end = cursor = h;
			} else {
				end->next = h;
				h->prev   = end;
				end       = cursor = h;
			}
			return true;
		} else {
			error = 1;
		}
	}
	return false;
}

void GeneratedBlock::Insert(HOOY *h, HOOY *at) {
	if (at != start) {
		at->prev->next = h;
		h->prev        = at->prev;
		h->next        = at;
		at->prev       = h;
	} else {
		h->next     = start;
		start       = h;
		start->prev = h;
		h->prev     = NULL;
	}
}

void GeneratedBlock::Detach(HOOY *h) {
	if (end != h) {
		if (start != h) {
			h->next->prev = h->prev;
			h->prev->next = h->next;
			h->next       = h->prev = NULL;
		} else {
			start   = h->next;
			h->prev = NULL;
		}
	} else {
		h->prev->next = NULL;
		end           = h->prev;
		h->prev       = NULL;
	}
}

void GeneratedBlock::AttachToMainList(HOOY *at) {
	assert(start && end);
	m->hooyList.insert_block(at, start, end);
}

int GeneratedBlock::Replace(HOOY *h) {
	return 0;
}

void GeneratedBlock::Free() {
	if (start && end) {
		for (HOOY *h = start; h != NULL; h = h->next) {
			//FreeHooy(h);
		}
	}
	start = end = cursor = NULL;
}

void GeneratedBlock::Reset() {
	error = 0;
	start = end = cursor = NULL;
}

bool GeneratedBlock::Valid() {
	return start && end;
}

HOOY *GeneratedBlock::GenOpRC(char *mnemonic, int reg, size_t imm) {
	HOOY *h = GenHOpRC(mnemonic, reg, imm);
	Attach(h);
	return h;
}

HOOY *GeneratedBlock::GenOpRR(char *mnemonic, int reg, int reg2) {
	HOOY *h = GenHOpRR(mnemonic, reg, reg2);
	Attach(h);
	return h;
}

HOOY *GeneratedBlock::GenOpRM(char *mnemonic, int reg, int baseReg, int indexReg, int scale, size_t displacement) {
	HOOY *h = GenHOpRM(mnemonic, reg, baseReg, indexReg, scale, displacement);
	Attach(h);
	return h;
}

HOOY *GeneratedBlock::GenOpMR(char *mnemonic, int baseReg, int indexReg, int scale, size_t displacement, int reg) {
	HOOY *h = GenHOpMR(mnemonic, baseReg, indexReg, scale, displacement, reg);
	Attach(h);
	return h;
}

HOOY *GeneratedBlock::GenOpMC(char *mnemonic, int baseReg, int indexReg, int scale, size_t displacement, size_t imm) {
	HOOY *h = GenHOpMC(mnemonic, baseReg, indexReg, scale, displacement, imm);
	Attach(h);
	return h;
}

HOOY *GeneratedBlock::GenOpR(char *mnemonic, int reg) {
	HOOY *h = GenHOpR(mnemonic, reg);
	Attach(h);
	return h;
}

HOOY *GeneratedBlock::GenOpM(char *mnemonic, int baseReg, int indexReg, int scale, size_t displacement) {
	HOOY *h = GenHOpM(mnemonic, baseReg, indexReg, scale, displacement);
	Attach(h);
	return h;
}

HOOY *GeneratedBlock::GenOpC(char *mnemonic, size_t imm) {
	HOOY *h = GenHOpC(mnemonic, imm);
	Attach(h);
	return h;
}

HOOY *GeneratedBlock::GenOp(char *mnemonic) {
	HOOY *h = GenHOp(mnemonic);
	Attach(h);
	return h;
}

HOOY *GeneratedBlock::GenOpRA(char *mnemonic, int reg, ARGTYPE *a, INSTRTYPE *i) {
	return GenHOpRA(mnemonic, reg, a, i);
}

HOOY *GeneratedBlock::GenOpMA(char *mnemonic, int baseReg, int indexReg, int scale, size_t displacement, ARGTYPE *a, INSTRTYPE *i) {
	return GenHOpMA(mnemonic, baseReg, indexReg, scale, displacement, a, i);
}

HOOY *GeneratedBlock::GenOpAC(char *mnemonic, ARGTYPE *a, INSTRTYPE *i, size_t imm) {
	return GenHOpAC(mnemonic, a, i, imm);
}

HOOY *GeneratedBlock::GenOpAR(char *mnemonic, ARGTYPE *a, INSTRTYPE *i, int reg) {
	return GenHOpAR(mnemonic, a, i, reg);
}

HOOY *GeneratedBlock::GenOpAM(char *mnemonic, ARGTYPE *a, INSTRTYPE *i, int baseReg, int indexReg, int scale, size_t displacement) {
	return GenHOpAM(mnemonic, a, i, baseReg, indexReg, scale, displacement);
}

HOOY *GeneratedBlock::GenOpAA(char *mnemonic, ARGTYPE *a1, INSTRTYPE *i1, ARGTYPE *a2, INSTRTYPE *i2) {
	return GenHOpAA(mnemonic, a1, i1, a2, i2);
}

HOOY *GeneratedBlock::GenOpA(char *mnemonic, ARGTYPE *a, INSTRTYPE *i) {
	return GenHOpA(mnemonic, a, i);
}

HOOY *GeneratedBlock::GenLabel() {
	HOOY *h = GenHLabel(m);
	Attach(h);
	return h;
}

HOOY *GeneratedBlock::GenCall(HOOY *dest) {
	HOOY *h = GenHCall(m, dest);
	Attach(h);
	return h;
}

HOOY *GeneratedBlock::GenCall(int reg) {
	HOOY *h = GenHCall(m, reg);
	Attach(h);
	return h;
}

HOOY *GeneratedBlock::GenJMP(HOOY *dest) {
	HOOY *h = GenHJMP(m, dest);
	Attach(h);
	return h;
}

HOOY *GeneratedBlock::GenJMP(int reg) {
	HOOY *h = GenHJMP(m, reg);
	Attach(h);
	return h;
}

HOOY *GeneratedBlock::GenJCC(int cond, HOOY *dest) {
	HOOY *h = GenHJCC(m, cond, dest);
	Attach(h);
	return h;
}

HOOY *GeneratedBlock::GenData(BYTE *p, size_t len) {
	HOOY *h    = (HOOY*)calloc(1, sizeof(HOOY));
	h->dataptr = p;
	h->datalen = len;
	h->flags  |= FL_PRESENT | FL_VPRESENT | FL_DATA | FL_GENERATED;
	Attach(h);
	return h;
}

void GeneratedBlock::Print(char *output, size_t len) {
	int  offset = 0;
	char line[256];

	output[0] = 0;

	for (HOOY *h = start; h; h = h->next) {
		if (h->diza) {
			sprintf_s(line, "%08X: %s\n", offset, h->diza->d.CompleteInstr);
		} else {
			if (h->flags & FL_LABEL) {
				sprintf_s(line, "%08X: %08X FL_LABEL\n", offset, h->oldrva);
			} else {
				sprintf_s(line, "%08X\n", offset);
			}
		}
		strcat_s(output, len, line);
		offset += h->datalen;
		if (h == end) {
			break;
		}
	}

	strcat_s(output, len, "\n");
}

void GeneratedBlock::Print() {
#define PRINT_BUF_SIZE 4096*8
	char *output = (char*)malloc(PRINT_BUF_SIZE);
	Print(output, PRINT_BUF_SIZE);
	Log(output);
	free(output);
}

int GeneratedBlock::Link() {
	outSize = GetSize();
	output  = (BYTE*)calloc(1, outSize + 1);
	
	DWORD v = 0;

	for (HOOY *h = start; h != NULL; h = h->next) {
		h->newofs = h->newrva = v;
		if (h->dataptr && h->datalen) {
			memcpy(&output[v], h->dataptr, h->datalen);
			v += h->datalen;
		}
		if (h == end) {
			break;
		}
	}

	return v;
}

/*
int GeneratedBlock::Link(size_t rva) {
	outSize = GetSize();
	output  = (BYTE*)calloc(1, outSize + 1);
	
	DWORD v = rva, p = 0;

	for (HOOY *h = start; h != NULL; h = h->next) {
		h->newofs = p;
		h->newrva = v;
		if (h->dataptr && h->datalen) {
			p += h->datalen;
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
	// TODO: if (m->isX64) {
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

	return p;
}
*/

size_t GeneratedBlock::GetSize() {
	size_t res = 0;
	for (HOOY *h = start; h != NULL; h = h->next) {
		res += h->datalen;
		if (h == end) {
			break;
		}
	}
	return res;
}
