#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include <utils/dlist.h>

typedef struct {
  int num;
  char *text;
  char buf[128];
} test_t;

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

  dlist_t *dlist = dlist_init(&t, test_t);
  assert_int_equal(dlist->len, 1);
  test_t *dt_ptr = dlist->current->el_ptr;
  assert_int_equal(dt_ptr->num, t.num);
  assert_string_equal(dt_ptr->buf, t.buf);
  assert_ptr_equal(dt_ptr->text, LOREM_IPSUM);
}

void test__init__add_several_times(void **state) {
  test_t t1 = {.buf = "TEST BUF", .num = 123, .text = LOREM_IPSUM};
  test_t t2 = {.buf = "TEST 1 BUF", .num = 11, .text = "RAND 1 TEXT"};
  test_t t3 = {.buf = "TEST 2 BUF", .num = 22, .text = "RAND 2 TEXT"};

  dlist_t *dlist = dlist_init(NULL, test_t);

  dlist_add(dlist, &t1, test_t, false);
  dlist_add(dlist, &t2, test_t, false);
  dlist_add(dlist, &t3, test_t, false);

  assert_int_equal(dlist->len, 3);

  test_t *dl_t1 = dlist->start->el_ptr;
  test_t *dl_t2 = dlist->start->next->el_ptr;
  test_t *dl_t3 = dlist->current->el_ptr;

  assert_memory_equal(dl_t1, &t1, sizeof(test_t));
  assert_memory_equal(dl_t2, &t2, sizeof(test_t));
  assert_memory_equal(dl_t3, &t3, sizeof(test_t));
}

void test__init__add_several_times_prepend(void **state) {
  test_t t1 = {.buf = "TEST BUF", .num = 123, .text = LOREM_IPSUM};
  test_t t2 = {.buf = "TEST 1 BUF", .num = 11, .text = "RAND 1 TEXT"};
  test_t t3 = {.buf = "TEST 2 BUF", .num = 22, .text = "RAND 2 TEXT"};

  dlist_t *dlist = dlist_init(NULL, test_t);

  dlist_add(dlist, &t1, test_t, true);
  dlist_add(dlist, &t2, test_t, true);
  dlist_add(dlist, &t3, test_t, true);

  assert_int_equal(dlist->len, 3);

  test_t *dl_t1 = dlist->current->el_ptr;
  test_t *dl_t2 = dlist->current->previous->el_ptr;
  test_t *dl_t3 = dlist->start->el_ptr;

  assert_memory_equal(dl_t1, &t1, sizeof(test_t));
  assert_memory_equal(dl_t2, &t2, sizeof(test_t));
  assert_memory_equal(dl_t3, &t3, sizeof(test_t));
}

void test__init__add_and_iterate(void **state) {
  test_t t1 = {.buf = "TEST BUF", .num = 123, .text = LOREM_IPSUM};
  test_t t2 = {.buf = "TEST 1 BUF", .num = 11, .text = "RAND 1 TEXT"};
  test_t t3 = {.buf = "TEST 2 BUF", .num = 22, .text = "RAND 2 TEXT"};

  dlist_t *dlist = dlist_init(NULL, test_t);

  dlist_add(dlist, &t1, test_t, false);
  dlist_add(dlist, &t2, test_t, false);
  dlist_add(dlist, &t3, test_t, false);

  test_t *dl_t3 = dlist_get_current(dlist);
  test_t *dl_t2 = dlist_it_prev(dlist);
  test_t *dl_t1 = dlist_it_prev(dlist);

  assert_memory_equal(dl_t1, &t1, sizeof(test_t));
  assert_memory_equal(dl_t2, &t2, sizeof(test_t));
  assert_memory_equal(dl_t3, &t3, sizeof(test_t));
}

int setup(void **state) { return 0; }
int tear_down(void **state) { return 0; }

int main(int argc, char **argv) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test__init),
      cmocka_unit_test(test__init__add_several_times),
      cmocka_unit_test(test__init__add_several_times_prepend),
      cmocka_unit_test(test__init__add_and_iterate),

  };

  return cmocka_run_group_tests(tests, setup, tear_down);
}