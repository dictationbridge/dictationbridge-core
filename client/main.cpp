/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "client.h"

void WINAPI DB_SendCommand(LPCSTR command)
{
	HWND hwnd = FindWindowEx(HWND_MESSAGE, nullptr, TEXT("DictationBridgeMaster"), nullptr);
	if (hwnd == nullptr)
	{
		return;
	}
	COPYDATASTRUCT cds;
	cds.dwData = 1;
	auto cbData = strlen(command);
	auto data = new BYTE[cbData];
	memcpy(data, command, cbData);
	cds.lpData = data;
	cds.cbData = DWORD(cbData);
	DWORD_PTR result;
	SendMessageTimeout(hwnd, WM_COPYDATA, 0, (LPARAM) &cds, SMTO_BLOCK, 1000, &result);
	delete[] data;
}
