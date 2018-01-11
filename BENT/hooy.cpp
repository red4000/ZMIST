#include "bent.h"
#include <stdio.h>
#include "log.h"

int LoadHooy(CHooyList *l, void *p, size_t len) {	
	HOOY  *h = (HOOY*)p, *t = NULL;
	BYTE  *b = (BYTE*)p;
	size_t i = 0;
	l->root = (list_entry*)h;
	while (i < len) {
		h = (HOOY*)&b[i];
		if (t) {
			t->next = h;
			h->prev = t;
		}
		i += sizeof(HOOY);
		if (h->dataptr) {
			h->dataptr = &b[i];
			i         += h->datalen;
		}
		if (h->diza) {
			h->diza = (XDA*)&b[i];
			i      += sizeof(XDA);
		}
		if (h->cmt) {
			h->cmt = (char*)&b[i];
			i     += strlen(h->cmt) + 1;
		}
		if (NULL != h->next) {
			t = h;
		} else {
			break;
		}
	}
	l->tail = (list_entry*)t;
	return 0;
}

int LoadHooy(CHooyList *l, char *fName) {
	void *p;
	FILE *f = fopen(fName, "rb");
	if (f) {
		fseek(f, 0, SEEK_END);
		size_t len = ftell(f);
		rewind(f);
		p = malloc(len + 1);
		if (p) {
			//free(p);
			fread(p, len, 1, f);
			fclose(f);
			return LoadHooy(l, p, len);
		}
		fclose(f);
		return 2;
	}
	return 1;
}

int SaveHooy(CHooyList *l, void **p, size_t *len) {
	size_t outLen = 0;
	for (HOOY *h = (HOOY*)l->root; h != NULL; h = h->next) {
		outLen += sizeof(HOOY);
		if (h->datalen) {
			outLen += h->datalen;
		}
		if (h->diza) {
			outLen += sizeof(XDA);
		}
		if (h->cmt) {
			outLen += strlen(h->cmt) + 1;
		}
	}
	*len = outLen;
	*p   = calloc(1, outLen + 1);
	if (NULL == *p) {
		return 1;
	}
	BYTE  *outb = (BYTE*)*p;
	size_t outi = 0;
	for (HOOY *h = (HOOY*)l->root; h != NULL; h = h->next) {
		memcpy(&outb[outi], h, sizeof(HOOY));
		outi += sizeof(HOOY);
		if (h->datalen) {
			memcpy(&outb[outi], h->dataptr, h->datalen);
			outi += h->datalen;
		}
		if (h->diza) {
			memcpy(&outb[outi], h->diza, sizeof(XDA));
			outi += sizeof(XDA);
		}
		if (h->cmt) {
			size_t cmtLen = strlen(h->cmt) + 1;
			memcpy(&outb[outi], h->cmt, cmtLen);
			outi += cmtLen;
		}
	}
	return 0;
}

int SaveHooy(CHooyList *l, char *fName) {
	void  *p = NULL;
	size_t len = 0;
	int res = SaveHooy(l, &p, &len);
	if (0 == res) {
		FILE *f = fopen(fName, "wb");
		if (f) {
			fwrite(p, len, 1, f);
			fflush(f);
			fclose(f);
			return 0;
		}
		return 1;
	}
	return res;
}
