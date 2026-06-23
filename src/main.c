/*
 * main.c - curl_repl entry point
 *
 * Copyright (C) 2026 Kapil Singaria
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <curl/curl.h>
#include "repl.h"
#include "command.h"
#include "script.h"

#define VERSION "0.1.0"

static void
sigint_handler(int sig)
{
  (void)sig;
  command_set_quit();
}

static void
print_usage(void)
{
  printf("Usage: curl_repl [options]\n");
  printf("\n");
  printf("Options:\n");
  printf("  -h, --help        show this help and exit\n");
  printf("  -v, --version     show version and exit\n");
  printf("  -f <file>         run commands from a script file\n");
  printf("  -k, --keep-going  continue past errors in script mode\n");
  printf("\n");
  printf("Without -f, runs interactively. Type 'help' inside for "
         "commands.\n");
}

int
main(int argc, char *argv[])
{
  const char *script_file = NULL;
  int keep_going = 0;
  CURLcode res;
  int i;

  for(i = 1; i < argc; i++) {
    if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      print_usage();
      return 0;
    }
    if(strcmp(argv[i], "-v") == 0 ||
       strcmp(argv[i], "--version") == 0) {
      printf("curl_repl " VERSION "\n");
      return 0;
    }
    if(strcmp(argv[i], "-k") == 0 ||
       strcmp(argv[i], "--keep-going") == 0) {
      keep_going = 1;
      continue;
    }
    if(strcmp(argv[i], "-f") == 0) {
      if(i + 1 >= argc) {
        fprintf(stderr, "-f requires a filename\n");
        return 1;
      }
      script_file = argv[++i];
      continue;
    }
    fprintf(stderr, "unknown option: %s\n", argv[i]);
    print_usage();
    return 1;
  }

  res = curl_global_init(CURL_GLOBAL_DEFAULT);
  if(res != CURLE_OK) {
    fprintf(stderr, "curl_global_init failed: %s\n",
            curl_easy_strerror(res));
    return 1;
  }

  if(script_file) {
    i = script_run(script_file, keep_going);
  }
  else {
    signal(SIGINT, sigint_handler);
    repl_run();
    i = 0;
  }

  curl_global_cleanup();
  return i;
}
