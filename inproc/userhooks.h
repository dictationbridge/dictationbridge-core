/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <windows.h>

typedef LRESULT (WINAPI *TSendMessageW)(HWND, UINT, WPARAM, LPARAM);
typedef LRESULT (WINAPI *TSendMessageTimeoutW)(HWND, UINT, WPARAM, LPARAM, UINT, UINT, PDWORD_PTR);
typedef int (WINAPI *TGetClassNameW)(HWND, LPWSTR, int);
typedef UINT (WINAPI *TRealGetWindowClassW)(HWND, LPWSTR, UINT);

extern TSendMessageW Original_SendMessageW;
extern TSendMessageTimeoutW Original_SendMessageTimeoutW;
extern TGetClassNameW Original_GetClassNameW;
extern TRealGetWindowClassW Original_RealGetWindowClassW;

LRESULT WINAPI Detour_SendMessageW(HWND, UINT, WPARAM, LPARAM);
LRESULT WINAPI Detour_SendMessageTimeoutW(HWND, UINT, WPARAM, LPARAM, UINT, UINT, PDWORD_PTR);
int WINAPI Detour_GetClassNameW(HWND, LPWSTR, int);
UINT WINAPI Detour_RealGetWindowClassW(HWND, LPWSTR, UINT);
