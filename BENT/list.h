#ifndef LIST_H
#define LIST_H

// available only for member functions
/*
#ifndef ForEach
#define ForEach(entry_s, var)  \
   for (var = (entry_s *)root; \
		var != NULL;           \
		var = (entry_s *)var->next)
#endif
*/

#define ForEachInList(list, entry_s, var) \
   for (var = (entry_s *)list.root;       \
		var != NULL;                      \
		var = (entry_s *)var->next)

#define ForEachInPList(list, entry_s, var) \
   for (var = (entry_s *)list->root;       \
		var != NULL;                       \
		var = (entry_s *)var->next)

struct list_entry {
	list_entry *last;
	list_entry *next;
};

class list {
public:
	size_t      count;      // # of entries
	size_t      entry_size; // size of entry
	list_entry *root;       // root entry
	list_entry *tail;       // tail entry

	void (__cdecl *on_free)(void *entry);
	list();
	list(size_t entry_size_0);
	~list();
	void  empty(); // empty list
	void *alloc();
	void  attach(void *entry);
	void  detach(void *entry);
	void  insert_before(void *w, void *a); // insert w before a
	void  insert_after(void *w, void *a);  // insert w after a
	void  swap(void *w, void *a);          // swap w with a
	void  swap_block(void *b, void *be, void *a, void *ae); // swap block b..be with block a..ae
	void  insert_block(void *at, void *b, void *be); // insert block b..be before at
	void  remove_block(void *b, void *be);  // remove b..be
	void *entry_by_index(void *start, size_t index); // get entry at index i from start
};

#endif
