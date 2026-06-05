/*
 * ws.h - WebSocket protocol handler interface
 *
 * Copyright (C) 2026 Kapil Singaria
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef CURL_REPL_WS_H
#define CURL_REPL_WS_H

#include <curl/curl.h>
#include "command.h"

int           ws_connect(const char *url);
int           ws_send(const char *message);
int           ws_ping(void);
void          ws_disconnect(void);
curl_socket_t ws_get_socket(void);
int           ws_recv_and_print(void);

extern const struct command ws_commands[];

#endif /* CURL_REPL_WS_H */
