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
#include "ipc.h"
#include "wordhooks.h"

static ULONGLONG g_lastNewParagraphTime = 0;
static bool g_lastEventWasNewParagraph = false;

static HWND GetCurrentWordWindow()
{
	HWND hwnd = GetFocus();
	if (hwnd != nullptr)
	{
		TCHAR className[256] = TEXT("");
		GetClassName(hwnd, className, ARRAYSIZE(className));
		if (_tcsicmp(className, _T("_wwg")) != 0 && _tcsicmp(className, _T("_wwo")) != 0 && _tcsicmp(className, _T("_wwn")) != 0)
		{
			hwnd = nullptr;
		}
	}
	return hwnd;
}

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
		Beep(440, 100);
		g_lastEventWasNewParagraph = false;
		BSTR bstrText = pDispParams->rgvarg[0].bstrVal;
		LONG cchText = LONG(SysStringLen(bstrText));
		LONG selStart = -1;
		HWND hwnd = GetCurrentWordWindow();
		if (cchText > 0)
		{
			SendTextInsertedEvent(hwnd, selStart, bstrText, cchText);
		}
	}
	if (dispidMember == 0x200 && wFlags == DISPATCH_METHOD && pDispParams->cArgs == 0)
	{
		ULONGLONG now = GetTickCount64();
		if (!(g_lastEventWasNewParagraph && ((now - g_lastNewParagraphTime) < 100)))
		{
			g_lastEventWasNewParagraph = true;
			g_lastNewParagraphTime = now;
			LONG selStart = -1;
			HWND hwnd = GetCurrentWordWindow();
			SendTextInsertedEvent(hwnd, selStart, L"\r\n\r\n", 4);
		}
	}
	if (dispidMember == 0 && wFlags == DISPATCH_PROPERTYPUT && pDispParams->cArgs == 1 && pDispParams->rgvarg[0].vt == VT_BSTR && pDispParams->rgvarg[0].bstrVal != nullptr && *(pDispParams->rgvarg[0].bstrVal) == L'\0') // set Text on Selection or Range to an empty string (delete text)
	{
		g_lastEventWasNewParagraph = false;
		BSTR bstrText = nullptr;
		_com_dispatch_raw_propget(This, 0, VT_BSTR, &bstrText);
		if (bstrText == nullptr)
		{
			return;
		}
		LONG cchText = LONG(SysStringLen(bstrText));
		if (cchText == 0)
		{
			SysFreeString(bstrText);
			return;
		}
		LONG selStart = -1;
		HWND hwnd = GetCurrentWordWindow();
		SendTextDeletedEvent(hwnd, selStart, bstrText, cchText);
		SysFreeString(bstrText);
	}
}
