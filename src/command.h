/*
 * command.h - command table types and dispatch interface
 *
 * Copyright (C) 2026 Kapil Singaria
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef CURL_REPL_COMMAND_H
#define CURL_REPL_COMMAND_H

typedef int (*cmd_handler_fn)(const char *args);

struct command {
  const char     *name;
  cmd_handler_fn  handler;
  const char     *help_text;
};

void command_dispatch(const char *line);
void command_set_quit(void);
int  command_should_quit(void);
int  command_last_error(void);

#endif /* CURL_REPL_COMMAND_H */
