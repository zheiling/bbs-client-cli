#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct dlist_el_t {
  struct dlist_el_t *next;
  struct dlist_el_t *previous;
  void *el_ptr;
} dlist_el_t;

typedef struct dlist_t {
  dlist_el_t *start;
  dlist_el_t *current;
  size_t len;
} dlist_t;

void dlist_add(dlist_t *dlist, void *el_ptr, int el_siz, bool prepend);

dlist_t *dlist_init(void *el_ptr, int el_siz) {
  dlist_t *dlist = malloc(sizeof(dlist_t));
  dlist->len = 0;
  dlist->current = NULL;
  dlist->start = NULL;

  if (el_ptr != NULL) {
    dlist_add(dlist, el_ptr, el_siz, false);
  }

  return dlist;
}

void dlist_add(dlist_t *dlist, void *el_ptr, int el_siz, bool prepend) {
  dlist_el_t *d_el = malloc(sizeof(dlist_el_t));
  d_el->el_ptr = malloc(el_siz);
  memcpy(d_el->el_ptr, el_ptr, el_siz);

  if (dlist->start == NULL) {
    d_el->previous = NULL;
    d_el->next = NULL;
    dlist->start = d_el;
    dlist->current = d_el;
  } else if (prepend) {
    d_el->previous = NULL;
    d_el->next = dlist->start;
    dlist->start = d_el;
  } else {
    d_el->next = NULL;
    dlist->current->next = d_el;
    dlist->current = d_el;
  }
  dlist->len++;
}

void *dlist_get_current(dlist_t *dlist) {
    return dlist->current->el_ptr;
}

void *dlist_it_prev(dlist_t *dlist) {
    dlist->current = dlist->current->previous;
    return dlist->current;
}