/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <windows.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef void (WINAPI *TTextInsertedCallback)(HWND hwnd, LONG startPosition, LPCWSTR text);
typedef void (WINAPI *TTextDeletedCallback)(HWND hwnd, LONG startPosition, LPCWSTR text);
typedef void (WINAPI *TCommandCallback)(LPCSTR command);
typedef void (WINAPI *TDebugLogCallback)(LPCSTR message);

void WINAPI DBMaster_SetTextInsertedCallback(TTextInsertedCallback callback);
void WINAPI DBMaster_SetTextDeletedCallback(TTextDeletedCallback callback);
void WINAPI DBMaster_SetCommandCallback(TCommandCallback callback);
void WINAPI DBMaster_SetDebugLogCallback(TDebugLogCallback callback);
BOOL WINAPI DBMaster_Start(void);
void WINAPI DBMaster_Stop(void);

#ifdef __cplusplus
}
#endif
