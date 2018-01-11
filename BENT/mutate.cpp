#include "bent.h"
#include "generator.h"
#include "cgen.h"
#include "mt19937.h"
#include "log.h"

HOOY *GenerateNop() {
	HOOY *h       = (HOOY*)calloc(1, sizeof(HOOY));
	h->flags      = FL_EXECUTABLE | FL_GENERATED | FL_PRESENT | FL_VPRESENT | FL_OPCODE | FL_CODE;
	h->datalen    = 1;
	h->dataptr    = (BYTE*)malloc(h->datalen + 1);
	h->dataptr[0] = 0x90;
	return h;
}

HOOY *GenerateInt() {
	HOOY *h       = (HOOY*)calloc(1, sizeof(HOOY));
	h->flags      = FL_EXECUTABLE | FL_GENERATED | FL_PRESENT | FL_VPRESENT | FL_OPCODE | FL_CODE;
	h->datalen    = 1;
	h->dataptr    = (BYTE*)malloc(h->datalen + 1);
	h->dataptr[0] = 0xCC;
	return h;
}

int RemoveFixups(Bent *m, HOOY *start, HOOY *end) {
	//generate PIC code to fix all fixups
	//place at entry
	return 0;
}

int RemoveImports(Bent *m) {
	return 0;
}

IMAGE_SECTION_HEADER *GetContainingSection(Bent *b, size_t rva) {
	IMAGE_SECTION_HEADER *oe = IMAGE_FIRST_SECTION(b->nt);

	for (size_t i = 0; i < b->nt->FileHeader.NumberOfSections; i++) {
		if (rva >= oe[i].VirtualAddress &&
			rva <= (oe[i].VirtualAddress + oe[i].Misc.VirtualSize)) {
			return MakePtr(IMAGE_SECTION_HEADER*, b->memb, MakeDelta(void*, &oe[i], b->physMem));
		}
	}

	return NULL;
}

int GenerateDecryptFunc(Bent *m, HOOY *data, HOOY *key, size_t len, HOOY *at) {
	GeneratedBlock gb(m);

	// preserve regs
	gb.GenOp("pushad");
	//gb.GenOpR("push ", R_EAX);
	//gb.GenOpR("push ", R_ESI);
	//gb.GenOpR("push ", R_ECX);
	//gb.GenOpR("push ", R_EDX);

	// create labels for refs
	HOOY *dataLabel = gb.GenLabel();
	dataLabel->flags |= FL_DREF;
	gb.Detach(dataLabel);
	HOOY *keyLabel = gb.GenLabel();
	keyLabel->flags |= FL_DREF;
	gb.Detach(keyLabel);

	// mov eax, offset data
	HOOY *h = gb.GenOpRC("mov ", R_EAX, MakePtr(size_t, dataLabel->oldrva, m->imageBase));
	h->flags |= FL_FIXUP;
	h->arg1   = dataLabel->oldrva;
	h->arg2   = h->datalen - 4;
	HOOY *dataMov = h;

	// mov esi, offset key
	h = gb.GenOpRC("mov ", R_ESI, MakePtr(size_t, keyLabel->oldrva, m->imageBase));
	h->flags |= FL_FIXUP;
	h->arg1   = keyLabel->oldrva;
	h->arg2   = h->datalen - 4;
	HOOY *keyMov = h;

	// mov ecx, len
	gb.GenOpRC("mov ", R_ECX, len);

	//gb.Attach(GenerateInt());
	// decrypt:
	h = gb.GenLabel();
	gb.GenOpRM("mov ", R_EDX, R_ESI, 0, 0, 0); // mov edx, [esi]
	gb.GenOpMR("xor ", R_EAX, 0, 0, 0, R_EDX); // xor [eax], edx

	gb.GenOpRC("add ", R_EAX, 4);
	gb.GenOpRC("add ", R_ESI, 4);
	gb.GenOpRC("sub ", R_ECX, 4);
	gb.GenJCC(C_JNE, h);                       // jne decrypt

	//restore regs
	gb.GenOp("popad");
	//gb.GenOpR("pop ", R_EDX);
	//gb.GenOpR("pop ", R_ECX);
	//gb.GenOpR("pop ", R_ESI);
	//gb.GenOpR("pop ", R_EAX);

	gb.Attach(dataLabel);
	gb.Attach(data); //gb.GenData(data->dataptr, data->datalen);

	// jmp over data
	HOOY *pastData = gb.GenLabel();
	gb.Detach(pastData);
	gb.GenJMP(pastData);

	gb.Attach(keyLabel);
	gb.Attach(key);//gb.GenData(key->dataptr, key->datalen);

	gb.Attach(pastData);

	// make containing section writable
	IMAGE_SECTION_HEADER *oe = GetContainingSection(m, at->oldrva);
	if (oe) {
		oe->Characteristics |= 0x80000000;
	} else {
		return 2;
	}

	if (gb.Valid()) {
		gb.Print();
		gb.AttachToMainList(at);
		return 0;
	}
	return 1;
}

void EncryptData(void *data, void *key, size_t len) {
	for (size_t i = 0; i < len; i++) {
		((BYTE*)data)[i] ^= ((BYTE*)key)[i];
	}
}

int EncryptHooyCode(Bent *m, HOOY *start, HOOY *end) {
	size_t totalSize = 0;

	for (HOOY *h = start; h; h = h->next) {
		if (h == end) {
			break;
		}
		totalSize += h->datalen;
	}

	if (totalSize < 4) {
		totalSize = 4;
	}

	void *encKey = calloc(1, totalSize + 1);
	for (size_t i = 0; i < totalSize; i++) {
		//((BYTE*)encKey)[i] = (rand() % 0xFF);
		((BYTE*)encKey)[i] = 77;
	}

	void *encData = calloc(1, totalSize + 1);
	memset(encData, 0x90909090, totalSize);
	size_t i = 0;

	for (HOOY *h = start; h; h = h->next) {
		if (h == end) {
			break;
		}
		memcpy(MakePtr(void*, encData, i), h->dataptr, h->datalen);
		i += h->datalen;
	}

	for (size_t i = 0; i < totalSize; i++) {
		*(MakePtr(BYTE*, encData, i)) ^= ((BYTE*)encKey)[i];
	}

	HOOY *dataHooy    = (HOOY*)calloc(1, sizeof(HOOY));
	dataHooy->dataptr = (BYTE*)encData;
	dataHooy->datalen = totalSize;
	dataHooy->flags  |= FL_PRESENT | FL_VPRESENT | FL_DATA | FL_GENERATED;

	HOOY *keyHooy    = (HOOY*)calloc(1, sizeof(HOOY));
	keyHooy->dataptr = (BYTE*)encKey;
	keyHooy->datalen = totalSize;
	keyHooy->flags  |= FL_PRESENT | FL_VPRESENT | FL_DATA | FL_GENERATED;

	start->prev->next = end->next;
	end->next->prev   = start->prev;
	HOOY *at = end->next;
	start->prev = end->next = NULL;

	int res = GenerateDecryptFunc(m, dataHooy, keyHooy, totalSize, at);

	return res;
}

int TCrypt(Bent *b) {
	HOOY *entry = b->GetHOOYByOldRVA(b->nt->OptionalHeader.AddressOfEntryPoint, FL_OPCODE);

	//return 0;
	return EncryptHooyCode(b, entry, entry->next);
}

//CFunction *GenerateOneWayTransform(CGen *g) {

/*

int GenerateCDecryptor(BYTE *p, size_t len) {
}

enum {
	A_RELOC,
	A_IMPORT,
	A_TLS,
	A_EP,
	A_CRYPT,
	A_DECRYPT, // <- turns into -> 1+x A_DECRYPT, A_XOR, A_CONTEXTSHIFT, A_CDOEHASH
	A_WIPE,
	A_ANTIEMU,
	A_ANTIDEBUG
};

// possibly A_FUNCTION
// A_FUNCCALL


class Action {
public:
	int type;
	size_t a1, a2, a3;
};

//Action *CreateAction(int type, size_t _a1, size_t _a2, size_t _a3);;
//Action::Action(int type, size_t _a1, size_t _a2, size_t _a3);

//maybe add recursive action generation,
//or either just randomize/generate/obfuscate actions after definition
//A_WIPE - able to wipe/garble app-specific/constant data after it is used
//

Action *CreateAction(int type, size_t _a1, size_t _a2, size_t _a3) {
	Action *res = new Action(type, _a1, _a2, _a3);

	switch (type) {
	case A_RELOC:
		break;
		
	case A_IMPORT,
		break;
		
	case A_TLS,
		break;
		
	case A_EP,
		break;
		
	case A_CRYPT,
		break;
		
	case A_WIPE,
		break;
		
	case A_ANTIEMU,
		break;
		
	case A_ANTIDEBUG
		break;

	default:
		break;
	}

	return res;
}


*/

//PermutateActions
//ObfuscateActions
//GenerateDecoyActions
//GenerateAntiDEbugAction
//Generate
//
//CFG Generation...
//the question is,
//how to generate dummy CFG/code, and THEN inject actions into it...
//

char currentCode[4096*2];
#define APPENDCODE(x) strcat_s(currentCode, sizeof(currentCode), x)
#define APPENDCODEF(fmt) strcatf_s(currentCode, sizeof(currentCode), fmt
#define APPENDCODEX strcatf_s(currentCode, sizeof(currentCode)

int CCrypt(Bent *b) {
	CGen g;

	char *dHeader = "typedef unsigned char BYTE;\n" \
		"typedef unsigned int DWORD;\n";
	char *code = "char code[] = {0x90,0x90,0xC3};\n" \
		"int De(unsigned char *d, unsigned int l, unsigned char *k) {\n" \
		"for(int i=0;i<l;i++){\nd[i]^=k[i];\n}\n}";

	int res = g.AddCode(dHeader);

	/*
	BYTE d[] = {0x90,0x90,0x90,0xC3};
	size_t dLen = sizeof(d);
	strcpy(currentCode, "char code[] = {");
	for (size_t i = 0; i < dLen; i++) {
		if (i != 0) {
			//APPENDCODEF(",0x%02X"), d[i]);
			//APPENDCODEX, ",0x%02X", d[i]);
			strcatf_s(currentCode, sizeof(currentCode), ",0x%02X", d[i]);
		} else {
			strcatf_s(currentCode, sizeof(currentCode), "0x%02X", d[i]);
		}
	}
	strcat_s(currentCode, sizeof(currentCode), "};\n");
	*/

	strcat_s(currentCode, sizeof(currentCode), "int De(unsigned char *d, unsigned int l, unsigned char *k) {\nint i;\n");

	int numRounds = RAND(2, 5);
	for (int i = 0; i < numRounds; i++) {
		if (RANDMAX(2)) {
			strcat_s(currentCode, sizeof(currentCode), "for (i=0;i<l;i++) {\n");
		} else {
			strcat_s(currentCode, sizeof(currentCode), "for (i=l;i>0;i--) {\n");
		}
		int numStatements = RAND(2, 6);
		for (int j = 0; j < numStatements; j++) {
			strcat_s(currentCode, sizeof(currentCode), "d[i] ^= (");
			int numVariables = RAND(3, 7);
			char randOperator[] = "^^^+-|&";
			for (int k = 0; k < numVariables; k++) {
				if (k > 0) {
					int rOp = RAND(0,6);
					char rOpsz[4];
					rOpsz[0] = randOperator[rOp];
					rOpsz[1] = 0;
					strcat_s(currentCode, sizeof(currentCode), rOpsz);
					//strcat_s(currentCode, sizeof(currentCode), " ^ ");
				}
				//if (RANDMAX(3) {
					int shift = 2 * RANDMAX(16);
					strcatf_s(currentCode, sizeof(currentCode), "(k[i]<<%i)",shift);
				//} else {
					//generate dummy one way function
					//better:
					//GenerateExpression    <-|
					//GenerateVariable        |
					//GenerateFunction  ------|
					//int GenerateFunction(char *name, 
				//}
			}
			strcat_s(currentCode, sizeof(currentCode), ");\n");
			
			strcat_s(currentCode, sizeof(currentCode), "k[i] ^= (");
			numVariables = RAND(3, 7);
			for (int k = 0; k < numVariables; k++) {
				if (k > 0) {
					strcat_s(currentCode, sizeof(currentCode), " ^ ");
				}
				int shift = 2 * RANDMAX(16);
				strcatf_s(currentCode, sizeof(currentCode), "(k[k[i]]<<%i)",shift);
			}
			strcat_s(currentCode, sizeof(currentCode), ");\n");
		}
		strcat_s(currentCode, sizeof(currentCode), "}\n");
	}
	strcat_s(currentCode, sizeof(currentCode), "return 0;\n}\n");

	res |= g.AddCode(currentCode);
	//JFunction *de = g.AddFunction("De", CDECL, RT_VOID, 3);

	if (res == 0) {
		res = g.Compile();
		if (res == 0) {
			DWORD rva = g.FindSymbol("De");
			//CryptType crypt = MakePtr(CryptType, g.output, rva);
			if (rva != -1) {
				g.b->flag[rva] |= FL_ENTRY | FL_SIGNATURE | FL_CREF | FL_LABEL;
				res = g.Reverse();
				if (0 == res) {
					g.Insert(b->GetHOOYByOldRVA(b->nt->OptionalHeader.AddressOfEntryPoint, FL_OPCODE));
					//GeneratedBlock gb(g.b, g.b->hooyList.root, g.b->hooyList.tail);
					//GenPrologue
					//GenCall(decryptorHooy)
					//GenCall(decryptedCode);
					//GenEpilogue
					//gb.Link
					DEBUG_LOG("------------TCC:\n");
					g.b->DumpList();
				}
			}
		}
	}

	return res;
}

int Mutate86(Bent *b) {
	int res = 0;

	HOOY *firstOpcode = NULL; // b->GetHOOYByOldRVA(0x1000, FL_DATA | FL_OPCODE)
	for (HOOY *h = (HOOY*)b->hooyList.root; h != NULL; h = h->next) {
		if (h->flags & FL_OPCODE) {
			firstOpcode = h;
			break;
		}
	}

	b->hooyList.insert_before(GenerateNop(), firstOpcode);

	//TCrypt(b);
	//res = CCrypt(b);

	return res;
}
