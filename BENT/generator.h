#ifndef GENERATOR_H
#define GENERATOR_H

#include "assemble.h"

enum {
	R_EAX = 0x1,
	R_ECX = 0x2,
	R_EDX = 0x4,
	R_EBX = 0x8,
	R_ESP = 0x10,
	R_EBP = 0x20,
	R_ESI = 0x40,
	R_EDI = 0x80
};

#define O_MOV  "mov "
#define O_SUB  "sub "
#define O_XOR  "xor "
#define O_OR   "or "
#define O_SHL  "shl "
#define O_SHR  "shr "
#define O_ROL  "rol "
#define O_ROR  "ror "
#define O_DEC  "dec "
#define O_NEG  "neg "
#define O_PUSH "push "
#define O_POP  "pop "
#define O_CMP  "cmp "
#define O_LEA  "lea "
#define O_RET  "ret "

void GenOpRC(DISASM *out, char *mnemonic, int reg, size_t imm);
void GenOpRR(DISASM *out, char *mnemonic, int reg, int reg2);
void GenOpRM(DISASM *out, char *mnemonic, int reg, int baseReg, int indexReg, int scale, size_t displacement);
void GenOpMR(DISASM *out, char *mnemonic, int baseReg, int indexReg, int scale, size_t displacement, int reg);
void GenOpMC(DISASM *out, char *mnemonic, int baseReg, int indexReg, int scale, size_t displacement, size_t imm);
void GenOpR(DISASM *out, char *mnemonic, int reg);
void GenOpM(DISASM *out, char *mnemonic, int baseReg, int indexReg, int scale, size_t displacement);
void GenOpC(DISASM *out, char *mnemonic, size_t imm);
void GenOp(DISASM *out, char *mnemonic);
void GenOpRA(DISASM *out, char *mnemonic, int reg, ARGTYPE *a, INSTRTYPE *i);
void GenOpMA(DISASM *out, char *mnemonic, int baseReg, int indexReg, int scale, size_t displacement, ARGTYPE *a, INSTRTYPE *i);
void GenOpAC(DISASM *out, char *mnemonic, ARGTYPE *a, INSTRTYPE *i, size_t imm);
void GenOpAR(DISASM *out, char *mnemonic, ARGTYPE *a, INSTRTYPE *i, int reg);
void GenOpAM(DISASM *out, char *mnemonic, ARGTYPE *a, INSTRTYPE *i, int baseReg, int indexReg, int scale, size_t displacement);
void GenOpAA(DISASM *out, char *mnemonic, ARGTYPE *a1, INSTRTYPE *i1, ARGTYPE *a2, INSTRTYPE *i2);
void GenOpA(DISASM *out, char *mnemonic, ARGTYPE *a, INSTRTYPE *i);
HOOY *GenHOpRC(char *mnemonic, int reg, size_t imm);
HOOY *GenHOpRR(char *mnemonic, int reg, int reg2);
HOOY *GenHOpRM(char *mnemonic, int reg, int baseReg, int indexReg, int scale, size_t displacement);
HOOY *GenHOpMR(char *mnemonic, int baseReg, int indexReg, int scale, size_t displacement, int reg);
HOOY *GenHOpMC(char *mnemonic, int baseReg, int indexReg, int scale, size_t displacement, size_t imm);
HOOY *GenHOpR(char *mnemonic, int reg);
HOOY *GenHOpM(char *mnemonic, int baseReg, int indexReg, int scale, size_t displacement);
HOOY *GenHOpC(char *mnemonic, size_t imm);
HOOY *GenHOp(char *mnemonic);
HOOY *GenHOpRA(char *mnemonic, int reg, ARGTYPE *a, INSTRTYPE *i);
HOOY *GenHOpMA(char *mnemonic, int baseReg, int indexReg, int scale, size_t displacement, ARGTYPE *a, INSTRTYPE *i);
HOOY *GenHOpAC(char *mnemonic, ARGTYPE *a, INSTRTYPE *i, size_t imm);
HOOY *GenHOpAR(char *mnemonic, ARGTYPE *a, INSTRTYPE *i, int reg);
HOOY *GenHOpAM(char *mnemonic, ARGTYPE *a, INSTRTYPE *i, int baseReg, int indexReg, int scale, size_t displacement);
HOOY *GenHOpAA(char *mnemonic, ARGTYPE *a1, INSTRTYPE *i1, ARGTYPE *a2, INSTRTYPE *i2);
HOOY *GenHOpA(char *mnemonic, ARGTYPE *a, INSTRTYPE *i);
HOOY *GenHLabel(Bent *m);
HOOY *GenHCall(Bent *m, HOOY *dest);
HOOY *GenHCall(Bent *m, int reg);
HOOY *GenHJMP(Bent *m, HOOY *dest);
HOOY *GenHJMP(Bent *m, int reg);
HOOY *GenHJCC(Bent *b, int cond, HOOY *dest);
HOOY *GenHData(BYTE *p, size_t len);
int ReDisasm(HOOY *h);

enum {
	C_JA,
	C_JNBE = C_JA,
	C_JAE,
	C_JNB = C_JAE,
	C_JB,
	C_JNAE = C_JB,
	C_JBE,
	C_JNA = C_JBE,
	C_JC,
	C_JE,
	C_JZ = C_JE,
	C_JNC,
	C_JNE,
	C_JNZ = C_JNE,
	C_JNP,
	C_JPO = C_JNP,
	C_JP,
	C_JPE = C_JP,
	//signed
	C_JG,
	C_JNLE = C_JG,
	C_JGE,
	C_JNL = C_JGE,
	C_JL,
	C_JNGE = C_JL,
	C_JLE,
	C_JNG = C_JLE,
	C_JNO,
	C_JNS,
	C_JO,
	C_JS
};

HOOY *GenHJCC(Bent *m, int cond, HOOY *dest);

class GeneratedBlock {
public:
	GeneratedBlock(Bent *f);
	GeneratedBlock();

	void Initialize(Bent *f);

	bool Attach(HOOY *h);
	void Insert(HOOY *h, HOOY *at);
	void Detach(HOOY *h);

	void AttachToMainList(HOOY *at); // insert this generated block to hooyList // TODO: rename to InsertInMainList
	int Replace(HOOY *h); // replace h with this block
	void Free();
	void Reset();
	bool Valid();

	HOOY *GenOpRC(char *mnemonic, int reg, size_t imm);
	HOOY *GenOpRR(char *mnemonic, int reg, int reg2);
	HOOY *GenOpRM(char *mnemonic, int reg, int baseReg, int indexReg, int scale, size_t displacement);
	HOOY *GenOpMR(char *mnemonic, int baseReg, int indexReg, int scale, size_t displacement, int reg);
	HOOY *GenOpMC(char *mnemonic, int baseReg, int indexReg, int scale, size_t displacement, size_t imm);
	HOOY *GenOpR(char *mnemonic, int reg);
	HOOY *GenOpM(char *mnemonic, int baseReg, int indexReg, int scale, size_t displacement);
	HOOY *GenOpC(char *mnemonic, size_t imm);
	HOOY *GenOp(char *mnemonic);
	HOOY *GenOpRA(char *mnemonic, int reg, ARGTYPE *a, INSTRTYPE *i);
	HOOY *GenOpMA(char *mnemonic, int baseReg, int indexReg, int scale, size_t displacement, ARGTYPE *a, INSTRTYPE *i);
	HOOY *GenOpAC(char *mnemonic, ARGTYPE *a, INSTRTYPE *i, size_t imm);
	HOOY *GenOpAR(char *mnemonic, ARGTYPE *a, INSTRTYPE *i, int reg);
	HOOY *GenOpAM(char *mnemonic, ARGTYPE *a, INSTRTYPE *i, int baseReg, int indexReg, int scale, size_t displacement);
	HOOY *GenOpAA(char *mnemonic, ARGTYPE *a1, INSTRTYPE *i1, ARGTYPE *a2, INSTRTYPE *i2);
	HOOY *GenOpA(char *mnemonic, ARGTYPE *a, INSTRTYPE *i);
	HOOY *GenLabel();
	HOOY *GenCall(HOOY *dest);
	HOOY *GenCall(int reg);
	HOOY *GenJMP(HOOY *dest);
	HOOY *GenJMP(int reg);
	HOOY *GenJCC(int cond, HOOY *dest);
	HOOY *GenData(BYTE *p, size_t len);
	void Print(char *output, size_t len);
	void Print();
	//int AssembleForRVA(size_t rva);
	int Link();

	size_t GetSize();

	int    error;
	HOOY  *start, *end, *cursor;
	Bent *m;

	BYTE  *output;
	size_t outSize;

	AsmJit::X86Assembler a;
};

class BlockGenerator
{
public:
};

#endif
