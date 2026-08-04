#include <cmocka.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <utils/dlist.h>

typedef struct {
  int num;
  char *text;
  char buf[128];
} test_t;

bool dblist_add_cb(void *dst, void *src) {
  // test_t *dst = _dst;
  // test_t *src = _src;
  memcpy(dst, src, sizeof(test_t));
  return true;
}

#define tlist_add(dlist, item, prepend)                                        \
  dlist_add(dlist, item, test_t, dblist_add_cb, prepend);

#define LOREM_IPSUM                                                            \
  "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "   \
  "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim "      \
  "veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea "   \
  "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate "    \
  "velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint "         \
  "occaecat cupidatat non proident, sunt in culpa qui officia deserunt "       \
  "mollit anim id est laborum."

void test__init(void **state) {
  test_t t = {.buf = "TEST BUF", .num = 123, .text = LOREM_IPSUM};

  dlist_t *dlist = dlist_init(&t, test_t, dblist_add_cb);
  assert_int_equal(dlist->len, 1);
  test_t *dt_ptr = dlist->current->el_ptr;
  assert_int_equal(dt_ptr->num, t.num);
  assert_string_equal(dt_ptr->buf, t.buf);
  assert_ptr_equal(dt_ptr->text, LOREM_IPSUM);
}

void test__add_several_times(void **state) {
  test_t t1 = {.buf = "TEST BUF", .num = 123, .text = LOREM_IPSUM};
  test_t t2 = {.buf = "TEST 1 BUF", .num = 11, .text = "RAND 1 TEXT"};
  test_t t3 = {.buf = "TEST 2 BUF", .num = 22, .text = "RAND 2 TEXT"};

  dlist_t *dlist = dlist_init(NULL, test_t, NULL);

  dlist_add(dlist, &t1, test_t, dblist_add_cb, false);
  dlist_add(dlist, &t2, test_t, dblist_add_cb, false);
  dlist_add(dlist, &t3, test_t, dblist_add_cb, false);

  assert_int_equal(dlist->len, 3);

  test_t *dl_t1 = dlist->start->el_ptr;
  test_t *dl_t2 = dlist->start->next->el_ptr;
  test_t *dl_t3 = dlist->current->el_ptr;

  assert_memory_equal(dl_t1, &t1, sizeof(test_t));
  assert_memory_equal(dl_t2, &t2, sizeof(test_t));
  assert_memory_equal(dl_t3, &t3, sizeof(test_t));
}

void test__add_several_times_prepend(void **state) {
  test_t t1 = {.buf = "TEST BUF", .num = 123, .text = LOREM_IPSUM};
  test_t t2 = {.buf = "TEST 1 BUF", .num = 11, .text = "RAND 1 TEXT"};
  test_t t3 = {.buf = "TEST 2 BUF", .num = 22, .text = "RAND 2 TEXT"};

  dlist_t *dlist = dlist_init(NULL, test_t, NULL);

  dlist_add(dlist, &t1, test_t, dblist_add_cb, true);
  dlist_add(dlist, &t2, test_t, dblist_add_cb, true);
  dlist_add(dlist, &t3, test_t, dblist_add_cb, true);

  assert_int_equal(dlist->len, 3);

  test_t *dl_t1 = dlist->current->el_ptr;
  test_t *dl_t2 = dlist->current->previous->el_ptr;
  test_t *dl_t3 = dlist->start->el_ptr;

  assert_memory_equal(dl_t1, &t1, sizeof(test_t));
  assert_memory_equal(dl_t2, &t2, sizeof(test_t));
  assert_memory_equal(dl_t3, &t3, sizeof(test_t));
}

void test__add_and_iterate(void **state) {
  test_t t1 = {.buf = "TEST BUF", .num = 123, .text = LOREM_IPSUM};
  test_t t2 = {.buf = "TEST 1 BUF", .num = 11, .text = "RAND 1 TEXT"};
  test_t t3 = {.buf = "TEST 2 BUF", .num = 22, .text = "RAND 2 TEXT"};

  dlist_t *dlist = dlist_init(NULL, test_t, NULL);

  dlist_add(dlist, &t1, test_t, dblist_add_cb, false);
  dlist_add(dlist, &t2, test_t, dblist_add_cb, false);
  dlist_add(dlist, &t3, test_t, dblist_add_cb, false);

  test_t *dl_t3 = dlist_it_prev(dlist);
  test_t *dl_t2 = dlist_it_prev(dlist);
  test_t *dl_t1 = dlist_it_prev(dlist);

  assert_memory_equal(dl_t1, &t1, sizeof(test_t));
  assert_memory_equal(dl_t2, &t2, sizeof(test_t));
  assert_memory_equal(dl_t3, &t3, sizeof(test_t));
}

bool _sort(void *_a, void *_b) {
  test_t *a = _a;
  test_t *b = _b;

  return a->num > b->num;
}

#define T_BUF "TEST BUF "
#define T_TEXT "RAND TEXT "
#define T_IT_NUM 120

void test__insert_sort(void **state) {
  srand(time(NULL));

  test_t t = {.text = T_TEXT};
  dlist_t *dlist = dlist_init(NULL, test_t, NULL);

  for (int i = 0; i < T_IT_NUM; i++) {
    sprintf(t.buf, "%s %d", T_BUF, i);
    t.num = rand();
    dlist_add_sort(dlist, &t, test_t, _sort, dblist_add_cb);
  }

  test_t *t1, *t2;

  t2 = dlist_it_prev(dlist);
  t1 = dlist_it_prev(dlist);

  assert_int_in_range(t2->num, t1->num, INT_MAX);

  for (int i = T_IT_NUM - 2; i > 0; i--) {
    t2 = t1;
    t1 = dlist_it_prev(dlist);
    assert_int_in_range(t2->num, t1->num, INT_MAX);
  }
}

int delete_times = 0;
test_t *b_ptr = NULL;

bool delete_cb(void *el_ptr) {
  assert_int_equal(b_ptr, el_ptr);
  delete_times++;
  return 1;
}

void test__remove_by_ptr(void **state) {
  test_t ta = {.buf = "TEST A", .num = 123, .text = "TEXT A"};
  test_t tb = {.buf = "TEST B", .num = 456, .text = "TEXT B"};
  test_t tc = {.buf = "TEST C", .num = 789, .text = "TEXT C"};
  dlist_t *dlist = dlist_init(NULL, int, NULL);

  tlist_add(dlist, &ta, false);
  tlist_add(dlist, &tb, false);
  tlist_add(dlist, &tc, false);

  b_ptr = dlist->start->next->el_ptr;
  assert_int_equal(tb.num, 456);

  dlist_remove_by_ptr(dlist, b_ptr, delete_cb);
  assert_ptr_equal(dlist->start->next, dlist->current);
  assert_ptr_equal(dlist->current->previous, dlist->start);
  assert_int_equal(dlist->len, 2);
  assert_int_equal(delete_times, 1);
  test_t *a_ptr = dlist->start->el_ptr;
  test_t *c_ptr = dlist->start->next->el_ptr;
  assert_int_equal(a_ptr->num, 123);
  assert_int_equal(c_ptr->num, 789);
}

int clean_db_call_times = 0;

bool clean_db(void *ptr) {
  free(ptr);
  clean_db_call_times++;
  return 1;
}

void test__clear_list(void **state) {
  test_t ta = {.buf = "TEST A", .num = 123, .text = "TEXT A"};
  test_t tb = {.buf = "TEST B", .num = 456, .text = "TEXT B"};
  test_t tc = {.buf = "TEST C", .num = 789, .text = "TEXT C"};

  dlist_t *dlist = dlist_init(NULL, int, NULL);

  tlist_add(dlist, &ta, false);
  tlist_add(dlist, &tb, false);
  tlist_add(dlist, &tc, false);

  assert_int_equal(dlist->len, 3);
  dlist_clear_list(dlist, clean_db);
  assert_int_equal(dlist->len, 0);
  assert_ptr_equal(dlist->current, NULL);
  assert_ptr_equal(dlist->start, NULL);
  assert_int_equal(clean_db_call_times, 3);
  clean_db_call_times = 0;
}

int setup(void **state) { return 0; }
int tear_down(void **state) { return 0; }

int main(int argc, char **argv) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test__init),
      cmocka_unit_test(test__add_several_times),
      cmocka_unit_test(test__add_several_times_prepend),
      cmocka_unit_test(test__add_and_iterate),
      cmocka_unit_test(test__insert_sort),
      cmocka_unit_test(test__remove_by_ptr),
      cmocka_unit_test(test__clear_list),
  };

  return cmocka_run_group_tests(tests, setup, tear_down);
}