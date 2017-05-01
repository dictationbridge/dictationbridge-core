/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <msctf.h>

#include "hooks.h"
#include "ipc.h"
#include "msctfhooks.h"

void MSCTFHooks_PostInsertTextAtSelection(ITfInsertAtSelection *This, TfEditCookie ec, DWORD dwFlags, const WCHAR *pchText, LONG cchText, ITfRange **ppRange)
{
	ThreadIn ti;
	if (cchText <= 0)
		return;
	if (!HooksActive())
		return;
	LastErrorSave les;
	HookSuspension hs;
	ITfRange* range = nullptr;
	if (ppRange != nullptr)
	{
		range = *ppRange;
	}
	if (range != nullptr)
	{
		MSCTFHooks_HookRange(range);
		ITfContext* ctx = nullptr;
		range->GetContext(&ctx);
		if (ctx != nullptr)
		{
			ITfContextView* view = nullptr;
			ctx->GetActiveView(&view);
			if (view != nullptr)
			{
				HWND hwnd = nullptr;
				view->GetWnd(&hwnd);
				ITfRangeACP* rangeACP = nullptr;
				range->QueryInterface(IID_ITfRangeACP, (void**) &rangeACP);
				if (rangeACP != nullptr)
				{
					LONG acpAnchor = -1, cchRange = -1;
					if (rangeACP->GetExtent(&acpAnchor, &cchRange) == S_OK)
					{
						SendTextInsertedEvent(hwnd, acpAnchor, pchText, cchText);
					}
					rangeACP->Release();
				}
				view->Release();
			}
			ctx->Release();
		}
	}
}

void MSCTFHooks_PreSetText(ITfRange *This, TfEditCookie ec, DWORD dwFlags, const WCHAR *pchText, LONG cch)
{
	ThreadIn ti;
	if (!HooksActive())
		return;
	LastErrorSave les;
	HookSuspension hs;
	if (cch == 0)
	{
		WCHAR pchOrigText[512];
		ULONG cchOrigText;
		HRESULT hr = This->GetText(ec, 0, pchOrigText, ARRAYSIZE(pchOrigText), &cchOrigText);
		if (hr == S_OK && cchOrigText > 0)
		{
			HWND hwnd = nullptr;
			ITfContext* ctx = nullptr;
			This->GetContext(&ctx);
			if (ctx != nullptr)
			{
				ITfContextView* view = nullptr;
				ctx->GetActiveView(&view);
				if (view != nullptr)
				{
					view->GetWnd(&hwnd);
				}
			}
			SendTextDeletedEvent(hwnd, -1, pchOrigText, cchOrigText);
		}
	}
}
