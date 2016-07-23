/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <windows.h>

typedef BOOL (WINAPI *TIsWow64Process)(HANDLE, PBOOL);

static inline bool IsWow64(HANDLE hProcess = nullptr)
{
	HMODULE hmod = GetModuleHandle(TEXT("kernel32.dll"));
	TIsWow64Process pfn =
		(TIsWow64Process) GetProcAddress(hmod, "IsWow64Process");
	if (!pfn)
		return false;
	if (!hProcess)
		hProcess = GetCurrentProcess();
	BOOL wow64;
	BOOL rv = pfn(hProcess, &wow64);
	return rv && wow64;
}
