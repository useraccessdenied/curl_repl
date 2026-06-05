/*
 * protocol.h - protocol registry interface
 *
 * Copyright (C) 2026 Kapil Singaria
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef CURL_REPL_PROTOCOL_H
#define CURL_REPL_PROTOCOL_H

#include <curl/curl.h>
#include "command.h"

struct protocol {
  const char           *scheme;
  const char           *name;
  const struct command *commands;
  int                 (*connect_fn)(const char *url);
  void                (*disconnect_fn)(void);
  curl_socket_t       (*get_socket_fn)(void);
  int                 (*recv_fn)(void);
};

const struct protocol *protocol_find(const char *scheme);
const struct protocol *protocol_active(void);
void                   protocol_set_active(const struct protocol *p);

#endif /* CURL_REPL_PROTOCOL_H */
