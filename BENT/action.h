#ifndef ACTION_H
#define ACTION_H

#include "list.h"

enum { // ARgumentType
	ART_IMM = 1,
	ART_RVA
};

enum { // ARgumentFlag
	ARF_DEREF = 1
};

typedef struct {
	int    type, flag;
	size_t data;
} Argument;

enum {
	ACT_ADD = 1,
	ACT_SUB,
	ACT_OR,
	ACT_XOR,
	ACT_AND
};

typedef struct {
	int       type;
	Argument *a1, *a2, *a3;
} Action;

class ActionList {
public:
	ActionList();
	~ActionList();

	list actions;
};

/*
ActionList *action_list_new();
Argument *CreateArg(int t, int f, size_t x);
Action *CreateAction(ActionList *al, int t, Argument *x, Argument *y);
void DumpAction(Action *a, char *buf, size_t bufSize);
void DumpActions(ActionList *al);
*/

#endif
