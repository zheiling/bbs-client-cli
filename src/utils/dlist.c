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

void _dlist_insert(dlist_t *dlist, dlist_el_t *new_el, dlist_el_t *prev_el) {
  if (prev_el != NULL) {
    new_el->next = prev_el->next;
    prev_el->next->previous = new_el;
    new_el->previous = prev_el;
    prev_el->next = new_el;
    if (new_el->next == NULL) {
      dlist->current = new_el;
      dlist->pointer = new_el;
    }
  } else {
    new_el->next = dlist->current;
    new_el->previous = NULL;
    dlist->start = new_el;
    dlist->current->previous = new_el;
  }
  dlist->len++;
}

void _dlist_insert_end(dlist_t *dlist, dlist_el_t *new_el) {
  dlist->current->next = new_el;
  new_el->previous = dlist->current;
  new_el->next = NULL;
  dlist->current = new_el;
  dlist->pointer = new_el;
  dlist->len++;
}

void _dlist_insert_sort(dlist_t *dlist, void *el_ptr, int el_siz,
                        dblist_cb_t *cb) {
  dlist_el_t *d_el = malloc(sizeof(dlist_el_t));
  d_el->el_ptr = malloc(el_siz);
  memcpy(d_el->el_ptr, el_ptr, el_siz);

  dlist_el_t *dlist_item = dlist->start;
  bool res = true;
  if (dlist->start == NULL) {
    dlist->start = d_el;
    dlist->current = d_el;
    d_el->next = NULL;
    d_el->previous = NULL;
    dlist->pointer = d_el;
    dlist->len++;
    return;
  }
  for (int i = 0; i < dlist->len; i++) {
    res = cb(el_ptr, dlist_item->el_ptr);
    if (!res) {
      dlist_item = dlist_item->previous;
      _dlist_insert(dlist, d_el, dlist_item);
      break;
    }
  }
  if (res) _dlist_insert_end(dlist, d_el);
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
  } else {
    return NULL;
  }
}