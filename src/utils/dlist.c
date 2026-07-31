#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "dlist.h"

/* TODO: Test for dlist_remove_by_ptr */

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

void _dlist_insert(dlist_t *dlist, dlist_node_t *new_el, dlist_node_t *prev_node) {
  if (prev_node != NULL) {
    new_el->next = prev_node->next;
    prev_node->next->previous = new_el;
    new_el->previous = prev_node;
    prev_node->next = new_el;
    if (new_el->next == NULL) {
      dlist->current = new_el;
      dlist->pointer = new_el;
    }
  } else {
    new_el->next = dlist->start;
    new_el->previous = NULL;
    dlist->start->previous = new_el;
    dlist->start = new_el;
  }
  dlist->len++;
}

void _dlist_insert_end(dlist_t *dlist, dlist_node_t *new_node) {
  dlist->current->next = new_node;
  new_node->previous = dlist->current;
  new_node->next = NULL;
  dlist->current = new_node;
  dlist->pointer = new_node;
  dlist->len++;
}

void _dlist_insert_sort(dlist_t *dlist, void *el_ptr, int el_siz,
                        dblist_sort_cb_t *cb) {
  dlist_node_t *node = malloc(sizeof(dlist_node_t));
  node->el_ptr = malloc(el_siz);
  memcpy(node->el_ptr, el_ptr, el_siz);

  dlist_node_t *dlist_item = dlist->start;
  bool res = true;
  if (dlist->start == NULL) {
    dlist->start = node;
    dlist->current = node;
    node->next = NULL;
    node->previous = NULL;
    dlist->pointer = node;
    dlist->len++;
    return;
  }

  for (int i = 0; i < dlist->len; i++) {
    res = cb(el_ptr, dlist_item->el_ptr);
    if (!res)
      break;
    dlist_item = dlist_item->next;
  }

  if (!res) {
    dlist_item = dlist_item->previous;
    _dlist_insert(dlist, node, dlist_item);
  } else {
    _dlist_insert_end(dlist, node);
  }
}

void _dlist_add(dlist_t *dlist, void *el_ptr, int el_siz, bool prepend) {
  dlist_node_t *node = malloc(sizeof(dlist_node_t));
  node->el_ptr = malloc(el_siz);
  memcpy(node->el_ptr, el_ptr, el_siz);

  if (dlist->start == NULL) {
    node->previous = NULL;
    node->next = NULL;
    dlist->start = node;
    dlist->current = node;
    dlist->pointer = node;
  } else if (prepend) {
    node->previous = NULL;
    node->next = dlist->start;
    dlist->start->previous = node;
    dlist->start = node;
  } else {
    node->previous = dlist->current;
    node->next = NULL;
    dlist->current->next = node;
    dlist->current = node;
    dlist->pointer = node;
  }
  dlist->len++;
}

dlist_node_t *find_node_by_ptr(dlist_t *dlist, void *el_ptr) {
  dlist_node_t *el = dlist->start;
  do {
    if (el->el_ptr == el_ptr) break;
  } while ((el = el->next) != NULL);
  if (el->el_ptr == el_ptr) return el_ptr;
  return NULL;
}

int dlist_remove_by_ptr(dlist_t *dlist, void *el_ptr, dblist_rm_cb_t cb) {
  dlist_node_t *node = find_node_by_ptr(dlist, el_ptr);
  if (node == NULL) return -1;
  cb(node->el_ptr);
  node->next->previous = node->previous;
  node->previous->next = node->next;
  free(node);
  return 1;
}

void *dlist_it_prev(dlist_t *dlist) {
  dlist_node_t *cur_ptr = dlist->pointer;
  if (dlist->pointer->previous != NULL) {
    dlist->pointer = dlist->pointer->previous;
  }
  return cur_ptr->el_ptr;
}

void *dlist_it_next(dlist_t *dlist) {
  dlist_node_t *cur_ptr = dlist->pointer;
  if (dlist->pointer->next != NULL) {
    dlist->pointer = dlist->pointer->next;
  }
  return cur_ptr->el_ptr;
}