/*
 * script.h - script file execution interface
 *
 * Copyright (C) 2026 Kapil Singaria
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef CURL_REPL_SCRIPT_H
#define CURL_REPL_SCRIPT_H

int script_run(const char *path, int keep_going);

#endif /* CURL_REPL_SCRIPT_H */
