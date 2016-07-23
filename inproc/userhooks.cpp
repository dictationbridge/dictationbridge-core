/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <windows.h>
#include <tchar.h>
#include <intrin.h>

#include "dragon.h"
#include "hooks.h"
#include "userhooks.h"

static void BeforeSendMessageFromDragon(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndMaster = FindWindowEx(HWND_MESSAGE, nullptr, TEXT("DictationBridgeMaster"), nullptr);
	if (hwndMaster == nullptr)
	{
		return;
	}
	if (Msg == EM_REPLACESEL)
	{
		LPCWSTR pchText = (LPCWSTR) lParam;
		DWORD cchText = DWORD(wcslen(pchText));
		DWORD selStart = -1;
		SendMessage(hWnd, EM_GETSEL, (WPARAM) (&selStart), 0);
		DWORD cbData = (sizeof(DWORD) * 3) + (cchText * sizeof(WCHAR));
		BYTE* data = new BYTE[cbData];
		DWORD tmp = (DWORD)((__int64)hWnd & 0xffffffff);
		memcpy(data, &tmp, sizeof(tmp));
		tmp = (DWORD)selStart;
		memcpy(data + sizeof(DWORD), &tmp, sizeof(tmp));
		tmp = (DWORD)cchText;
		memcpy(data + sizeof(DWORD) * 2, &tmp, sizeof(tmp));
		memcpy(data + sizeof(DWORD) * 3, pchText, cchText * sizeof(WCHAR));
		COPYDATASTRUCT cds;
		cds.dwData = 0;
		cds.lpData = (PVOID) data;
		cds.cbData = cbData;
		DWORD_PTR result;
		SendMessageTimeout(hwndMaster, WM_COPYDATA, 0, (LPARAM) &cds, SMTO_BLOCK, 1000, &result);
		delete[] data;
	}
}

TSendMessageW Original_SendMessageW = nullptr;

LRESULT WINAPI Detour_SendMessageW(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	ThreadIn ti;
	if (HooksActive())
	{
		ApiHookBody hb;
		if ((Msg == EM_REPLACESEL) && IsDragon(_ReturnAddress()))
		{
			BeforeSendMessageFromDragon(hWnd, Msg, wParam, lParam);
		}
	}
	return Original_SendMessageW(hWnd, Msg, wParam, lParam);
}
