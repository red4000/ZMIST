#ifndef XALLOC
#define XALLOC

class XAlloc {
public:
	XAlloc();
	~XAlloc();

	void *xmalloc(size_t len);
	void *xmalloc(size_t count, size_t len);
	void xfree(void *p);
	void xfreeall();
};

#endif
