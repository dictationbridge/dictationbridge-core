/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

void WordHooks_Init(void);
void WordHooks_HookIDispatchInvoke(IDispatch *disp);
void WordHooks_PostIDispatchInvoke(IDispatch *This, DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr);
void WordHooks_Shutdown(void);

#ifdef __cplusplus
}
#endif
