/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <tchar.h>
#include <comdef.h>
#include <windows.h>

#include "hooks.h"
#include "wordhooks.h"

void WordHooks_PreIDispatchInvoke(IDispatch *This, DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr)
{
	ThreadIn ti;
	if (!HooksActive())
		return;
	LastErrorSave les;
	HookSuspension hs;
	if (IsDebuggerPresent())
	{
		WCHAR buf[256];
		swprintf(buf, ARRAYSIZE(buf), L"Word IDispatch Invoke %d %d %d\n", int(dispidMember), int(wFlags), int(pDispParams->cArgs));
		OutputDebugString(buf);
	}
	if (dispidMember == 0x1fb && wFlags == DISPATCH_METHOD && pDispParams->cArgs == 1 && pDispParams->rgvarg[0].vt == VT_BSTR) // TypeText() on Selection
	{
		HWND hwndMaster = FindWindowEx(HWND_MESSAGE, nullptr, TEXT("DictationBridgeMaster"), nullptr);
		if (hwndMaster == nullptr)
		{
			return;
		}
		BSTR bstrText = pDispParams->rgvarg[0].bstrVal;
		DWORD cchText = DWORD(wcslen(bstrText));
		DWORD selStart = -1;
		DWORD cbData = (sizeof(DWORD) * 3) + (cchText * sizeof(WCHAR));
		BYTE* data = new BYTE[cbData];
		HWND hwnd = GetFocus();
		if (hwnd != nullptr)
		{
			TCHAR className[256] = TEXT("");
			GetClassName(hwnd, className, ARRAYSIZE(className));
			if (_tcsicmp(className, _T("_wwg")) != 0 &&
					_tcsicmp(className, _T("_wwo")) != 0 &&
					_tcsicmp(className, _T("_wwn")) != 0)
			{
				hwnd = nullptr;
			}
		}
		DWORD tmp = (DWORD)((__int64)hwnd & 0xffffffff);
		memcpy(data, &tmp, sizeof(tmp));
		tmp = (DWORD)selStart;
		memcpy(data + sizeof(DWORD), &tmp, sizeof(tmp));
		tmp = (DWORD)cchText;
		memcpy(data + sizeof(DWORD) * 2, &tmp, sizeof(tmp));
		memcpy(data + sizeof(DWORD) * 3, bstrText, cchText * sizeof(WCHAR));
		COPYDATASTRUCT cds;
		cds.dwData = 0;
		cds.lpData = (PVOID) data;
		cds.cbData = cbData;
		DWORD_PTR result;
		SendMessageTimeout(hwndMaster, WM_COPYDATA, 0, (LPARAM) &cds, SMTO_BLOCK, 1000, &result);
		delete[] data;
	}
	if (dispidMember == 0 && wFlags == DISPATCH_PROPERTYPUT && pDispParams->cArgs == 1 && pDispParams->rgvarg[0].vt == VT_BSTR && pDispParams->rgvarg[0].bstrVal != nullptr && *(pDispParams->rgvarg[0].bstrVal) == L'\0') // set Text on Selection or Range to an empty string (delete text)
	{
		HWND hwndMaster = FindWindowEx(HWND_MESSAGE, nullptr, TEXT("DictationBridgeMaster"), nullptr);
		if (hwndMaster == nullptr)
		{
			return;
		}
		BSTR bstrText = nullptr;
		_com_dispatch_raw_propget(This, 0, VT_BSTR, &bstrText);
		if (bstrText == nullptr)
		{
			return;
		}
		if (*bstrText == L'\0')
		{
			SysFreeString(bstrText);
			return;
		}
		DWORD cchText = DWORD(wcslen(bstrText));
		DWORD selStart = -1;
		DWORD cbData = (sizeof(DWORD) * 3) + (cchText * sizeof(WCHAR));
		BYTE* data = new BYTE[cbData];
		HWND hwnd = GetFocus();
		if (hwnd != nullptr)
		{
			TCHAR className[256] = TEXT("");
			GetClassName(hwnd, className, ARRAYSIZE(className));
			if (_tcsicmp(className, _T("_wwg")) != 0 &&
					_tcsicmp(className, _T("_wwo")) != 0 &&
					_tcsicmp(className, _T("_wwn")) != 0)
			{
				hwnd = nullptr;
			}
		}
		DWORD tmp = (DWORD)((__int64)hwnd & 0xffffffff);
		memcpy(data, &tmp, sizeof(tmp));
		tmp = (DWORD)selStart;
		memcpy(data + sizeof(DWORD), &tmp, sizeof(tmp));
		tmp = (DWORD)cchText;
		memcpy(data + sizeof(DWORD) * 2, &tmp, sizeof(tmp));
		memcpy(data + sizeof(DWORD) * 3, bstrText, cchText * sizeof(WCHAR));
		COPYDATASTRUCT cds;
		cds.dwData = 2;
		cds.lpData = (PVOID) data;
		cds.cbData = cbData;
		DWORD_PTR result;
		SendMessageTimeout(hwndMaster, WM_COPYDATA, 0, (LPARAM) &cds, SMTO_BLOCK, 1000, &result);
		delete[] data;
	}
}
