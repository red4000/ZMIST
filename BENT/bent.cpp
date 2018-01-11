#include "bent.h"
#include <stdio.h>
#include "log.h"
#include "loader.h"
#include "analysis.h"
#include "reassemble.h"
#include "dbginfo.h"

Bent::Bent() {
	memset(this, 0, sizeof(Bent));
	hooyList.entry_size = sizeof(HOOY);
}

Bent::~Bent() {
	SAFE_FREE(physMem);
	SAFE_FREE(memb);
	SAFE_FREE(flag);
	SAFE_FREE(arg1);
	SAFE_FREE(arg2);
	SAFE_FREE(insn);
	if (comment) {
		for (size_t i = 0; i < size; i++) {
			if (comment[i]) {
				//if (comment[i][0] != '|') { // to not try to free static strings
				//free(comment[i]);
			}
		}
		free(comment);
	}
	SAFE_FREE(outputMem);
}

int Bent::LoadMemory(BYTE *p, DWORD len, void *base) {
	physMem   = memb = p;
	physSize  = size = len;
	imageBase = base;
	return LoadCode(this);
}

int Bent::LoadFile(char *_path) {
	int res = 0;

	FILE *f;
	path = _path;
	fopen_s(&f, path, "rb");

	if (f) {
		fseek(f, 0, SEEK_END);
		physSize = ftell(f);
		rewind(f);
		physMem = (BYTE*)calloc(1, physSize + 1);
		fread(physMem, physSize, 1, f);
		fclose(f);
		DEBUG_LOG("Loading %s %i(%X)\n", path, physSize, physSize);
		res = LoadCode(this);
	} else {
		res = 1;
	}

	return res && (errorCount == 0);
}

int Bent::Analyze() {
	AnalysisContext a;
	return a.Analyze(this);
}

int Bent::MakeList() {
	int res = 0;

	for (size_t i = 0; ((i < size) || (i == size && flag[i])); ) {
		DWORD  l;
		size_t fix1 = 0, fix2 = -1, fix3 = 0, fix4 = -1;

		if (flag[i] & (FL_LABEL | FL_SECTALIGN)) {
			l = 0;
		} else if (flag[i] & FL_OPCODE) {
			XDA *da = DisassembleOpcode(i);
			l       = da->len;

			for (unsigned int t = 1; t < l; t++) {
				if (flag[i + t] & FL_FIXUP) {
					if (fix1 == 0) {
						fix1 = arg1[i + t];
						fix2 = t;
					} else {
						fix3 = fix1;
						fix4 = fix2;

						fix1 = arg1[i + t];
						fix2 = t;
					}
				}

				/*if ((flag[i + t] & (~FL_FIXUP)) != (FL_CODE | FL_ANALYZED | FL_PRESENT | FL_VPRESENT)) {
				Log("error: building list at %08X+%d\n", i, t);
				errorCount++;
				flag[i] |= FL_ERROR;
				l        = t;
				break;
				}*/
			}
		} else if (flag[i] & (FL_ARVA | FL_RVA | FL_DELTA | FL_FIXUP)) {
			l = archWordSize;
		} else {
			for (l = 1; (i + l) < size; l++) {
				if (flag[i] != flag[i + l]) {
					break;
				}
			}

			flag[i] |= FL_DATA;
		}

		HOOY *t = (HOOY*)hooyList.alloc();

		if (t == NULL) {
			errorCount++;
			return 1;
		}

		t->flags   = flag[i];
		t->oldrva  = i;
		t->datalen = l;

		if (l) {
			if (nt && t->oldrva < nt->OptionalHeader.SizeOfHeaders) {
				t->flags  |= FL_HEADER;
				t->dataptr = &memb[i];
			} else {
				t->dataptr = &memb[i];
				t->diza    = DisassembleOpcode(i);
			}
		} else {
			t->dataptr = NULL;
		}

		t->arg1 = arg1[i];
		t->arg2 = arg2[i];
		t->next = NULL;

		if (fix2 != -1) {
			t->flags |= FL_FIXUP;
			t->arg1   = fix1;
			t->arg2   = fix2;
		}

		if (fix4 != -1) {
			t->flags |= FL_FIXUP;
			t->arg3   = fix3;
			t->arg4   = fix4;
		} else {
			t->arg3 = 0;
			t->arg4 = 0;
		}

		if (flag[i] & FL_SECTALIGN) {
			t->flags &= FL_SECTALIGN;
			flag[i]  &= ~(FL_SECTALIGN);
		} else if (flag[i] & FL_LABEL) {
			t->flags &= FL_LABEL | FL_CREF | FL_DREF;
			flag[i]  &= ~(FL_LABEL | FL_CREF | FL_DREF);

			/*
			if (hooyList.tail &&
				((HOOY*)hooyList.tail)->flags & FL_SECTALIGN) {
				t->flags |= FL_SECTALIGN;
				hooyList.detach(hooyList.tail);
			}
			*/
		}

		if (flag[i] & FL_COMMENT) {
			t->cmt = comment[i];
		}

		hooyList.attach(t);

		i += l;
	}
	DEBUG_LOG("%i list entries, size %i(%X)\n", hooyList.count, hooyList.count * hooyList.entry_size, hooyList.count * hooyList.entry_size);

	free(flag);
	free(arg1);
	free(arg2);
	flag = arg1 = arg2 = NULL;

	return res;
}

int Bent::Assemble() {
	switch (fileType) {
	case FT_PE:
		return AssembleX86(this);

	default:
	case FT_BIN:
		return AssembleX86Bin(this);
	}
}

int Bent::SaveFile(char *_path) {
	int res = 0;

	FILE *f;
	fopen_s(&f, _path, "wb");

	if (f) {
#ifdef _DEBUG
		char fullPath[MAX_PATH];
		GetFullPathNameA(_path, MAX_PATH, fullPath, NULL);
		DEBUG_LOG("Writing %i(%X) bytes to %s - %s\n", outputSize, outputSize, _path, fullPath);
#endif
		fwrite(outputMem, outputSize, 1, f);
		fclose(f);
	} else {
		res = 1;
	}

	return res;
}

void Bent::DumpRVA(size_t rva) {

}

void Bent::DumpBinary() {
	
}

void Bent::DumpList() {
	Log("---BList---\nOVA     |NVA     |DATA            |OPCODE                          |FLAGS\n");
	for (HOOY *h = (HOOY*)hooyList.root; h != NULL; h = h->next) {
		char data[64];
		data[0] = 0;
		if (h->datalen) {
			size_t truncatedLen = h->datalen;
			if (truncatedLen > 8) {
				truncatedLen = 8;
			}
			data[0] = 0;
			for (size_t i = 0; i < truncatedLen; i++) {
				char byteStr[8];
				sprintf_s(byteStr, sizeof(byteStr), "%02X", h->dataptr[i]);
				strcat_s(data, sizeof(data), byteStr);
			}
		}

		char flags[128];
		flags[0] = 0;

#define LOGFLAG(x) if (h->flags & x) { strcat_s(flags, sizeof(flags), #x); strcat_s(flags, sizeof(flags), ","); }
		LOGFLAG(FL_RVA);
		LOGFLAG(FL_DELTA);
		LOGFLAG(FL_FORCEFILEALIGN);
		LOGFLAG(FL_FORCEOBJALIGN);
		LOGFLAG(FL_FIXUP);
		LOGFLAG(FL_LABEL);
		LOGFLAG(FL_CREF);
		LOGFLAG(FL_DREF);
		LOGFLAG(FL_OPCODE);
		LOGFLAG(FL_HAVEREL);
		LOGFLAG(FL_STOP);
		LOGFLAG(FL_IMPORT);
		LOGFLAG(FL_SWITCH);
		LOGFLAG(FL_SECTALIGN);
		LOGFLAG(FL_DATA);
		LOGFLAG(FL_SIGNATURE);
		LOGFLAG(FL_ENTRY);
		LOGFLAG(FL_EXECUTABLE);

		char opcode[256];
		if (h->flags & FL_OPCODE) {
			strcpy_s(opcode, sizeof(opcode), h->diza->d.CompleteInstr);
		} else {
			opcode[0] = 0;
			if (h->flags & FL_RVA) {
				sprintf_s(opcode, sizeof(opcode), "--RVA %08X", h->arg1);
			} else if (h->flags & FL_ARVA) {
				sprintf_s(opcode, sizeof(opcode), "--ARVA %08X", h->arg1);
			} else if (h->flags & FL_DELTA) {
				sprintf_s(opcode, sizeof(opcode), "--Del %08X %08X", h->arg1, h->arg2);
			} else if (h->flags & FL_FIXUP) {
				sprintf_s(opcode, sizeof(opcode), "--Fix %08X", h->arg1);
			} else if (h->flags & FL_LABEL) {
				Symbol *s = GetSymbolForRVA(h->oldrva);
				if (s) {
					sprintf_s(opcode, sizeof(opcode), "-%s:", s->name);
				}
				char xref[64];
				if (h->arg1) {
					if (h->flags & FL_CREF) {
						sprintf_s(xref, sizeof(xref), " CREF %08X", h->arg1);
					} else {
						sprintf_s(xref, sizeof(xref), " DREF %08X", h->arg1);
					}
					strcat_s(opcode, sizeof(opcode), xref);
				}
			} else if (h->flags & FL_DATA) {
				sprintf_s(opcode, sizeof(opcode), "(%08X..%08X)", h->newrva, h->newrva + h->datalen);
			}
		}
		
		if (h->flags & FL_COMMENT) {
			char cmtBuf[256];
			sprintf_s(cmtBuf, sizeof(cmtBuf), " ; %s", (char*)h->cmt);
			strcat_s(opcode, sizeof(opcode), cmtBuf);
		}

		Log("%08X|%08X|%-16s|%-32s|%s\n", h->oldrva, h->newrva, data, opcode, flags);
	}
}

void Bent::DumpSymbols() {

}

XDA *Bent::DisassembleOpcode(size_t rva) {
	if (rva > size) {
		return NULL;
	}
	XDA *res = insn[rva];
	if (res) {
		return res;
	} else {
		XDA ins;
		memset(&ins, 0, sizeof(XDA));
		int len;
		if (0 == this->isX64) {
			len = XDisasm(&ins, &memb[rva], rva);
		} else {
			len = XDisasm64(&ins, &memb[rva], rva);
		}
		if (len) {
			res = (XDA*)malloc(sizeof(XDA));
			memcpy(res, &ins, sizeof(XDA));
			insn[rva] = res;
		}
	}

	return res;
}

XDA *Bent::DisassembleOpcode64(size_t rva) {
	if (rva > size) {
		return NULL;
	}
	XDA *res = insn[rva];
	if (res) {
		return res;
	} else {
		XDA ins;
		memset(&ins, 0, sizeof(XDA));
		int len = XDisasm64(&ins, &memb[rva], rva);
		if (len) {
			res = (XDA*)malloc(sizeof(XDA));
			memcpy(res, &ins, sizeof(XDA));
			insn[rva] = res;
		}
	}

	return res;
}

HOOY *Bent::GetHOOYByOldRVA(size_t rva, DWORD flags) {
	HOOY *h;

	ForEachInList(hooyList, HOOY, h) {
		if (h->flags & flags) {
			if (h->oldrva == rva) {
				return h;
			}
		}
	}

	return NULL;
}

Symbol *Bent::GetSymbolForRVA(size_t rva) {
	Symbol *res = NULL;
	for (size_t i = 0; i < symbols.size(); i++) {
		if (symbols[i].rva == rva) {
			res = &symbols[i];
			break;
		}
	}
	return res;
}

DWORD Bent::GenRVA() {
	return size + 2 + (nrva++);
}

int Bent::AllocBuffers(size_t _size) {
	size = _size + 1;
	memb = (BYTE*)calloc(1, size + 1);
	flag = (DWORD*)calloc(1, (size + 1) * sizeof(DWORD));
	arg1 = (DWORD*)calloc(1, (size + 1) * sizeof(DWORD));
	arg2 = (DWORD*)calloc(1, (size + 1) * sizeof(DWORD));
	insn = (XDA**)calloc(1, (size + 1) * sizeof(XDA*));
	comment = (char**)calloc(1, (size + 1) * sizeof(char*));

	if (memb && flag && arg1 && arg2 && insn && comment) {
		return 0;
	} else {
		DEBUG_LOG("Error: Couldn't allocate buffers %X\n", size);
		return 1;
	}
}

void Bent::MarkARVA(size_t rva) {
	size_t x = MakeDelta(size_t, memd[rva], imageBase);
	if (x <= size) {
		flag[rva] |= FL_ARVA | FL_DATA;
		flag[x]   |= FL_LABEL | FL_DREF;
		arg1[rva]  = x;
		if (arg1[x] == 0) {
			arg1[x] = rva;
		}
	} else {
		DEBUG_LOG("Error: ARVA %08X(%08X) out of bounds\n", rva, x);
		errorCount++;
		flag[rva] |= FL_ERROR;
	}
}

void Bent::MarkRVA(size_t rva) {
	size_t x = memd[rva];
	if (x <= size) {
		flag[rva] |= FL_RVA | FL_DATA;
		flag[x]   |= FL_LABEL | FL_DREF;
		arg1[rva]  = x;
		if (arg1[x] == 0) {
			arg1[x] = rva;
		}
	} else {
		DEBUG_LOG("Error: RVA %08X(%08X) out of bounds\n", rva, x);
		errorCount++;
		flag[rva] |= FL_ERROR;
	}
}

void Bent::MarkRVAData(size_t rva) {
	size_t x = memd[rva];
	if (x <= size) {
		flag[rva] |= FL_RVA | FL_DATA;
		flag[x]   |= FL_LABEL | FL_DREF | FL_DATA;
		arg1[rva]  = x;
		if (arg1[x] == 0) {
			arg1[x] = rva;
		}
	} else {
		DEBUG_LOG("Error: RVAData %08X(%08X) out of bounds\n", rva, x);
		errorCount++;
		flag[rva] |= FL_ERROR;
	}
}

void Bent::MarkFixup(size_t rva) {
	size_t x = memd[rva];
	if ((x >= size) &&
		(x < MakePtr(size_t, imageBase, size))) {
		flag[rva] |= FL_FIXUP;
		size_t fix = MakeDelta(DWORD, x, imageBase);
		flag[fix] |= FL_LABEL | FL_DREF;
		arg2[rva]  = 0;
		arg1[rva]  = fix;
		if (arg1[fix] == 0) {
			arg1[fix] = rva;
		}
	} else {
		DEBUG_LOG("Error: Fixup %08X(%08X) out of bounds\n", rva, x);
		errorCount++;
		flag[rva] |= FL_ERROR;
	}
}

void Bent::MarkARVA64(size_t rva) {
	if (memd64[rva] <= size) {
		flag[rva] |= FL_ARVA | FL_DATA;
		flag[memd64[rva]] |= FL_LABEL | FL_DREF;
		arg1[rva] = memd64[rva];
	}
	else {
		Log("ARVA %08X out of bounds\n", rva);
		errorCount++;
		flag[rva] |= FL_ERROR;
	}
}

void Bent::MarkRVA64(size_t rva) {
	if (memd64[rva] <= size) {
		flag[rva] |= FL_RVA | FL_DATA;
		flag[memd64[rva]] |= FL_LABEL | FL_DREF;
		arg1[rva] = memd64[rva];
	}
	else {
		Log("RVA %08X out of bounds\n", rva);
		errorCount++;
		flag[rva] |= FL_ERROR;
	}
}

void Bent::MarkRVAData64(size_t rva) {
	if (memd64[rva] <= size) {
		flag[rva] |= FL_RVA | FL_DATA;
		flag[memd64[rva]] |= FL_LABEL | FL_DREF | FL_DATA;
		arg1[rva] = memd64[rva];
	}
	else {
		Log("RVAData %08X out of bounds\n", rva);
		errorCount++;
		flag[rva] |= FL_ERROR;
	}
}

void Bent::MarkFixup64(size_t rva) {
	size_t x = memd64[rva];
	if ((x >= size) &&
		(x < MakePtr(size_t, imageBase, size))) {
		flag[rva] |= FL_FIXUP;
		size_t fix = MakeDelta(size_t, x, imageBase);
		arg1[rva] = fix;
		arg2[rva] = 0;
		flag[fix] |= FL_LABEL | FL_DREF;
		if (arg1[fix] == 0) {
			arg1[fix] = rva;
		}
	}
	else {
		DEBUG_LOG("Error: Fixup %08X(%08X) out of bounds\n", rva, x);
		errorCount++;
		flag[rva] |= FL_ERROR;
	}
}

void Bent::MarkDelta(size_t x, size_t y, size_t z) {
	if ((y <= size) &&
		(z <= size)) {
		arg1[x]      = y;
		arg2[x]      = y + z;
		flag[x]     |= FL_DELTA;
		flag[y]     |= FL_LABEL | FL_DREF;
		flag[y + z] |= FL_LABEL | FL_DREF;
	} else {
		DEBUG_LOG("Error: Delta %08X(%08X %08X) out of bounds\n", x, y, z);
		errorCount++;
		flag[x] |= FL_ERROR;
	}
}

void Bent::MarkData(size_t rva, size_t len) {
	for (size_t i = 0; i < len; i++) {
		flag[rva + i] |= FL_DATA;
	}
}

void Bent::SetComment(size_t rva, char *cmt) {
	comment[rva] = cmt;
	flag[rva]   |= FL_COMMENT;
}

void Bent::SetCommentf(size_t rva, char *fmt, ...) {
	va_list valist;
	char    cmtBuf[256];

	va_start(valist, fmt);
	int len = vsprintf_s(cmtBuf, sizeof(cmtBuf), fmt, valist);
	va_end(valist);
	char *cmt = (char*)calloc(1, len + 1);
	memcpy(cmt, &cmtBuf[0], len);
	SetComment(rva, cmt);
}
