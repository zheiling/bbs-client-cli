#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "dlist.h"

dlist_t *_dlist_init(void *el_ptr, int el_siz) {
  dlist_t *dlist = malloc(sizeof(dlist_t));
  dlist->len = 0;
  dlist->current = NULL;
  dlist->start = NULL;

  if (el_ptr != NULL) {
    _dlist_add(dlist, el_ptr, el_siz, false);
  }

  return dlist;
}

void _dlist_add(dlist_t *dlist, void *el_ptr, int el_siz, bool prepend) {
  dlist_el_t *d_el = malloc(sizeof(dlist_el_t));
  d_el->el_ptr = malloc(el_siz);
  memcpy(d_el->el_ptr, el_ptr, el_siz);

  if (dlist->start == NULL) {
    d_el->previous = NULL;
    d_el->next = NULL;
    dlist->start = d_el;
    dlist->current = d_el;
    dlist->pointer = d_el;
  } else if (prepend) {
    d_el->previous = NULL;
    d_el->next = dlist->start;
    dlist->start->previous = d_el;
    dlist->start = d_el;
  } else {
    d_el->previous = dlist->current;
    d_el->next = NULL;
    dlist->current->next = d_el;
    dlist->current = d_el;
    dlist->pointer = d_el;
  }
  dlist->len++;
}

void *dlist_it_prev(dlist_t *dlist) {
  if (dlist->pointer->previous != NULL) {
    dlist->pointer = dlist->pointer->previous;
    return dlist->pointer->el_ptr;
  } else {
    return NULL;
  }
}

void *dlist_it_next(dlist_t *dlist) {
  if (dlist->pointer->next != NULL) {
    dlist->pointer = dlist->pointer->next;
    return dlist->pointer->el_ptr;
  } else  {
    return NULL;
  }
}