#include <unistd.h>
#include <string.h>
#include <stdio.h>

#define BYTES_IN_GB 1073741824
#define BYTES_IN_MB 1048576
#define BYTES_IN_KB 1024

#define PRINT_SIZE(text, f_arg, s_arg, unit)                                   \
  s_arg /= 100;                                                                \
  if (s_arg > 0) {                                                             \
    sprintf(text, "%zu,%zu%s", f_arg, s_arg, unit);                            \
  } else {                                                                     \
    sprintf(text, "%zu%s", f_arg, unit);                                       \
  }

void size_to_text(size_t size, char *text) {
  size_t kb = 0;
  size_t mb = 0;
  size_t gb = 0;

  gb = size / BYTES_IN_GB;
  mb = size / BYTES_IN_MB;
  kb = size / BYTES_IN_KB;
  if (gb > 0) {
    size = size % BYTES_IN_GB;
    mb = size / BYTES_IN_MB;
    PRINT_SIZE(text, gb, mb, "GB");
  } else if (mb > 0) {
    size = size % BYTES_IN_MB;
    kb = size / BYTES_IN_KB;
    PRINT_SIZE(text, mb, kb, "MB");
  } else if (kb > 0) {
    size = size % BYTES_IN_KB;
    PRINT_SIZE(text, kb, size, "KB");
  } else {
    sprintf(text, "%zu B", size);
  }
}