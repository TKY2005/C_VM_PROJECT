#include "./llist.h"
#include<stdio.h>

#include<stdlib.h>

lnode* lnode_create(void* data) {
	lnode* n = (lnode*) calloc(1, sizeof(lnode));
	n->data = data;
	return n;
}

llist* llist_create() {
	llist* l = (llist*) malloc(sizeof(llist));
	l->head = l->tail = NULL;
	l->size = 0;
	
	return l;
}

void llist_push_head(llist* l, void* data) {

	lnode* new_head = lnode_create(data);
	if (l->size == 0) l->tail = l->head = new_head;

	else {
		new_head->next = l->head;
		l->head = new_head;
	}
	l->size++;
}

void* llist_get_head(llist* l) {
	if (l->size == 0) return NULL;
	else {
		return l->head->data;
	}
}

void llist_push_back(llist* l, void* data) {
	lnode* new_tail = lnode_create(data);

	if (l->size == 0) l->head = l->tail = new_tail;
	else {
		l->tail->next = new_tail;
		l->tail = new_tail;
	}
	l->size++;
}

void* llist_get_tail(llist* l) {
	if (l->size == 0) return NULL;

	else {
		return l->tail->data;
	}
}

void llist_display(llist* l) {
	if (l->size == 0) return;
	lnode* p = l->head;
	while (p != NULL) {
		printf("0x%p ", p->data);
		p = p->next;
	}
	printf("\n");
}

void* llist_get(llist* l, int index) {

	if (index >= l->size) return NULL;
	else {
		int counter = 0;
		lnode* p = l->head;
		while (counter < index) {
			if (p == NULL) return NULL;
			p = p->next;
			counter++;
		}
		return p->data;
	}
	return NULL;
}

void llist_destroy(llist* l) {
	lnode* p = l->head;
	lnode* next;
	while (p != NULL) {
		next = p->next;
		if (next) free(p);
		p = next;
	}
	if(next) free(next);
	free(l);
}
void llist_destroy_except_nodes(llist* l) {
	free(l);
}