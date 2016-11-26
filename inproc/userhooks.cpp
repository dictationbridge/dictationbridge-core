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
#include "handle.h"
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
		LPWSTR pszText = (LPWSTR) lParam;
		DWORD cchText = DWORD(wcslen(pszText));
		if (cchText == 0)
		{
			DWORD selStart = 0, selEnd = 0;
			SendMessage(hWnd, EM_GETSEL, (WPARAM)&selStart, (LPARAM)&selEnd);
			if (selStart == selEnd)
			{
				return;
			}
			DWORD cchAllText = (DWORD)(SendMessageW(hWnd, WM_GETTEXTLENGTH, 0, 0));
			if (cchAllText < selEnd)
			{
				return;
			}
			LPWSTR pszAllText = new WCHAR[cchAllText + 1];
			cchAllText = (DWORD)(SendMessageW(hWnd, WM_GETTEXT, cchAllText + 1, (LPARAM) pszAllText));
			if (cchAllText < selEnd)
			{
				delete[] pszAllText;
				return;
			}
			cchText = selEnd - selStart;
			pszText = new WCHAR[cchText + 1];
			wcsncpy(pszText, pszAllText + selStart, cchText);
			pszText[cchText] = L'\0';
			delete[] pszAllText;
			DWORD cbData = (sizeof(DWORD) * 3) + (cchText * sizeof(WCHAR));
			BYTE* data = new BYTE[cbData];
			DWORD tmp = (DWORD)((__int64)hWnd & 0xffffffff);
			memcpy(data, &tmp, sizeof(tmp));
			tmp = (DWORD)selStart;
			memcpy(data + sizeof(DWORD), &tmp, sizeof(tmp));
			tmp = (DWORD)cchText;
			memcpy(data + sizeof(DWORD) * 2, &tmp, sizeof(tmp));
			memcpy(data + sizeof(DWORD) * 3, pszText, cchText * sizeof(WCHAR));
			COPYDATASTRUCT cds;
			cds.dwData = 2;
			cds.lpData = (PVOID) data;
			cds.cbData = cbData;
			DWORD_PTR result;
			SendMessageTimeout(hwndMaster, WM_COPYDATA, 0, (LPARAM) &cds, SMTO_BLOCK, 1000, &result);
			delete[] data;
			delete[] pszText;
			return;
		}
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
		memcpy(data + sizeof(DWORD) * 3, pszText, cchText * sizeof(WCHAR));
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

TSendMessageTimeoutW Original_SendMessageTimeoutW = nullptr;

LRESULT WINAPI Detour_SendMessageTimeoutW(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, UINT fuFlags, UINT uTimeout, PDWORD_PTR lpdwResult)
{
	ThreadIn ti;
	if (Msg == WM_GETOBJECT && HooksActive())
	{
		ApiHookBody hb;
		TCHAR className[256] = _T("");
		GetClassName(hWnd, className, ARRAYSIZE(className));
		if (_tcsicmp(className, _T("ListBox")) == 0)
		{
			DWORD pid = 0;
			GetWindowThreadProcessId(hWnd, &pid);
			if (pid > 0)
			{
				Handle process(OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid));
				if (process)
				{
					TCHAR exePath[MAX_PATH];
					DWORD exePathSize = ARRAYSIZE(exePath);
					if (QueryFullProcessImageName(process, 0, exePath, &exePathSize))
					{
						TCHAR* p = _tcsrchr(exePath, _T('\\'));
						if (p == nullptr)
						{
							p = exePath;
						}
						else
						{
							p++;
						}
						if (_tcsicmp(p, _T("natspeak.exe")) == 00)
						{
							if (lpdwResult != nullptr)
							{
								*lpdwResult = 0;
							}
							return 0;
						}
					}
				}
			}
		}
	}
	return Original_SendMessageTimeoutW(hWnd, Msg, wParam, lParam, fuFlags, uTimeout, lpdwResult);
}

TGetClassNameW Original_GetClassNameW = nullptr;

int WINAPI Detour_GetClassNameW(HWND hWnd, LPWSTR lpClassName, int nMaxCount)
{
	ThreadIn ti;
	int result = Original_GetClassNameW(hWnd, lpClassName, nMaxCount);
	if (result > 0 && HooksActive())
	{
		ApiHookBody hb;
		if (_wcsicmp(lpClassName, L"ListBox") == 0)
		{
			DWORD pid = 0;
			GetWindowThreadProcessId(hWnd, &pid);
			if (pid > 0)
			{
				Handle process(OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid));
				if (process)
				{
					TCHAR exePath[MAX_PATH];
					DWORD exePathSize = ARRAYSIZE(exePath);
					if (QueryFullProcessImageName(process, 0, exePath, &exePathSize))
					{
						TCHAR* p = _tcsrchr(exePath, _T('\\'));
						if (p == nullptr)
						{
							p = exePath;
						}
						else
						{
							p++;
						}
						if (_tcsicmp(p, _T("natspeak.exe")) == 00)
						{
							LPCWSTR newName = L"CustomListBox";
							wcsncpy(lpClassName, newName, nMaxCount);
							int newNameLen = int(wcslen(newName));
							return (nMaxCount < newNameLen) ? nMaxCount : newNameLen;
						}
					}
				}
			}
		}
	}
	return result;
}

TRealGetWindowClassW Original_RealGetWindowClassW = nullptr;

UINT WINAPI Detour_RealGetWindowClassW(HWND hWnd, LPWSTR lpClassName, UINT nMaxCount)
{
	ThreadIn ti;
	UINT result = Original_RealGetWindowClassW(hWnd, lpClassName, nMaxCount);
	if (result > 0 && HooksActive())
	{
		ApiHookBody hb;
		if (_wcsicmp(lpClassName, L"ListBox") == 0)
		{
			DWORD pid = 0;
			GetWindowThreadProcessId(hWnd, &pid);
			if (pid > 0)
			{
				Handle process(OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid));
				if (process)
				{
					TCHAR exePath[MAX_PATH];
					DWORD exePathSize = ARRAYSIZE(exePath);
					if (QueryFullProcessImageName(process, 0, exePath, &exePathSize))
					{
						TCHAR* p = _tcsrchr(exePath, _T('\\'));
						if (p == nullptr)
						{
							p = exePath;
						}
						else
						{
							p++;
						}
						if (_tcsicmp(p, _T("natspeak.exe")) == 00)
						{
							LPCWSTR newName = L"CustomListBox";
							wcsncpy(lpClassName, newName, nMaxCount);
							UINT newNameLen = UINT(wcslen(newName));
							return (nMaxCount < newNameLen) ? nMaxCount : newNameLen;
						}
					}
				}
			}
		}
	}
	return result;
}
