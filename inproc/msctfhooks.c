/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#define _WINNT_WINVER 0x0501
#define _WIN32_WINNT 0x0501

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <msctf.h>

#include <MinHook.h>

#include "msctfhooks.h"

typedef struct _HEWK
{
	BOOL bInstalled;
	LPVOID pApiAddr;
	HMODULE hApiMod;
	LPVOID *ppApiCall;
	LPVOID pApiNew;
} HEWK, *PHEWK;

#define HewkDefOriginal(ApiName) \
	T##ApiName Original_##ApiName

#define HewkDef(ApiName) \
	static HEWK AH_##ApiName = {FALSE, NULL, NULL, (LPVOID *)&Original_##ApiName, Detour_##ApiName}

typedef HRESULT (__stdcall *TITfInsertAtSelection_InsertTextAtSelection)(ITfInsertAtSelection *, TfEditCookie, DWORD, const WCHAR *, LONG, ITfRange **);
HewkDefOriginal(ITfInsertAtSelection_InsertTextAtSelection);

static HRESULT __stdcall Detour_ITfInsertAtSelection_InsertTextAtSelection(ITfInsertAtSelection *This, TfEditCookie ec, DWORD dwFlags, const WCHAR *pchText, LONG cch, ITfRange **ppRange)
{
	HRESULT hr = Original_ITfInsertAtSelection_InsertTextAtSelection(This, ec, dwFlags, pchText, cch, ppRange);
	if (hr == S_OK)
	{
		MSCTFHooks_PostInsertTextAtSelection(This, ec, dwFlags, pchText, cch, ppRange);
	}
	return hr;
}

HewkDef(ITfInsertAtSelection_InsertTextAtSelection);

static PHEWK g_Hewks[] =
{
	&AH_ITfInsertAtSelection_InsertTextAtSelection,
	NULL
};

static BOOLEAN g_bAHLockInited = FALSE;
static CRITICAL_SECTION g_HooksLock;

#define HooksLock() EnterCriticalSection(&g_HooksLock)
#define HooksUnlock() LeaveCriticalSection(&g_HooksLock)
#define HooksLockCreate() InitializeCriticalSection(&g_HooksLock)
#define HooksLockDelete() DeleteCriticalSection(&g_HooksLock)

static ULONG Unhook()
{
	ULONG Result, i;

	HooksLock();

	for (Result = i = 0; g_Hewks[i]; i++)
	{
		PHEWK pHewk = g_Hewks[i];
		if (pHewk->bInstalled)
		{
			if (MH_RemoveHook(pHewk->pApiAddr) == MH_OK)
			{
				pHewk->bInstalled = FALSE;
				pHewk->pApiAddr = NULL;
				if (pHewk->hApiMod != NULL)
				{
					FreeLibrary(pHewk->hApiMod);
					pHewk->hApiMod = NULL;
				}
			}
			else
				Result++;
		}
	}

	HooksUnlock();

	return Result;
}

static ULONG Hook(PHEWK pHewk, LPVOID pApiAddr)
{
	ULONG Result = 0;

	HooksLock();

	assert(pApiAddr != NULL);
	if (!pHewk->bInstalled &&
			(pHewk->pApiAddr == NULL || pHewk->pApiAddr == pApiAddr))
	{
		if (pHewk->pApiAddr == NULL)
		{
			pHewk->pApiAddr = pApiAddr;
			pHewk->hApiMod = NULL;
			GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
												(LPCTSTR) pApiAddr, &(pHewk->hApiMod));
		}
		if (pHewk->pApiAddr != NULL &&
				MH_CreateHook(pHewk->pApiAddr, pHewk->pApiNew, pHewk->ppApiCall) == MH_OK &&
				MH_EnableHook(pHewk->pApiAddr))
		{
			pHewk->bInstalled = TRUE;
		}
	}

	if (pHewk->bInstalled && pHewk->pApiAddr == pApiAddr)
		Result++;

	HooksUnlock();

	return Result;
}

void MSCTFHooks_Init(void)
{
	HooksLockCreate();
	g_bAHLockInited = TRUE;
}

void MSCTFHooks_Shutdown(void)
{
	if (g_bAHLockInited)
	{
		Unhook();
		g_bAHLockInited = FALSE;
		HooksLockDelete();
	}
}

void MSCTFHooks_HookInsertAtSelection(ITfInsertAtSelection *ias)
{
	Hook(&AH_ITfInsertAtSelection_InsertTextAtSelection, ias->lpVtbl->InsertTextAtSelection);
}
