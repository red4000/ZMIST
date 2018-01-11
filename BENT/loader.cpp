#include "bent.h"
#include "loader.h"
#include "peloader.h"
#include "peloader64.h"
#include "binloader.h"

int LoadCode(Bent *b) {
	int res = 0;
	b->archWordSize = 4;

	if (b->mz->e_magic == IMAGE_DOS_SIGNATURE) {
		b->nt = MakePtr(IMAGE_NT_HEADERS32*, b->mz, b->mz->e_lfanew);

		if (b->nt->Signature == IMAGE_NT_SIGNATURE) {
			if (b->nt->FileHeader.Machine == IMAGE_FILE_MACHINE_I386) {
				res         = LoadPE(b);
			} else if (b->nt->FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64) {
				b->fileType     = FT_PE64;
				b->isX64        = 1;
				b->archWordSize = 8;
				res         = LoadPE64(b);
			} else {
				res = 1;
			}
		} else {
			res = 1;
		}
	} else {
		LoadBin(b);
	}

	return res;
}
