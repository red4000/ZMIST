#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "xda.h"
#include "crc32.h"

int XDisasm(XDA *da, BYTE *p, size_t va) {
	da->d.EIP         = (UIntPtr)p;
	da->d.VirtualAddr = va;
	da->d.Archi       = 32;
	int len = Disasm(&da->d);
	da->len = len;

	if (len == UNKNOWN_OPCODE || len == OUT_OF_BLOCK || (da->d.Instruction.Category & ILLEGAL_INSTRUCTION) || (p[0] == 0  && p[1] == 0)) {
		return 0;
	}

	if (0xCC == p[0]) {
		da->flag |= C_STOP;
	}

	switch (da->d.Instruction.BranchType) {
	case JO:
	case JC:
	case JE:
	case JA:
	case JS:
	case JP:
	case JL:
	case JG:
	case JB:
	case JECXZ:
	case CallType:
	case JNO:
	case JNC:
	case JNE:
	case JNA:
	case JNS:
	case JNP:
	case JNL:
	case JNG:
	case JNB:
		if (0 == (da->d.Argument1.ArgType & REGISTER_TYPE) &&
			0xFF != p[0]) {
			if (0 != da->d.Instruction.AddrValue) {
				if (da->len < 5) {
					da->d.Argument1.ArgSize = 1;
				}
				else {
					da->d.Argument1.ArgSize = 4;
				}

				//da->flag |= C_REL;
			}
			da->flag |= C_REL;
		}

		break;

	case JmpType:
		if (0 != da->d.Instruction.AddrValue) {
			if (da->len < 5) {
				da->d.Argument1.ArgSize = 1;
			} else {
				da->d.Argument1.ArgSize = 4;
			}
			//da->flag |= C_REL;
		}
		if (0 == (da->d.Argument1.ArgType & (REGISTER_TYPE | MEMORY_TYPE))) {
			da->flag |= C_REL;
		}
		da->flag |= C_JMP | C_STOP;
		break;

	case RetType:
		da->flag |= C_STOP;
		break;
	}

	if (len > 1) {
		WORD w = *(unsigned short*)p;
		if (w == 0x0000 ||
		    w == 0xFFFF ||
			w == 0x0001 ||
			w == 0x0011 ||
			w == 0x0111 ||
			w == 0x1100) {
			da->flag |= C_BAD;
		}

		DWORD insnCrc = CRC32String(da->d.CompleteInstr),
			  mnemCrc = CRC32String(da->d.Instruction.Mnemonic);

		if (0x8DDEFA10 == insnCrc || // "dec ebp"
			0xD59A88D6 == insnCrc || // "push cs"
			0xEE26AE8E == insnCrc || // "hlt "
			0x78574F5C == mnemCrc || // "in "
			0x60060EC8 == mnemCrc) { // "out "
			da->flag |= C_ERROR;
			da->len   = 0;
			return 0;
		}
	}

	da->srcSet = 0;
	da->dstSet = da->d.Instruction.ImplicitModifiedRegs;

	if (da->d.Argument1.AccessMode == READ) {
		da->srcSet |= da->d.Argument1.ArgType & (~(REGISTER_TYPE | NO_ARGUMENT));
	} else if (da->d.Argument1.AccessMode == WRITE) {
		da->dstSet |= da->d.Argument1.ArgType & (~(REGISTER_TYPE | NO_ARGUMENT));
	}

	if (da->d.Argument2.AccessMode == READ) {
		da->srcSet |= da->d.Argument2.ArgType & (~(REGISTER_TYPE | NO_ARGUMENT));
	} else if (da->d.Argument2.AccessMode == WRITE) {
		da->dstSet |= da->d.Argument2.ArgType & (~(REGISTER_TYPE | NO_ARGUMENT));
	}

	if (da->d.Argument3.AccessMode == READ) {
		da->srcSet |= da->d.Argument3.ArgType & (~(REGISTER_TYPE | NO_ARGUMENT));
	} else if (da->d.Argument3.AccessMode == WRITE) {
		da->dstSet |= da->d.Argument3.ArgType & (~(REGISTER_TYPE | NO_ARGUMENT));
	}
	return len;
}

int XDisasm64(XDA *da, BYTE *p, size_t va) {
	da->d.EIP         = (UIntPtr)p;
	da->d.VirtualAddr = va;
	da->d.Archi       = 64;
	int len = Disasm(&da->d);
	da->len = len;

	if (len == UNKNOWN_OPCODE || len == OUT_OF_BLOCK || (da->d.Instruction.Category & ILLEGAL_INSTRUCTION) || (p[0] == 0  && p[1] == 0)) {
		return 0;
	}

	if (0xCC == p[0]) {
		da->flag |= C_STOP;
	}

	switch (da->d.Instruction.BranchType) {
	case JO:
	case JC:
	case JE:
	case JA:
	case JS:
	case JP:
	case JL:
	case JG:
	case JB:
	case JECXZ:
	case CallType:
	case JNO:
	case JNC:
	case JNE:
	case JNA:
	case JNS:
	case JNP:
	case JNL:
	case JNG:
	case JNB:
		if (0 != da->d.Instruction.AddrValue) {
			if (0 == (da->d.Argument1.ArgType & MEMORY_TYPE)) {
				if (da->len < 5) {
					da->d.Argument1.ArgSize = 1;
				} else {
					da->d.Argument1.ArgSize = 4;
				}
			} else {
				da->d.Argument1.ArgSize = 8;
			}

			da->flag |= C_REL;
		}

		break;

	case JmpType:
		if (0 != da->d.Instruction.AddrValue) {
			if (0 == (da->d.Argument1.ArgType & MEMORY_TYPE)) {
				if (da->len < 5) {
					da->d.Argument1.ArgSize = 1;
				} else {
					da->d.Argument1.ArgSize = 4;
				}
			} else {
				da->d.Argument1.ArgSize = 8;
			}

			da->flag |= C_REL;
		}

		da->flag |= C_STOP;
		break;

	case RetType:
		da->flag |= C_STOP;
		break;
	}

	if (len > 1) {
		if (*(unsigned short*)p == 0x0000 ||
		    *(unsigned short*)p == 0xFFFF) {
			da->flag |= C_BAD;
		}

		if (0 == strcmp("dec ebp", da->d.CompleteInstr) ||
		    0 == strcmp("push cs", da->d.CompleteInstr) ||
		    0 == strcmp("hlt ", da->d.CompleteInstr) ||
		    0 == strcmp("in ", da->d.Instruction.Mnemonic) ||
		    0 == strcmp("out ", da->d.Instruction.Mnemonic)) {
			da->flag |= C_ERROR;
			da->len   = 0;
			return 0;
		}
	}

	return len;
}
