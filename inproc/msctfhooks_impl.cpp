/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <msctf.h>

#include "hooks.h"
#include "msctfhooks.h"

void MSCTFHooks_PostInsertTextAtSelection(ITfInsertAtSelection *This, TfEditCookie ec, DWORD dwFlags, const WCHAR *pchText, LONG cchText, ITfRange **ppRange)
{
	ThreadIn ti;
	if (!HooksActive())
		return;
	LastErrorSave les;
	HookSuspension hs;
	HWND hwndMaster = FindWindowEx(HWND_MESSAGE, nullptr, TEXT("DictationBridgeMaster"), nullptr);
	if (hwndMaster == nullptr)
	{
		return;
	}
	ITfRange* range = nullptr;
	if (ppRange != nullptr)
	{
		range = *ppRange;
	}
	if (range != nullptr)
	{
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
						DWORD cbData = (sizeof(DWORD) * 3) + (cchText * sizeof(WCHAR));
						BYTE* data = new BYTE[cbData];
						DWORD tmp = (DWORD)((__int64)hwnd & 0xffffffff);
						memcpy(data, &tmp, sizeof(tmp));
						tmp = (DWORD)acpAnchor;
						memcpy(data + sizeof(DWORD), &tmp, sizeof(tmp));
						tmp = (DWORD)cchRange;
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
					rangeACP->Release();
				}
				view->Release();
			}
			ctx->Release();
		}
	}
}
