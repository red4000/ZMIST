#ifndef PATCH_H
#define PATCH_H

int PatchBlock(Bent *b, HOOY *start, HOOY *end, HOOY *with); // patch start..end with single hooy 'with' and pad to keep the size
int PatchX86(Bent *b);

#endif
