#ifndef DLIST_H
#define DLIST_H

#include <stdbool.h>
#include <stddef.h>

typedef struct dlist_node_t {
  struct dlist_node_t *next;
  struct dlist_node_t *previous;
  void *el_ptr;
} dlist_node_t;

typedef struct dlist_t {
  dlist_node_t *start;
  dlist_node_t *current;
  dlist_node_t *pointer;
  size_t len;
} dlist_t;

dlist_t *_dlist_init(void *el_ptr, int el_siz);

/* Returns if a > b */
typedef bool dblist_sort_cb_t(void *a, void *b);

typedef bool dblist_rm_cb_t(void *el_ptr);

void *dlist_get_current(dlist_t *dlist);
void *dlist_it_prev(dlist_t *dlist);
void _dlist_add(dlist_t *dlist, void *el_ptr, int el_siz, bool prepend);
void *dlist_it_prev(dlist_t *dlist);
void *dlist_it_next(dlist_t *dlist);
void _dlist_insert_sort(dlist_t *dlist, void *el_ptr, int el_siz,
                        dblist_sort_cb_t *cb);

#define dlist_init(el_ptr, el_def) _dlist_init(el_ptr, sizeof(el_def))
#define dlist_add(dlist, el_ptr, el_def, prepend)                              \
  _dlist_add(dlist, el_ptr, sizeof(el_def), prepend)
#define dlist_get_current(dlist) dlist->current->el_ptr
#define dlist_insert_sort(dlist, el_ptr, el_typ, cb)                           \
  _dlist_insert_sort(dlist, el_ptr, sizeof(el_typ), cb)

#endif