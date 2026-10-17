#ifndef LLIST_H
#define LLIST_H

#include<stddef.h>

typedef struct tky_llist tky_llist;
typedef struct tky_lnode tky_lnode;

tky_lnode*  lnode_create(void* data);

tky_lnode*  lnode_next(tky_lnode* n);
tky_lnode*  lnode_prev(tky_lnode* n);
void*       lnode_data(tky_lnode* n);

void        lnode_free(tky_lnode* n);

tky_llist*  llist_init();

tky_lnode*  llist_get_head(tky_llist* l);
tky_lnode*  llist_get_tail(tky_llist* l);

void        llist_push_back(tky_llist *l, void *data);
void        llist_push_front(tky_llist *l, void *data);
void        llist_remove_back(tky_llist *l);
void        llist_remove_front(tky_llist *l);
void*       llist_pop_front(tky_llist* l);
void*       llist_pop_back(tky_llist* l);
void*       llist_get(tky_llist* l, int index);
void        llist_remove_idx(tky_llist* l, int index);

void        llist_destroy(tky_llist* l);

void        llist_display(tky_llist* l);

int         llist_empty(tky_llist *l);
size_t      llist_size(tky_llist* l);

#endif
