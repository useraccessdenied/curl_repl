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
#include <curl/curl.h>
#include "repl.h"

#define VERSION "0.1.0"

static void
print_usage(void)
{
  printf("Usage: curl_repl [options]\n");
  printf("\n");
  printf("Options:\n");
  printf("  -h, --help     show this help and exit\n");
  printf("  -v, --version  show version and exit\n");
  printf("\n");
  printf("Once running, type 'help' to see available commands.\n");
}

int
main(int argc, char *argv[])
{
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

  repl_run();

  curl_global_cleanup();
  return 0;
}
