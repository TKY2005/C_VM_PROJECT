#ifndef LLIST_H
#define LLIST_H

typedef struct lnode {
	void* data;
	struct lnode* next;
} lnode;

lnode* lnode_create(void* data);

typedef struct {
	struct lnode* head;
	struct lnode* tail;
	int size;
} llist;

llist* llist_create();
void llist_push_head(llist* l, void* data);
void* llist_get_head(llist* l);

void llist_push_back(llist* l, void* data);
void* llist_get_tail(llist* l);

void* llist_get(llist* l, int index);

void llist_display(llist* l);

void llist_destroy(llist* l);
void llist_destroy_except_nodes(llist* l);
#endif