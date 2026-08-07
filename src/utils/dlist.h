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
  dlist_node_t *work_pointer;
  size_t len;
} dlist_t;

/* Returns if a > b */
typedef bool dblist_sort_cb_t(void *a, void *b);
typedef bool dblist_add_cb_t(void *dst, void *src);
typedef bool dblist_rm_cb_t(void *el_ptr);

dlist_t *_dlist_init(void *el_ptr, int el_siz, dblist_add_cb_t);
void *dlist_get_current(dlist_t *dlist);
void *dlist_it_prev(dlist_t *dlist);
void _dlist_add(dlist_t *dlist, void *el_ptr, int el_siz,
                dblist_add_cb_t add_cb, bool prepend);
void *dlist_it_prev(dlist_t *dlist);
void *dlist_it_next(dlist_t *dlist);
void _dlist_add_sort(dlist_t *dlist, void *el_ptr, int el_siz,
                     dblist_sort_cb_t cb, dblist_add_cb_t add_cb);
int dlist_remove_by_ptr(dlist_t *dlist, void *el_ptr, dblist_rm_cb_t cb);
int dlist_clear_list(dlist_t *dlist, dblist_rm_cb_t cb);
dlist_node_t *dlist_wind_fwd(dlist_t *dlist, dlist_node_t *start, int count);
dlist_node_t *dlist_wind_bwd(dlist_t *dlist, dlist_node_t *start, int count);

#define dlist_init(el_ptr, el_def, add_cb)                                     \
  _dlist_init(el_ptr, sizeof(el_def), add_cb)
#define dlist_add(dlist, el_ptr, el_def, add_cb, prepend)                      \
  _dlist_add(dlist, el_ptr, sizeof(el_def), add_cb, prepend)
#define dlist_get_current(dlist) dlist->current->el_ptr
#define dlist_get_working_point(dlist) dlist->current->el_ptr
#define dlist_add_sort(dlist, el_ptr, el_typ, sort_cb, add_cb)                 \
  _dlist_add_sort(dlist, el_ptr, sizeof(el_typ), sort_cb, add_cb)
#define dlist_get_ptr(node_ptr) node_ptr == NULL ? NULL : node_ptr->el_ptr
#define dlist_set_wp(dlist, node_ptr) dlist->work_pointer = node_ptr
#define dlist_get_wp(dlist) dlist->work_pointer

#endif