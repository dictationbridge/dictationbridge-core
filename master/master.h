/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <windows.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef void (WINAPI *TTextInsertedCallback)(HWND hwnd, DWORD startPosition, LPCWSTR text);

void WINAPI DBMaster_SetTextInsertedCallback(TTextInsertedCallback callback);
BOOL WINAPI DBMaster_Start(void);
void WINAPI DBMaster_Stop(void);

#ifdef __cplusplus
}
#endif
