#ifndef DLIST_H
#define DLIST_H

#include <stdbool.h>
#include <stddef.h>

typedef struct dlist_el_t {
  struct dlist_el_t *next;
  struct dlist_el_t *previous;
  void *el_ptr;
} dlist_el_t;

typedef struct dlist_t {
  dlist_el_t *start;
  dlist_el_t *current;
  dlist_el_t *pointer;
  size_t len;
} dlist_t;

dlist_t *_dlist_init(void *el_ptr, int el_siz);
void *dlist_get_current(dlist_t *dlist);
void *dlist_it_prev(dlist_t *dlist);
void _dlist_add(dlist_t *dlist, void *el_ptr, int el_siz, bool prepend);
void *dlist_it_prev(dlist_t *dlist);
void *dlist_it_next(dlist_t *dlist);

#define dlist_init(el_ptr, el_def) _dlist_init(el_ptr, sizeof(el_def))
#define dlist_add(dlist, el_ptr, el_def, prepend) _dlist_add(dlist, el_ptr, sizeof(el_def), prepend)
#define dlist_get_current(dlist) dlist->current->el_ptr

#endif