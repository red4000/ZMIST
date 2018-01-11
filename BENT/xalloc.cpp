#include <stdlib.h>
#include <string.h>
#include "log.h"
#include "xalloc.h"

XAlloc::XAlloc() {

}

XAlloc::~XAlloc() {

}

void *XAlloc::xmalloc(size_t len) {
	void *res = malloc(len);
	return res;
}

void *XAlloc::xmalloc(size_t count, size_t len) {
	void *res = calloc(1, len);
	return res;
}

void XAlloc::xfree(void *p) {
	free(p);
}

void XAlloc::xfreeall() {
	
}
