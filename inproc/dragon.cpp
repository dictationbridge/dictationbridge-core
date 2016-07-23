/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <windows.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>

#include "dragon.h"

extern "C" BOOL IsDragon(void* address)
{
	HMODULE hmod = nullptr;
	GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR) address, &hmod);
	if (hmod == nullptr)
	{
		return FALSE;
	}
	TCHAR path[MAX_PATH];
	if (!GetModuleFileName(hmod, path, ARRAYSIZE(path)))
	{
		FreeLibrary(hmod);
		return FALSE;
	}
	FreeLibrary(hmod);
	TCHAR longPath[MAX_PATH];
	if (!GetLongPathName(path, longPath, ARRAYSIZE(longPath)))
	{
		return FALSE;
	}
	size_t l = _tcslen(longPath);
	for (size_t i = 0; i < l; i++)
	{
		if (_istalpha(longPath[i]))
		{
			longPath[i] = _totlower(longPath[i]);
		}
	}
	if (_tcsstr(longPath, _T("naturallyspeaking")) != nullptr)
	{
		return TRUE;
	}
	return FALSE;
}
