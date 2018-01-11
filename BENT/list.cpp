#include <windows.h>
#include "list.h"

list::list()
{
	count      = 0;
	root       = tail = NULL;
	entry_size = 0;
	on_free    = NULL;
}

list::list(size_t entry_size_0)
{
	count      = 0;
	root       = tail = NULL;
	entry_size = entry_size_0;
	on_free    = NULL;
}

void list::empty()
{
	list_entry *x = root;

	while (x)
	{
		list_entry *t = x->next;

		if (on_free)
		{
			on_free(x);
		}

		//free(x); // fucking leaky bugs
		x = t;
	}

	root  = tail = NULL;
	count = 0;
}

list::~list()
{
	empty();
}

void* list::alloc()
{
	return (calloc(entry_size, 1)); // alloc + zerofill
}

void list::attach(void *entry)
{
	count++;

	list_entry *t = (list_entry*)entry;

	t->last = tail;
	t->next = NULL;

	if (root == NULL)
	{
		root = t;
	}
	else
	{
		tail->next = t;
	}

	tail = t;
}

void list::detach(void *entry)
{
	list_entry *t = (list_entry*)entry;

	if (t == root) // (t->prev == NULL)
	{
		root = t->next;
	}
	else
	{
		if (t->last) {
			t->last->next = t->next;
		}
	}

	if (t == tail) // (t->next == NULL)
	{
		tail = t->last;
	}
	else
	{
		t->next->last = t->last;
	}

	t->last = NULL;
	t->next = NULL;

	count--;
}

void list::insert_before(void *w, void *a) // insert w before a
{
	list_entry *x = (list_entry*)w;
	list_entry *y = (list_entry*)a;

	x->next = y;
	x->last = y->last;

	if (y->last)
	{
		y->last->next = x;
	}

	y->last = x;

	if ((list_entry*)root == a)
	{
		root = (list_entry*)w;
	}
}

void list::insert_after(void *w, void *a) // insert w after a
{
	list_entry *x = (list_entry*)w;
	list_entry *y = (list_entry*)a;

	x->next = y->next;
	x->last = y;

	if (y->next)
	{
		y->next->last = x;
	}

	y->next = x;

	if ((list_entry*)tail == a)
	{
		tail = (list_entry*)w;
	}
}

void list::swap(void *b, void *a)
{
	list_entry *x = (list_entry*)b,
			*y = (list_entry*)a,
			*v = x->last,
			*w = x->next,
			*t = y->last,
			*u = y->next;

	// v <-> x <-> w
	// t <-> y <-> u

	v->next = y;
	y->last = v;
	t->next = x;
	x->last = t;
	w->last = y;
	y->next = w;
	u->last = x;
	x->next = u;
}

void list::swap_block(void *b, void *be, void *a, void *ae)
{
	list_entry *v = ((list_entry*)b)->last,
			*w = ((list_entry*)be)->next,
			*x = ((list_entry*)a)->last,
			*y = ((list_entry*)ae)->next,
			*q = (list_entry*)b,
			*r = (list_entry*)be,
			*t = (list_entry*)a,
			*u = (list_entry*)ae;

	// v <-> q <-> ... <-> r <-> w
	// x <-> t <-> ... <-> u <-> y
	y->last = r;
	r->next = y;
	w->last = u;
	u->next = w;

	v->next = t;
	t->last = v;
	x->next = q;
	q->last = x;
}

void list::insert_block(void *at, void *b, void *be) {
	list_entry *v = (list_entry*)b,
			*w = (list_entry*)be,
			*x = (list_entry*)at,
			*z = (list_entry*)x->last;

	// v <-> ... <-> w
	// z <-> x
	// z <-> v <-> ... <-> w <->x
	z->next = v;
	v->last = z;

	w->next = x;
	x->last = w;
}

void list::remove_block(void *b, void *be) {
	list_entry *v = (list_entry*)b,
			   *w = (list_entry*)be,
			   *x = (list_entry*)v->last,
			   *z = (list_entry*)w->next;

	z->last = x;
	x->next = z;
	
	v->last = NULL;
	w->next = NULL;
}

void* list::entry_by_index(void *start, size_t index)
{
	for (size_t current_index = 0; current_index < index; current_index++)
	{
		start = ((list_entry*)start)->next;
	}
  
	return start;
}
