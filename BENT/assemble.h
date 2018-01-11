#ifndef ASSEMBLE_H
#define ASSEMBLE_H

#define ASMJIT_X86
#include "asmjit\asmjit.h"

AsmJit::GpReg GetReg(int reg);
AsmJit::Mem GetMem(ARGTYPE *arg);
AsmJit::Imm GetImm(INSTRTYPE *insn);
int AssembleInstruction(DISASM *da, AsmJit::X86Assembler *a);
size_t AssembleInstruction(DISASM *da, void **buffer);

#endif
