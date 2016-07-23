/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <windows.h>

typedef LRESULT (WINAPI *TSendMessageW)(HWND, UINT, WPARAM, LPARAM);

extern TSendMessageW Original_SendMessageW;

LRESULT WINAPI Detour_SendMessageW(HWND, UINT, WPARAM, LPARAM);
