/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <windows.h>
#include <msctf.h>

#ifdef __cplusplus
extern "C" {
#endif

void MSCTFHooks_Init(void);
void MSCTFHooks_HookInsertAtSelection(ITfInsertAtSelection *ias);
void MSCTFHooks_PostInsertTextAtSelection(ITfInsertAtSelection *This, TfEditCookie ec, DWORD dwFlags, const WCHAR *pchText, LONG cch, ITfRange **ppRange);
void MSCTFHooks_Shutdown(void);

#ifdef __cplusplus
}
#endif
