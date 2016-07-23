/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <windows.h>

typedef LRESULT (WINAPI *TSendMessageW)(HWND, UINT, WPARAM, LPARAM);
typedef void (WINAPI *Tkeybd_event)(BYTE, BYTE, DWORD, ULONG_PTR);
typedef UINT (WINAPI *TSendInput)(UINT, LPINPUT, int);

extern TSendMessageW Original_SendMessageW;
extern Tkeybd_event Original_keybd_event;
extern TSendInput Original_SendInput;

LRESULT WINAPI Detour_SendMessageW(HWND, UINT, WPARAM, LPARAM);
void WINAPI Detour_keybd_event(BYTE, BYTE, DWORD, ULONG_PTR);
UINT WINAPI Detour_SendInput(UINT, LPINPUT, int);
