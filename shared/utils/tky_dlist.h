#ifndef DYNAMIC_LIST_H
#define DYNAMIC_LIST_H

#define DL_INIT_SIZE 50

#include<stddef.h>

typedef struct tky_dlist tky_dlist;

/*initialize a dynamic list and prepare it for use and optionally set the initial size with 'init_size' (default 50)*/
tky_dlist*   dlist_init(size_t* init_size);
/*appends 'data' to the end of the given list and automatically expands the list when neccessary*/
void         dlist_add(tky_dlist* l, void* data);
/*shifts all the elements one position to the right and places the element at the specified index*/
/*(The list will expand automatically if the index exceeds it's current capacity)*/
void         dlist_insert(tky_dlist* l, int index, void* data);
/*overwrites the element at 'index' with 'data'*/
void         dlist_set(tky_dlist* l, int index, void* data);
/*removes the element at the specified index by shifting all the list elements one position to the left*/
void         dlist_remove(tky_dlist* l, int index);
/*retrieves the data at the specified index*/
void*        dlist_get(tky_dlist* l, int index);

/*returns the amount of elements currently present in the list*/
/*NOTE: if an element is inserted at a position > l->size then size = index of inserted element*/
size_t       dlist_size(tky_dlist* l);
/*returns the total number of elements the list can hold before needing to expand*/
size_t       dlist_capacity(tky_dlist* l);

/*sets the value of list.size to match list.capacity, treating all empty slots as valid and addressable*/
void         dlist_match_capacity(tky_dlist* l);

void         dlist_free(tky_dlist* l);

#endif
