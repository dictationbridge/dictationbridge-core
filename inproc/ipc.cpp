/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <stdio.h>
#include <tchar.h>
#include <windows.h>

#include "ipc.h"

HWND GetMasterWindow()
{
	return FindWindowEx(HWND_MESSAGE, nullptr, TEXT("DictationBridgeMaster"), nullptr);
}

bool IsMasterRunning()
{
	return GetMasterWindow() != nullptr;
}

void SendTextInsertedEvent(HWND hwnd, LONG startPosition, LPCWSTR text, LONG cchText)
{
	HWND hwndMaster = GetMasterWindow();
	if (hwndMaster == nullptr)
	{
		return;
	}
	DWORD cbData = (sizeof(DWORD) * 2) + (cchText * sizeof(WCHAR));
	BYTE* data = new BYTE[cbData];
	DWORD tmp = (DWORD)((__int64)hwnd & 0xffffffff);
	memcpy(data, &tmp, sizeof(tmp));
	tmp = (DWORD)startPosition;
	memcpy(data + sizeof(DWORD), &tmp, sizeof(tmp));
	memcpy(data + sizeof(DWORD) * 2, text, cchText * sizeof(WCHAR));
	COPYDATASTRUCT cds;
	cds.dwData = 0;
	cds.lpData = (PVOID) data;
	cds.cbData = cbData;
	DWORD_PTR result;
	SendMessageTimeout(hwndMaster, WM_COPYDATA, 0, (LPARAM) &cds, SMTO_NORMAL, 1000, &result);
	delete[] data;
}

void SendTextDeletedEvent(HWND hwnd, LONG startPosition, LPCWSTR text, LONG cchText)
{
	HWND hwndMaster = GetMasterWindow();
	if (hwndMaster == nullptr)
	{
		return;
	}
	DWORD cbData = (sizeof(DWORD) * 2) + (cchText * sizeof(WCHAR));
	BYTE* data = new BYTE[cbData];
	DWORD tmp = (DWORD)((__int64)hwnd & 0xffffffff);
	memcpy(data, &tmp, sizeof(tmp));
	tmp = (DWORD)startPosition;
	memcpy(data + sizeof(DWORD), &tmp, sizeof(tmp));
	memcpy(data + sizeof(DWORD) * 2, text, cchText * sizeof(WCHAR));
	COPYDATASTRUCT cds;
	cds.dwData = 2;
	cds.lpData = (PVOID) data;
	cds.cbData = cbData;
	DWORD_PTR result;
	SendMessageTimeout(hwndMaster, WM_COPYDATA, 0, (LPARAM) &cds, SMTO_NORMAL, 1000, &result);
	delete[] data;
}
