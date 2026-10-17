#ifndef TKY_NTREE
#define TKY_NTREE

typedef struct tky_ntnode tky_ntnode;

tky_ntnode*		ntnode_create(void* data);
void			ntnode_destroy(tky_ntnode* n);

typedef struct tky_ntree tky_ntree;

tky_ntree*		ntree_init();
void			ntree_add_child(void* data);
void			ntree_add_sibling(void* data);

void			ntree_destroy(tky_ntree* t);


#endif
