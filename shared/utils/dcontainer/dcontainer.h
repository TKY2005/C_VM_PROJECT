#ifndef DCONTAINER_H
#define DCONTAINER_H

typedef struct {
	int capacity;
	void** data;
} dcontainer;

dcontainer mkcontainer();

int dcontainer_capacity(dcontainer d);
void dcontainer_free(dcontainer* d);

#endif