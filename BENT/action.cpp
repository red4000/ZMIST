#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "action.h"

ActionList::ActionList() {
	actions.entry_size = sizeof(Action);
}

ActionList::~ActionList() {

}

Argument *CreateArg(int t, int f, size_t x) {
	Argument *res = (Argument*)calloc(1, sizeof(Argument));
	res->type = t;
	res->flag = f;
	res->data = x;
	return res;
}

Action *CreateAction(ActionList *al, int t, Argument *x, Argument *y) {
	//Action *res = (Action*)calloc(1, sizeof(Action));
	Action *res = (Action*)al->actions.alloc();
	res->type = t;
	res->a1   = x;
	res->a2   = y;
	return res;
}

/*
void DumpArgument(Argument *a, char *buf, size_t bufSize) {
	sprintf(buf, "AT:%i AF:%i AD:%i", a->type, a->flag, a->data);
}

void DumpAction(Action *a, char *buf, size_t bufSize) {
	sprintf(buf, "A  T:%i: ", a->type);
	char aBuf[512];
	if (a->a1) {
		DumpArgument(a->a1, aBuf, sizeof(aBuf));
		strcat(buf, aBuf);
	}
	if (a->a2) {
		strcat(buf, ", ");
		DumpArgument(a->a2, aBuf, sizeof(aBuf));
		strcat(buf, aBuf);
	}
	strcat(buf, "\n");
}
 
void DumpActions(ActionList *al) {
	Action *a = (Action*)al->actions.p;
	char    aBuf[512];
	
	for (int i = 0; i < al->actions.usedCapacity; i++) {
		DumpAction(&a[i], aBuf, sizeof(aBuf));
		printf(aBuf);
	}
}
*/
