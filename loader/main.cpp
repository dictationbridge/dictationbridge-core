/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <windows.h>
#include <tchar.h>

#include "inproc.h"

static void nap(DWORD ms)
{
	MSG msg;
	HANDLE h = 0;
	DWORD startTime = GetTickCount();
	DWORD now;
	while ((now = GetTickCount()) < (startTime + ms))
	{
		MsgWaitForMultipleObjects(0, &h, FALSE, (startTime + ms) - now,
															QS_ALLINPUT | QS_ALLEVENTS);
		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

int WINAPI _tWinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPTSTR cmdLine,
										 int nCmdShow)
{
	if (!InstallHooks())
	{
		return 1;
	}
	while (true)
	{
		HWND hwnd = FindWindowEx(HWND_MESSAGE, nullptr, TEXT("DictationBridgeMaster"), nullptr);
		if (hwnd == nullptr)
			break;
		nap(500);
	}
	RemoveHooks();
	return 0;
}
