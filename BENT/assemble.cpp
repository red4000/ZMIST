#include <Windows.h>
#define _NO_CVCONST_H
#include <dbghelp.h>
#include <stdio.h>
#include <algorithm>
#define ASMJIT_X86
#include "asmjit/asmjit.h"
#include "bent.h"
#include "log.h"
#include "crc32.h"

AsmJit::X86Assembler mainAssembler;

AsmJit::GpReg GetReg(int reg) {
	switch (reg & (REG0 | REG1 | REG2 | REG3 | REG4 | REG5 | REG6 | REG7 | REG8)) {
	default:
		return AsmJit::GpReg();

	case REG0:
		return AsmJit::eax;

	case REG1:
		return AsmJit::ecx;

	case REG2:
		return AsmJit::edx;

	case REG3:
		return AsmJit::ebx;

	case REG4:
		return AsmJit::esp;

	case REG5:
		return AsmJit::ebp;

	case REG6:
		return AsmJit::esi;

	case REG7:
		return AsmJit::edi;
	}
}

AsmJit::Mem GetMem(ARGTYPE *arg) {
	if (0 != arg->Memory.BaseRegister) {
		if (0 != arg->Memory.Displacement) {
			if (0 != arg->Memory.Scale) {
				return AsmJit::Mem(GetReg(arg->Memory.BaseRegister), GetReg(arg->Memory.IndexRegister), arg->Memory.Scale, arg->Memory.Displacement);
			} else {
				return AsmJit::Mem(GetReg(arg->Memory.BaseRegister), arg->Memory.Displacement);
			}
		} else {
			if (0 != arg->Memory.Scale) {
				return AsmJit::Mem(GetReg(arg->Memory.BaseRegister), GetReg(arg->Memory.IndexRegister), arg->Memory.Scale, arg->Memory.Displacement);
			} else {
				if (0 != arg->Memory.IndexRegister) {
					return AsmJit::Mem(GetReg(arg->Memory.BaseRegister), GetReg(arg->Memory.IndexRegister), 0, arg->Memory.Displacement);
				} else {
					return AsmJit::Mem(GetReg(arg->Memory.BaseRegister), 0);
				}
			}
		}
	} else {
		if (0 != arg->Memory.Displacement) {
			if (0 != arg->Memory.Scale) {
				return AsmJit::Mem(GetReg(arg->Memory.BaseRegister), GetReg(arg->Memory.IndexRegister), arg->Memory.Scale, arg->Memory.Displacement);
			} else {
				return AsmJit::Mem(GetReg(arg->Memory.BaseRegister), arg->Memory.Displacement);
			}
		} else {
			if (0 != arg->Memory.Scale) {
				return AsmJit::Mem(GetReg(arg->Memory.BaseRegister), GetReg(arg->Memory.IndexRegister), 2, arg->Memory.Displacement, 4);
			} else {
				return AsmJit::Mem(GetReg(arg->Memory.BaseRegister), 0);
			}
		}
	}
}

AsmJit::Imm GetImm(INSTRTYPE *insn) {
	return AsmJit::Imm(insn->Immediat);
}

size_t AssembleInstruction(DISASM *da, AsmJit::X86Assembler *a) {
#define ASSEMBLE(opcode) break
	DWORD crc = CRC32String(da->Instruction.Mnemonic);
	switch (crc) {
	case 0xE9888267: // "mov"
		if (da->Argument1.ArgType & REGISTER_TYPE) {
			if (da->Argument2.ArgType & REGISTER_TYPE) {
				a->mov(GetReg(da->Argument1.ArgType), GetReg(da->Argument2.ArgType));
			} else if (da->Argument2.ArgType & MEMORY_TYPE) {
				a->mov(GetReg(da->Argument1.ArgType), GetMem(&da->Argument2));
			} else if (da->Argument2.ArgType & CONSTANT_TYPE) {
				a->mov(GetReg(da->Argument1.ArgType), GetImm(&da->Instruction));
			}
		} else if (da->Argument1.ArgType & MEMORY_TYPE) {
			if (da->Argument2.ArgType & REGISTER_TYPE) {
				a->mov(GetMem(&da->Argument1), GetReg(da->Argument2.ArgType));
			} else if (da->Argument2.ArgType & CONSTANT_TYPE) {
				a->mov(GetMem(&da->Argument1), GetImm(&da->Instruction));
			}
		}
		ASSEMBLE(mov);

	case 0xD7FFA2ED: // add
		if (da->Argument1.ArgType & REGISTER_TYPE) {
			if (da->Argument2.ArgType & REGISTER_TYPE) {
				a->add(GetReg(da->Argument1.ArgType), GetReg(da->Argument2.ArgType));
			} else if (da->Argument2.ArgType & MEMORY_TYPE) {
				a->add(GetReg(da->Argument1.ArgType), GetMem(&da->Argument2));
			} else if (da->Argument2.ArgType & CONSTANT_TYPE) {
				a->add(GetReg(da->Argument1.ArgType), GetImm(&da->Instruction));
			}
		} else if (da->Argument1.ArgType & MEMORY_TYPE) {
			if (da->Argument2.ArgType & REGISTER_TYPE) {
				a->add(GetMem(&da->Argument1), GetReg(da->Argument2.ArgType));
			} else if (da->Argument2.ArgType & CONSTANT_TYPE) {
				a->add(GetMem(&da->Argument1), GetImm(&da->Instruction));
			}
		}
		ASSEMBLE(add);

	case 0x66515338: // sub
		if (da->Argument1.ArgType & REGISTER_TYPE) {
			if (da->Argument2.ArgType & REGISTER_TYPE) {
				a->sub(GetReg(da->Argument1.ArgType), GetReg(da->Argument2.ArgType));
			} else if (da->Argument2.ArgType & MEMORY_TYPE) {
				a->sub(GetReg(da->Argument1.ArgType), GetMem(&da->Argument2));
			} else if (da->Argument2.ArgType & CONSTANT_TYPE) {
				a->sub(GetReg(da->Argument1.ArgType), GetImm(&da->Instruction));
			}
		} else if (da->Argument1.ArgType & MEMORY_TYPE) {
			if (da->Argument2.ArgType & REGISTER_TYPE) {
				a->sub(GetMem(&da->Argument1), GetReg(da->Argument2.ArgType));
			} else if (da->Argument2.ArgType & CONSTANT_TYPE) {
				a->sub(GetMem(&da->Argument1), GetImm(&da->Instruction));
			}
		}
		ASSEMBLE(sub);

	case 0xEA23E0CE: // xor
		if (da->Argument1.ArgType & REGISTER_TYPE) {
			if (da->Argument2.ArgType & REGISTER_TYPE) {
				a->xor_(GetReg(da->Argument1.ArgType), GetReg(da->Argument2.ArgType));
			} else if (da->Argument2.ArgType & MEMORY_TYPE) {
				a->xor_(GetReg(da->Argument1.ArgType), GetMem(&da->Argument2));
			} else if (da->Argument2.ArgType & CONSTANT_TYPE) {
				a->xor_(GetReg(da->Argument1.ArgType), GetImm(&da->Instruction));
			}
		} else if (da->Argument1.ArgType & MEMORY_TYPE) {
			if (da->Argument2.ArgType & REGISTER_TYPE) {
				a->xor_(GetMem(&da->Argument1), GetReg(da->Argument2.ArgType));
			} else if (da->Argument2.ArgType & CONSTANT_TYPE) {
				a->xor_(GetMem(&da->Argument1), GetImm(&da->Instruction));
			}
		}
		ASSEMBLE(xor_);

	case 0x9AAD6EB3: // or
		if (da->Argument1.ArgType & REGISTER_TYPE) {
			if (da->Argument2.ArgType & REGISTER_TYPE) {
				a->or_(GetReg(da->Argument1.ArgType), GetReg(da->Argument2.ArgType));
			} else if (da->Argument2.ArgType & MEMORY_TYPE) {
				a->or_(GetReg(da->Argument1.ArgType), GetMem(&da->Argument2));
			} else if (da->Argument2.ArgType & CONSTANT_TYPE) {
				a->or_(GetReg(da->Argument1.ArgType), GetImm(&da->Instruction));
			}
		} else if (da->Argument1.ArgType & MEMORY_TYPE) {
			if (da->Argument2.ArgType & REGISTER_TYPE) {
				a->or_(GetMem(&da->Argument1), GetReg(da->Argument2.ArgType));
			} else if (da->Argument2.ArgType & CONSTANT_TYPE) {
				a->or_(GetMem(&da->Argument1), GetImm(&da->Instruction));
			}
		}
		ASSEMBLE(or_);

	case 0xDA68273B: // and
		if (da->Argument1.ArgType & REGISTER_TYPE) {
			if (da->Argument2.ArgType & REGISTER_TYPE) {
				a->and_(GetReg(da->Argument1.ArgType), GetReg(da->Argument2.ArgType));
			} else if (da->Argument2.ArgType & MEMORY_TYPE) {
				a->and_(GetReg(da->Argument1.ArgType), GetMem(&da->Argument2));
			} else if (da->Argument2.ArgType & CONSTANT_TYPE) {
				a->and_(GetReg(da->Argument1.ArgType), GetImm(&da->Instruction));
			}
		} else if (da->Argument1.ArgType & MEMORY_TYPE) {
			if (da->Argument2.ArgType & REGISTER_TYPE) {
				a->and_(GetMem(&da->Argument1), GetReg(da->Argument2.ArgType));
			} else if (da->Argument2.ArgType & CONSTANT_TYPE) {
				a->and_(GetMem(&da->Argument1), GetImm(&da->Instruction));
			}
		}
		ASSEMBLE(and_);

	case 0xEC2C4E95: // shl
		if (da->Argument1.ArgType & REGISTER_TYPE) {
			if (da->Argument2.ArgType & REGISTER_TYPE) {
				a->shl(GetReg(da->Argument1.ArgType), GetReg(da->Argument2.ArgType));
			} else if (da->Argument2.ArgType & CONSTANT_TYPE) {
				a->shl(GetReg(da->Argument1.ArgType), GetImm(&da->Instruction));
			}
		} else if (da->Argument1.ArgType & MEMORY_TYPE) {
			if (da->Argument2.ArgType & REGISTER_TYPE) {
				a->shl(GetMem(&da->Argument1), GetReg(da->Argument2.ArgType));
			} else if (da->Argument2.ArgType & CONSTANT_TYPE) {
				a->shl(GetMem(&da->Argument1), GetImm(&da->Instruction));
			}
		}
		ASSEMBLE(shl);

	case 0x386D714A: // shr
		if (da->Argument1.ArgType & REGISTER_TYPE) {
			if (da->Argument2.ArgType & REGISTER_TYPE) {
				a->shr(GetReg(da->Argument1.ArgType), GetReg(da->Argument2.ArgType));
			} else if (da->Argument2.ArgType & CONSTANT_TYPE) {
				a->shr(GetReg(da->Argument1.ArgType), GetImm(&da->Instruction));
			}
		} else if (da->Argument1.ArgType & MEMORY_TYPE) {
			if (da->Argument2.ArgType & REGISTER_TYPE) {
				a->shr(GetMem(&da->Argument1), GetReg(da->Argument2.ArgType));
			} else if (da->Argument2.ArgType & CONSTANT_TYPE) {
				a->shr(GetMem(&da->Argument1), GetImm(&da->Instruction));
			}
		}
		ASSEMBLE(shr);

	case 0x51DF3F75: // rol
		if (da->Argument1.ArgType & REGISTER_TYPE) {
			if (da->Argument2.ArgType & REGISTER_TYPE) {
				a->rol(GetReg(da->Argument1.ArgType), GetReg(da->Argument2.ArgType));
			} else if (da->Argument2.ArgType & CONSTANT_TYPE) {
				a->rol(GetReg(da->Argument1.ArgType), GetImm(&da->Instruction));
			}
		} else if (da->Argument1.ArgType & MEMORY_TYPE) {
			if (da->Argument2.ArgType & REGISTER_TYPE) {
				a->rol(GetMem(&da->Argument1), GetReg(da->Argument2.ArgType));
			} else if (da->Argument2.ArgType & CONSTANT_TYPE) {
				a->rol(GetMem(&da->Argument1), GetImm(&da->Instruction));
			}
		}
		ASSEMBLE(rol);

	case 0x859E00AA: // ror
		if (da->Argument1.ArgType & REGISTER_TYPE) {
			if (da->Argument2.ArgType & REGISTER_TYPE) {
				a->ror(GetReg(da->Argument1.ArgType), GetReg(da->Argument2.ArgType));
			} else if (da->Argument2.ArgType & CONSTANT_TYPE) {
				a->ror(GetReg(da->Argument1.ArgType), GetImm(&da->Instruction));
			}
		} else if (da->Argument1.ArgType & MEMORY_TYPE) {
			if (da->Argument2.ArgType & REGISTER_TYPE) {
				a->ror(GetMem(&da->Argument1), GetReg(da->Argument2.ArgType));
			} else if (da->Argument2.ArgType & CONSTANT_TYPE) {
				a->ror(GetMem(&da->Argument1), GetImm(&da->Instruction));
			}
		}
		ASSEMBLE(ror);

	case 0x509D9913: // inc
		if (da->Argument1.ArgType & REGISTER_TYPE) {
			a->inc(GetReg(da->Argument1.ArgType));
		} else if (da->Argument1.ArgType & MEMORY_TYPE) {
			a->inc(GetMem(&da->Argument1));
		}
		ASSEMBLE(inc);

	case 0xAEA2AE2F: // dec
		if (da->Argument1.ArgType & REGISTER_TYPE) {
			a->dec(GetReg(da->Argument1.ArgType));
		} else if (da->Argument1.ArgType & MEMORY_TYPE) {
			a->dec(GetMem(&da->Argument1));
		}
		ASSEMBLE(dec);

	case 0xA5738B4F: // neg
		if (da->Argument1.ArgType & REGISTER_TYPE) {
			a->neg(GetReg(da->Argument1.ArgType));
		} else if (da->Argument1.ArgType & MEMORY_TYPE) {
			a->neg(GetMem(&da->Argument1));
		}
		ASSEMBLE(neg);

	case 0xA3EC5012: // push
		if (da->Argument1.ArgType & REGISTER_TYPE) {
			a->push(GetReg(da->Argument1.ArgType));
		} else if (da->Argument1.ArgType & MEMORY_TYPE) {
			a->push(GetMem(&da->Argument1));
		} else if (da->Argument1.ArgType & CONSTANT_TYPE) {
			a->push(GetImm(&da->Instruction));
		}
		ASSEMBLE(push);

	case 0x82A5A990: // pushad
		a->pushad();
		ASSEMBLE(pushad);

	case 0x3C53B4B7: // pushf
		a->pushf();
		ASSEMBLE(pushf);

	case 0x1DA1AAA3: // pop
		if (da->Argument1.ArgType & REGISTER_TYPE) {
			a->pop(GetReg(da->Argument1.ArgType));
		} else if (da->Argument1.ArgType & MEMORY_TYPE) {
			a->pop(GetMem(&da->Argument1));
		}
		ASSEMBLE(pop);

	case 0x3E7D6770: // popad
		a->popad();
		ASSEMBLE(popad);

	case 0x821E4E06: // popf
		a->popf();
		ASSEMBLE(popf);

	case 0x11D5C3B9: // lodsb
		a->rep_lodsb();
		ASSEMBLE(rep_lodsb);

	case 0x7C082752: // lodsw
		a->rep_lodsw();
		ASSEMBLE(rep_lodsw);

	case 0xF8B6668C: // lodsd
		a->rep_lodsd();
		ASSEMBLE(rep_lodsd);

	case 0x7828FC95: // stosb
		a->rep_stosb();
		ASSEMBLE(rep_stosb);

	case 0x15F5187E: // stosw
		a->rep_stosw();
		ASSEMBLE(rep_stosw);

	case 0x914B59A0: // stosd
		a->rep_stosd();
		ASSEMBLE(rep_stosd);

	case 0x5C8986BC: // cmp
		if (da->Argument1.ArgType & REGISTER_TYPE) {
			if (da->Argument2.ArgType & REGISTER_TYPE) {
				a->cmp(GetReg(da->Argument1.ArgType), GetReg(da->Argument2.ArgType));
			} else if (da->Argument2.ArgType & MEMORY_TYPE) {
				a->cmp(GetReg(da->Argument1.ArgType), GetMem(&da->Argument2));
			} else if (da->Argument2.ArgType & CONSTANT_TYPE) {
				a->cmp(GetReg(da->Argument1.ArgType), GetImm(&da->Instruction));
			}
		} else if (da->Argument1.ArgType & MEMORY_TYPE) {
			if (da->Argument2.ArgType & REGISTER_TYPE) {
				a->cmp(GetMem(&da->Argument1), GetReg(da->Argument2.ArgType));
			} else if (da->Argument2.ArgType & CONSTANT_TYPE) {
				a->cmp(GetMem(&da->Argument1), GetImm(&da->Instruction));
			}
		}
		ASSEMBLE(cmp);

	case 0x5920E442: // lea
		if (da->Argument1.ArgType & REGISTER_TYPE) {
			if (da->Argument2.ArgType & MEMORY_TYPE) {
				a->lea(GetReg(da->Argument1.ArgType), GetMem(&da->Argument2));
			}
		}
		ASSEMBLE(lea);

	case 0xDE5322FA: // ret
		if (da->Argument1.ArgType & CONSTANT_TYPE) {
			a->ret(GetImm(&da->Instruction));
		} else {
			a->ret();
		}
		ASSEMBLE(ret);
		
	case 0x551E1D85: // int
		a->int3();
		ASSEMBLE(int);

	case 0x2181C936: // jmp
		if (da->Argument1.ArgType & REGISTER_TYPE) {
			a->jmp(GetReg(da->Argument1.ArgType));
		} else if (da->Argument1.ArgType & MEMORY_TYPE) {
			a->jmp(GetMem(&da->Argument1));
		} else if (da->Argument1.ArgType & CONSTANT_TYPE) {
			a->jmp(GetImm(&da->Instruction));
		}
		ASSEMBLE(jmp);

	case 0xCC8E2F3E: // call
		if (da->Argument1.ArgType & REGISTER_TYPE) {
			a->call(GetReg(da->Argument1.ArgType));
		} else if (da->Argument1.ArgType & MEMORY_TYPE) {
			a->call(GetMem(&da->Argument1));
		} else if (da->Argument1.ArgType & CONSTANT_TYPE) {
			a->call(GetImm(&da->Instruction));
		}
		ASSEMBLE(call);

	default:
		DEBUG_LOG("Error: assembling unknown instruction %X %s\n", crc, da->Instruction.Mnemonic);
		return 1;
		break;
	}

	//void *code = a->make();
	//(*buffer)  = code;

	return 0;
}

size_t AssembleInstruction(DISASM *da, void **buffer) {
	int ares = AssembleInstruction(da, &mainAssembler);

	if (0 == ares) {
		void *code = mainAssembler.make();
		size_t res = mainAssembler.getCodeSize();
		//mainAssembler.relocCode(newBuf, newVA); 
		if (code && res) {
			(*buffer) = malloc(res + 1);
			memcpy(*buffer, code, res);
			mainAssembler.clear();

			return res;
		}
	}

	(*buffer) = NULL;
	return 0;
}
