/*
 * command.c - command parsing and dispatch
 *
 * Copyright (C) 2026 Kapil Singaria
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <stdio.h>
#include <string.h>
#include "command.h"
#include "protocol.h"

static int g_quit = 0;

static int cmd_connect(const char *args);
static int cmd_help(const char *args);
static int cmd_quit(const char *args);
static int cmd_exit(const char *args);

static const struct command GLOBAL_COMMANDS[] = {
  {"connect", cmd_connect, "connect <url>  - connect to a server"},
  {"help",    cmd_help,    "help           - list available commands"},
  {"quit",    cmd_quit,    "quit           - exit the REPL"},
  {"exit",    cmd_exit,    "exit           - exit the REPL"},
  {NULL, NULL, NULL}
};

static const struct command *
find_in_table(const struct command *table, const char *name)
{
  int i;
  for(i = 0; table[i].name; i++) {
    if(strcmp(table[i].name, name) == 0)
      return &table[i];
  }
  return NULL;
}

/* Split line into a command token and the remainder (args).
 * Skips leading whitespace before both. */
static void
split_command(const char *line, char *token, int tokensize,
              const char **args_out)
{
  const char *p = line;
  int len = 0;

  while(*p == ' ' || *p == '\t')
    p++;
  while(*p && *p != ' ' && *p != '\t' && len < tokensize - 1)
    token[len++] = *p++;
  token[len] = '\0';
  while(*p == ' ' || *p == '\t')
    p++;
  *args_out = p;
}

static int
cmd_connect(const char *args)
{
  char scheme[16];
  const char *sep;
  const struct protocol *proto;
  int len;

  sep = strstr(args, "://");
  if(!sep) {
    fprintf(stderr, "invalid URL: missing '://'\n");
    return -1;
  }

  len = (int)(sep - args);
  if(len <= 0 || len >= (int)sizeof(scheme)) {
    fprintf(stderr, "invalid URL scheme\n");
    return -1;
  }

  memcpy(scheme, args, len);
  scheme[len] = '\0';

  proto = protocol_find(scheme);
  if(!proto) {
    fprintf(stderr, "unsupported scheme: %s\n", scheme);
    return -1;
  }

  /* disconnect any active protocol before connecting a new one */
  {
    const struct protocol *active = protocol_active();
    if(active)
      active->disconnect_fn();
  }

  if(proto->connect_fn(args) != 0)
    return -1;

  protocol_set_active(proto);
  return 0;
}

static int
cmd_help(const char *args)
{
  const struct protocol *proto;
  int i;
  (void)args;

  printf("Global commands:\n");
  for(i = 0; GLOBAL_COMMANDS[i].name; i++)
    printf("  %s\n", GLOBAL_COMMANDS[i].help_text);

  proto = protocol_active();
  if(proto) {
    printf("[%s commands]:\n", proto->name);
    for(i = 0; proto->commands[i].name; i++)
      printf("  %s\n", proto->commands[i].help_text);
  }

  return 0;
}

static int
cmd_quit(const char *args)
{
  (void)args;
  command_set_quit();
  return 0;
}

static int
cmd_exit(const char *args)
{
  (void)args;
  command_set_quit();
  return 0;
}

void
command_dispatch(const char *line)
{
  char token[64];
  const char *args;
  const struct command *cmd;
  const struct protocol *proto;

  split_command(line, token, sizeof(token), &args);

  if(!token[0])
    return;

  cmd = find_in_table(GLOBAL_COMMANDS, token);
  if(!cmd) {
    proto = protocol_active();
    if(proto)
      cmd = find_in_table(proto->commands, token);
  }

  if(!cmd) {
    fprintf(stderr, "unknown command: %s (type 'help' for help)\n",
            token);
    return;
  }

  cmd->handler(args);
}

void
command_set_quit(void)
{
  g_quit = 1;
}

int
command_should_quit(void)
{
  return g_quit;
}
