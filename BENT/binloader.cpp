#include "bent.h"

int LoadBin(Bent *b) {
	int res = 0;

	b->fileType = FT_BIN;
	b->AllocBuffers(b->physSize);
	memcpy(b->memb, b->physMem, b->size);

	for (size_t i = 0; i < b->size; i++) {
		b->flag[i] |= FL_PRESENT | FL_VPRESENT | FL_EXECUTABLE;
	}
	
	b->flag[0] |= FL_ENTRY | FL_CREF;

	return res;
}
