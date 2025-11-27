#include "file_processor.h"
#include "main.h"
#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int process_client_command(char *line, int l_len, query_args_t *q_args) {
  /* FILE LIST */
  if (!strncmp(line, "file list", sizeof("file list") - 1)) {
    write(q_args->sd, "file list", sizeof("file list") - 1);
    q_args->state = STATE_FILE_LIST;
    return 0;
  }

  /* FILE UPLOAD */
  if (!strncmp(line, "file upload", sizeof("file upload") - 1)) {
    if (!file_upload_request(line, q_args)) {
      q_args->state = STATE_UPLOAD_REQUESTED;
    } else {
      print_prompt(q_args->params);
      q_args->state = WAIT_CLIENT;
    }
    return 0;
  }

  /* FILE DOWNLOAD */
  if (!strncmp(line, "file download", sizeof("file download") - 1)) {
    return 0;
  }

  /* EXIT */
  if (!strncmp(line, "exit", sizeof("exit") - 1)) {
    close(q_args->sd);
    printf("Bye!\n");
    exit(0);
  }

  return 0;
}

void print_prompt(params_t *params) {
  printf("%s> ", params->uname);
  fflush(stdout);
}