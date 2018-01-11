#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "bent.h"
#include "generator.h"
#include "cgen.h"
#include "log.h"

CGen::CGen() {
	s = tcc_new();
	if (!s) {
		DEBUG_LOG("Couldn't create TCC state\n");
		exit(9);
	}

	tcc_set_lib_path(s, "C:\\ausb\\usb.tar\\tcc-0.9.26-win32-bin\\tcc\\");
	tcc_set_output_type(s, TCC_OUTPUT_MEMORY);
	//tcc_set_options(s, "-fno-common");
	//tcc_set_options(s, "-static");
	codes.reserve(32);
}

CGen::~CGen() {
	tcc_delete(s);
}

int CGen::AddCode(char *code) {
	return tcc_compile_string(s, code);
}

int CGen::Compile() {
	for (int i = 0; i < codes.size(); i++) {
		int res = tcc_compile_string(s, codes[i]);
		if (res != -1) {
		} else {
			DEBUG_LOG("error compiling code:\n%s\n", codes[i]);
			return 1;
		}
	}
	int size = tcc_relocate(s, (void*)0);
	if (size < 0)
		return 1;
	output = (BYTE*)calloc(1, size + 1);
	outputSize = size;
	int res = tcc_relocate(s, (void*)output);
	if (res != -1) {
		b = new Bent();

		/*
		Bent *tb = new Bent();
		BYTE *toutput = (BYTE*)calloc(1, size + 1);
		int tsize = size;
		res = tcc_relocate(s, (void*)toutput);
		//tsize = tcc_relocate(s, (void*)0);
		if (tsize != size) {
			return 1;
		}
		//res = tcc_relocate(s, (void*)toutput);
		res = tb->LoadMemory(toutput, tsize, (void*)toutput);

		res = b->LoadMemory(output, outputSize, (void*)output);

		int count = 0;

		for (size_t i = 0; i < tsize; i++) {
			if (b->physMem[i] != tb->physMem[i]) {
				count++;
			}
		}*/
		res = b->LoadMemory(output, outputSize, (void*)output);
		//b->flag[0] &= ~(FL_CREF | FL_ENTRY);

		/*
		for (size_t i = 0; i < outputSize - 3; i++) {
			DWORD x = *(DWORD*)&b->physMem[i];
			if (x >= (DWORD)output &&
				x <= MakePtr(DWORD, output, outputSize)) {
				size_t y;
				if (i >= 8) {
					y = 8;
				} else {
					y = i;
				}
				for (size_t z = 0; z < y; z++) {
					if (b->flag[i - z] & FL_OPCODE) {
						XDA *d = b->DisassembleOpcode(i - z);
						if (d->len >= z) {
							//probable fixup
							b->flag[i] |= FL_FIXUP;
						} else {
							DEBUG_LOG("???\n");
						}
					}
				}
			}
		}
		*/

		return res;
	}
	return res;
}

int CGen::Reverse() {
	//b->flag |= M_VIRTUAL; // set analysis to mark absolute VAs as fixups
	int res = b->Analyze();
	if (res == 0) {
		res = b->MakeList();
		if (res == 0) {
			res = Optimize();
		}
	}
	return res;
}

int CGen::Optimize() {
	HOOY *t = (HOOY*)b->hooyList.tail;
	if (t->datalen) {
		BYTE x = 0;
		for (int i = 0; i < t->datalen; i++) {
			if (t->dataptr[i] != 0x90 &&
				t->dataptr[i] != 0xC3 &&
				t->dataptr[i] != 0xCC) {
				x |= t->dataptr[i];
			}
		}
		if (0 == x) {
			b->hooyList.tail = (list_entry*)t->prev;
			//HOOY *end = GenHLabel(b);
			//b->hooyList.tail = (list_entry*)end;
			//FreeHooy(t);
		}
	}
	t = (HOOY*)b->hooyList.root;
	if (t->datalen) {
		BYTE x = 0;
		for (int i = 0; i < t->datalen; i++) {
			if (t->dataptr[i] != 0x90 &&
				t->dataptr[i] != 0xC3 &&
				t->dataptr[i] != 0xCC) {
				x |= t->dataptr[i];
			}
		}
		if (0 == x) {
			b->hooyList.root = (list_entry*)t->next;
			//HOOY *end = GenHLabel(b);
			//FreeHooy(t);
		}
	}

	HOOY *h = (HOOY*)b->hooyList.root;
	while (h) {
		HOOY *n = h->next;
		if (h->flags & FL_OPCODE) {
			if (h->datalen == 1) { 
				GeneratedBlock gb(b);
				int res;
				switch (h->dataptr[0]) {
				case 0x90: // nop
					b->hooyList.detach((void*)h);
					//FreeHooy(h);
					break;

				case 0xC9: // leave
					gb.GenOpRR("mov ", R_ESP, R_EBP);
					gb.GenOpR("pop ", R_EBP);
					res = gb.Link();
					if (0 < res) {
						b->hooyList.detach((void*)h);
						b->hooyList.insert_block((void*)n, (void*)gb.start, (void*)gb.end);
					}
					break;

				default:
					break;
				}
			} else {
				if (h->dataptr[0] == 0xB8) { // mov eax, 00000000
					if (0 == *(DWORD*)&h->dataptr[1]) {
						GeneratedBlock gb(b);
						gb.GenOpRR("xor ", R_EAX, R_EAX);
						int res = gb.Link();
						if (0 < res) {
							b->hooyList.detach((void*)h);
							//b->hooyList.insert_block((void*)n, (void*)gb.start, (void*)gb.end);
							b->hooyList.insert_before(gb.start, n);
						}
					}
				} else if (h->dataptr[0] == 0xE9) {
					if (*(DWORD*)&h->dataptr[1] == 0) {
						b->hooyList.detach((void*)h);
					}
				} else {
/*
					if (h->prev) {
						if (h->prev->prev) {
							if (h->prev->prev->flags & FL_OPCODE) {
								if (h->prev->flags & FL_OPCODE) {
									if (h->prev->prev->diza->d.Argument1.ArgType & REGISTER_TYPE)) {
										if (h->prev->diza->d.Argument1.ArgType & REGISTER_TYPE)) {
											if (h->diza->d.Argument1.ArgType & REGISTER_TYPE)) {
												if (h->diza->d.Argument2.ArgType & REGISTER_TYPE)) {
												}
											}
										}
									}
								}
							}
						}
					}
00000030|00000000|8B4508          |mov eax, dword ptr [ebp+08h]    |FL_OPCODE,FL_EXECUTABLE,
00000033|00000000|8B4DFC          |mov ecx, dword ptr [ebp-04h]    |FL_OPCODE,FL_EXECUTABLE,
00000036|00000000|01C8            |add eax, ecx                    |FL_OPCODE,FL_EXECUTABLE,
*/
				}
			}
		}
		h = n;
	}

	//ReAssemble(b)
	//b.Assemble()

	//realign all funcs to 0xF boundary
	h = (HOOY*)b->hooyList.root;
	while (h) {
		HOOY *n = h->next;
		if (h->flags & FL_CREF) {
			if (h->flags & FL_LABEL) {
			}
		}
		h = n;
	}
	return 0;
}

DWORD CGen::FindSymbol(char *sym) {
	void *sm = tcc_get_symbol(s, sym);

	if (sm) {
		return MakeDelta(DWORD, sm, output);
	}

	return -1;
}

void CGen::Insert(HOOY *at) {
	HOOY *x                = at->prev;
	b->hooyList.root->last = (list_entry*)at->prev;
	b->hooyList.tail->next = (list_entry*)at;
	at->prev               = (HOOY*)b->hooyList.tail;
	x->next                = (HOOY*)b->hooyList.root;
}

char tbuf[1024*8];

void strcatf_s(char *buf, size_t len, char *fmt, ...) {
	va_list valist;

	va_start(valist, fmt);
	vsprintf_s(tbuf, len, fmt, valist);
	va_end(valist);
	strcat_s(buf, len, tbuf);
}
