#include "bent.h"
#include "analysis.h"
#include "log.h"

AnalysisContext::AnalysisContext() {
}

AnalysisContext::~AnalysisContext() {
}

int FollowRel(Bent *m, XDA *insn, Vector<size_t> *next, bool mark = false) {
	size_t rel = (size_t)insn->d.Instruction.AddrValue;

	if (m->size < rel) {//||
						//m->IsFixup(rel)) {
		return 1;
	}
	size_t ip = (size_t)insn->d.VirtualAddr;
	m->arg1[ip] = rel;
	if (insn->d.Argument1.ArgSize == 4) {
		m->arg2[ip] = 4;
	} else {
		m->arg2[ip] = 1;
	}

	if (m->flag[rel] & FL_EXECUTABLE) {
		XDA *diza2 = m->DisassembleOpcode(rel);

		if (diza2) {
			if (m->flag[rel] & FL_CODE &&
				0 == (m->flag[rel] & FL_OPCODE)) {
				return 3;
			}
			if (m->flag[rel] & (FL_FIXUP | FL_DATA)) {
				return 4;
			}
			if (mark) {
				m->flag[rel] |= FL_LABEL | FL_CREF;
			}

			if (0 == (m->flag[rel] & FL_OPCODE)) {
				if (0x5E == m->memb[rel] &&
					0xE8 == m->memb[ip]) {   // special fix for CALL to POP(and call isn't retn'd), in LDE disassembler in hack
					insn->flag |= C_STOP;
					next->push_back(rel);
				} else {
					next->push_back(rel);
				}
			}
			return 0;
		} else {
			return 2;
		}
	} else {
		return 1;
	}
}

int FollowJmp(Bent *m, XDA *insn, Vector<size_t> *next, bool mark = false) {
	size_t ip = (size_t)insn->d.VirtualAddr;

	if (insn->d.Instruction.BranchType == JmpType) {
		if (m->memb[ip] == 0xFF) {
			if (m->memb[ip + 1] == 0x24 || m->memb[ip + 1] == 0x25) {
				size_t ip2 = MakeDelta(size_t, insn->d.Argument1.Memory.Displacement, m->imageBase);

				if (mark) {
					m->flag[ip2] |= FL_LABEL | FL_DREF;
				}

				for (ip2; ip2 < m->size; ip2 += 4) {
					if (m->flag[ip2] & (FL_FIXUP | FL_RVA)) {
						if (0 == (m->flag[ip2] & FL_IMPORT)) {
							size_t dst = m->arg1[ip2];
							if (dst < m->size && m->flag[dst] & FL_EXECUTABLE) {
								if (mark) {
									m->flag[dst] |= FL_LABEL | FL_DREF | FL_CREF;
									for (size_t i = 0; i < 4; i++) {
										m->flag[ip2 + i] |= FL_SWITCH;
									}
								}
								if (0 == (m->flag[dst] & FL_OPCODE)) {
									next->push_back(dst);
								}
							}
						} else {
							break;
						}
					} else {
						break;
					}
				}
				return 0;
			}
		}
	}
	return 1;
}

int AnalysisContext::MarkBlock(size_t rva) {
	int res = 0;

	bNext.clear();
	//cref.clear();

	bNext.push_back(rva);

	size_t ip, ipStart = 0;

	while (bNext.size()) {
		ip = ipStart = bNext[0];
		if (b->flag[ip] & FL_OPCODE) {
			goto markNext;
		}
		while (ip != -1) {
			if (b->flag[ip] & FL_EXECUTABLE) {
				XDA *d = b->DisassembleOpcode(ip);
				if (d) {
					b->flag[ip] |= FL_OPCODE;
					for (int i = 0; i < d->len; i++) {
						b->flag[ip + i] |= FL_CODE;
					}
					if (d->flag & C_REL) {
						b->flag[ip] |= FL_HAVEREL;
						FollowRel(b, d, &bNext, true);
					}
					if (d->flag & C_JMP) {
						FollowJmp(b, d, &bNext, true);
					}
					if (d->flag & C_STOP) {
						b->flag[ip] |= FL_STOP;
						break;
					}
					ip += d->len;
				} else {
					return 3;
				}
			} else {
				return 4;
			}
		}
		markNext:
		bNext.remove(0);
	}

	return res;
}

int AnalysisContext::AnalyzeBlock(size_t rva) {
	int res = 0;

	bNext.clear();
	cref.clear();

	bNext.push_back(rva);

	size_t ip, ipStart = 0;

	while (bNext.size()) {
		ip = ipStart = bNext[0];
		if (b->flag[ip] & FL_OPCODE) {
			goto analyzeNext;
		}
		for (size_t i = 0; i < cref.size(); i++) {
			if (cref[i] == ip) {
				goto analyzeNext;
			}
		}
		cref.push_back(ip);
		while (ip != -1) {
			if (b->flag[ip] & FL_EXECUTABLE) {
				if (ip == 0x1715) {
					Log("...");
				}
				XDA *d = b->DisassembleOpcode(ip);
				if (d) {
					for (int i = 0; i < d->len; i++) {
						if (i > 0) {
							if (b->flag[ip + i] & FL_OPCODE) {
								DEBUG_LOG("IP in middle of insn at %08X block %08X start %08X\n", ip, ipStart, rva);
								return 2;
							}
						} else {
							if (0 == (b->flag[ip] & FL_OPCODE)) {
								if (b->flag[ip] & FL_CODE) {
									DEBUG_LOG("IP in middle of insn at %08X block %08X start %08X\n", ip, ipStart, rva);
									return 2;
								}
							}
						}
					}
					if (d->flag & C_REL) {
						int rRes = FollowRel(b, d, &bNext, false);
						if (rRes == 0) {
						} else {
							DEBUG_LOG("Error %i following relative at %08X block %08X start %08X\n", rRes, ip, ipStart, rva);
						}
					}
					//if (d->flag & C_JMP) {
					if (d->flag & (C_JMP | C_REL)) {
						int jRes = FollowJmp(b, d, &bNext, false);
					}
					if (d->flag & C_STOP) {
						break;
					}
					ip += d->len;
				} else {
					return 3;
				}
			} else {
				return 4;
			}
		}
		analyzeNext:
		bNext.remove(0);
	}

	return res;
}

int AnalysisContext::AnalyzeNext() {
	int res = 0;

	while (next.size()) {
		size_t rva = next[0];
		if (b->flag[rva] & FL_OPCODE) {
			goto _next;
		}
		int aRes = AnalyzeBlock(rva);
		if (0 == aRes) {
			MarkBlock(rva);
			DEBUG_LOG("B %08X\n", rva);
			res++;
		} else {
			DEBUG_LOG("Error %i analyzing block %08X\n", aRes, rva);
		}
		_next:
		next.remove(0);
	}
	
	//DEBUG_LOG("\n");

	return res;
}

int AnalysisContext::Analyze(Bent *_b) {
	int res = 0;

	b = _b;
	next.reserve(512);
	bNext.reserve(512);
	cref.reserve(512);

	for (size_t i = 0; i < b->size; i++) {
		if (b->flag[i] & FL_ENTRY) {
			next.push_back(i);
		}
	}

	res = AnalyzeNext();
	DEBUG_LOG("Analyzed %i blocks from EP\n\n", res);

	for (size_t i = 0; i < b->size; i++) {
		if (0 == (b->flag[i] & FL_CODE)) {
			if (b->flag[i] & FL_SIGNATURE) {
				next.push_back(i);
			}
		}
	}

	res = AnalyzeNext();
	DEBUG_LOG("Analyzed %i blocks from debug info\n\n", res);

	for (size_t i = 0; i < b->size; i) {
		if (b->flag[i] & (FL_FIXUP | FL_DELTA | FL_RVA)) {
			i += 4;
		} else {
			if (b->flag[i] & FL_EXECUTABLE) {
				if (0 == (b->flag[i] & (FL_CODE | FL_FIXUP | FL_RVA | FL_DATA | FL_IMPORT))) {
					if (b->flag[i] & FL_DREF) {
						if (0 == (b->flag[i - 4] & FL_FIXUP) ||
							(0xFF == b->memb[i - 6] &&
							 0x25 == b->memb[i - 5])) {
							XDA *d = b->DisassembleOpcode(i);
							if (d && 0 == (d->flag & C_BAD)) {
								next.push_back(i);
							}
						}
					}
				}
			}
			i++;
		}
	}

	res = AnalyzeNext();
	DEBUG_LOG("Analyzed %i blocks from DRefs\n\n", res);

	for (size_t i = 0; i < (b->size - 6); i++) {
		if (b->flag[i] & FL_EXECUTABLE) {
			if (0 == (b->flag[i] & (FL_CODE | FL_FIXUP | FL_RVA | FL_DATA | FL_IMPORT))) {
				if ((i & (~0xF)) == i) {
					if (b->memb[i] == 0x56 &&     // junk in statically linked CRT, alignment bytes
						b->memb[i + 1] == 0x43 &&     // get mistaken for instructions
						b->memb[i + 2] == 0x32 &&
						b->memb[i + 3] == 0x30 &&
						b->memb[i + 4] == 0x58 &&
						b->memb[i + 5] == 0x43) {
						i += 8;
						continue;
					} else if (b->memb[i] == 0x8B &&
						b->memb[i + 1] == 0xFF &&
						b->memb[i + 2] == 0x55 &&
						b->memb[i + 3] == 0x8B) {
						b->flag[i] |= FL_SIGNATURE;
					} else if (b->memb[i] == 0x55 &&
						((b->memb[i + 1] == 0x89 &&
							b->memb[i + 2] == 0xE5) ||
							(b->memb[i + 1] == 0x8B &&
							 b->memb[i + 2] == 0xEC))) {
						b->flag[i] |= FL_SIGNATURE;
					}
				}
				if (b->flag[i] & FL_SIGNATURE) {
					next.push_back(i);
				}
			}
		}
	}

	res = AnalyzeNext();
	DEBUG_LOG("Analyzed %i blocks from aligned signatures\n\n", res);

	for (size_t i = 0; i < (b->size - 6); i++) {
		if (b->flag[i] & FL_EXECUTABLE) {
			if (0 == (b->flag[i] & (FL_CODE | FL_FIXUP | FL_RVA | FL_DATA | FL_IMPORT))) {
				if (b->memb[i] == 0x56 &&     // junk in statically linked CRT, alignment bytes
					b->memb[i + 1] == 0x43 &&     // get mistaken for instructions
					b->memb[i + 2] == 0x32 &&
					b->memb[i + 3] == 0x30 &&
					b->memb[i + 4] == 0x58 &&
					b->memb[i + 5] == 0x43) {
					i += 8;
					continue;
				} else if (b->memb[i] == 0x8B &&
						   b->memb[i + 1] == 0xFF &&
						   b->memb[i + 2] == 0x55 &&
						   b->memb[i + 3] == 0x8B) {
						   b->flag[i] |= FL_SIGNATURE;
				} else if (b->memb[i] == 0x55 &&
						 ((b->memb[i + 1] == 0x89 &&
						   b->memb[i + 2] == 0xE5) ||
						  (b->memb[i + 1] == 0x8B &&
						   b->memb[i + 2] == 0xEC))) {
					b->flag[i] |= FL_SIGNATURE;
				}
				if (b->flag[i] & FL_SIGNATURE) {
					next.push_back(i);
				}
			}
		}
	}

	res = AnalyzeNext();
	DEBUG_LOG("Analyzed %i blocks from signatures\n\n", res);

	/*
	^disassemble
	propagate blocks, cfg, reg usage, xrefs
	*/


	return 0;
}
