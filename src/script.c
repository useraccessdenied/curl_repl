/*
 * script.c - script file execution
 *
 * Copyright (C) 2026 Kapil Singaria
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <stdio.h>
#include <string.h>
#include "script.h"
#include "command.h"

#define LINE_MAX 1024

int
script_run(const char *path, int keep_going)
{
  FILE *fp;
  char line[LINE_MAX];
  int lineno = 0;
  int errors = 0;
  size_t len;

  fp = fopen(path, "r");
  if(!fp) {
    fprintf(stderr, "script: cannot open '%s'\n", path);
    return -1;
  }

  while(fgets(line, sizeof(line), fp)) {
    lineno++;

    /* strip trailing newline (and optional \r) */
    len = strlen(line);
    if(len > 0 && line[len - 1] == '\n')
      line[--len] = '\0';
    if(len > 0 && line[len - 1] == '\r')
      line[--len] = '\0';

    /* skip blank lines and comments */
    if(len == 0 || line[0] == '#')
      continue;

    command_dispatch(line);

    if(command_should_quit())
      break;

    if(command_last_error() != 0) {
      fprintf(stderr, "script: error at line %d\n", lineno);
      errors++;
      if(!keep_going)
        break;
    }
  }

  fclose(fp);
  return errors > 0 ? 1 : 0;
}
