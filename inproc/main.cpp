/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <string.h>
#include <tchar.h>
#include <windows.h>
#include <oleacc.h>
#include <tlhelp32.h>

#include <MinHook.h>

#include "hooks.h"
#include "inproc.h"
#include "ipc.h"
#include "msctfhooks.h"
#include "userhooks.h"
#include "wordhooks.h"

typedef struct _HEWK
{
	bool bInstalled;
	LPCSTR pTargetName;
	LPVOID pTarget;
	LPVOID *ppOriginal;
	LPVOID pDetour;
} HEWK, *PHEWK;

typedef struct _HOOKEE
{
	PHEWK Hooks;
	LPCTSTR Name;
} HOOKEE, *PHOOKEE;

#define HewkEntry(TargetName) \
	{false, #TargetName, nullptr, (LPVOID *)(&Original_##TargetName), Detour_##TargetName}

static HMODULE g_hDll = nullptr;
static CRITICAL_SECTION g_Lock;
static bool g_bLockInitialized = false;
static volatile bool g_bActive = false;
static volatile bool g_bUnloading = false;

#define HooksLock() EnterCriticalSection(&g_Lock)
#define HooksUnlock() LeaveCriticalSection(&g_Lock)

extern "C" volatile LONG g_ThreadsIn = 0;

bool ShouldBePresent()
{
	return IsMasterRunning();
}

static HEWK hkUSER32[] =
{
	HewkEntry(SendMessageW),
	HewkEntry(SendMessageTimeoutW),
	HewkEntry(GetClassNameW),
	HewkEntry(RealGetWindowClassW),
	{false, nullptr}
};

static HOOKEE Hookees[] =
{
	{hkUSER32, TEXT("USER32.DLL")},
	{nullptr}
};

void InitInproc();
void ShutdownInproc();

bool OnLoad()
{
	if (!ShouldBePresent())
		return false;
	InitInproc();
	return true;
}

bool OnUnload()
{
	if (g_ThreadsIn > 0)
	{
		Sleep(250);
		if (g_ThreadsIn > 0)
			return false;
	}
	ShutdownInproc();
	return true;
}

ULONG Unhook1(LONG iHookee)
{
	ULONG Result, i;
	PHOOKEE pHookee = &(Hookees[iHookee]);

	for (Result = i = 0; pHookee->Hooks[i].pTargetName; i++)
	{
		PHEWK pHook = &(pHookee->Hooks[i]);
		if (pHook->bInstalled)
		{
			if (MH_RemoveHook(pHook->pTarget) == MH_OK)
				pHook->bInstalled = false;
			else
				Result++;
		}
	}
	return Result;
}

ULONG Unhook(LONG iHookee)
{
	ULONG Result;

	HooksLock();

	if (iHookee == -1)
	{
		for (Result = iHookee = 0; Hookees[iHookee].Hooks; iHookee++)
			Result += Unhook1(iHookee);
	}
	else
		Result = Unhook1(iHookee);

	HooksUnlock();

	return Result;
}

ULONG Hook1(LONG iHookee, HMODULE hHookee)
{
	ULONG Result, i;
	PHOOKEE pHookee = &(Hookees[iHookee]);

	if (hHookee == nullptr)
	{
		hHookee = GetModuleHandle(pHookee->Name);
		if (hHookee == nullptr)
			return 0;
	}
	for (Result = i = 0; pHookee->Hooks[i].pTargetName; i++)
	{
		PHEWK pHook = &(pHookee->Hooks[i]);
		if (!pHook->bInstalled)
		{
			pHook->pTarget = GetProcAddress(hHookee, pHook->pTargetName);
			if (pHook->pTarget != nullptr &&
					MH_CreateHook(pHook->pTarget, pHook->pDetour, pHook->ppOriginal) == MH_OK &&
					MH_EnableHook(pHook->pTarget) == MH_OK)
			{
				pHook->bInstalled = true;
				Result++;
			}
		}
	}
	return Result;
}

ULONG Hook(LONG iHookee, HMODULE hHookee)
{
	ULONG Result;

	HooksLock();

	if (iHookee == -1)
	{
		for (Result = iHookee = 0; Hookees[iHookee].Hooks; iHookee++)
			Result += Hook1(iHookee, hHookee);
	}
	else
		Result = Hook1(iHookee, hHookee);

	HooksUnlock();

	return(Result);
}

DWORD WINAPI UnhookAndUnload(LPVOID lpv)
{
	bool UnloadNow = false;

	while (1)
	{
		HooksLock();
		if (!ShouldBePresent())
		{
			g_bUnloading = true;
			g_bActive = false;
			UnloadNow = true;
		}
		HooksUnlock();
		if (UnloadNow)
			break;
		Sleep(127);
	}

	while (Unhook(-1))
	{
		Sleep(127);
	}
	while (1)
	{
		bool UnloadSucceeded = false;
		HooksLock();
		UnloadSucceeded = OnUnload();
		HooksUnlock();
		if (UnloadSucceeded)
			break;
		Sleep(127);
	}

	FreeLibraryAndExitThread(g_hDll, 0);
	return 0;
}

void CreateUUThread()
{
	HANDLE hThread = CreateThread(nullptr, 0, UnhookAndUnload, nullptr, 0, nullptr);
	if (hThread)
		CloseHandle(hThread);
}

extern "C" DWORD WINAPI Attach()
{
	DWORD Result = 0x80000000;
	HooksLock();
	if (g_bUnloading)
		Result = 0x80000001;
	else
	{
		if (g_bActive)
			Result = 0;
		else if (OnLoad())
		{
			TCHAR ModuleFileName[MAX_PATH];
			GetModuleFileName(g_hDll, ModuleFileName, ARRAYSIZE(ModuleFileName));
			LoadLibrary(ModuleFileName);
			g_bActive = true;
			CreateUUThread();
			Result = 0;
		}
		if (!(Result & 0x80000000))
		{
			Result = Hook(-1, nullptr);
		}
	}
	HooksUnlock();
	return Result;
}

static DWORD g_dwHooksSuspendedTLS = TLS_OUT_OF_INDEXES;

bool HooksActive()
{
	if (g_bActive)
	{
		LastErrorSave les;
		return (TlsGetValue(g_dwHooksSuspendedTLS) == 0);
	}
	else
		return false;
}

HookSuspension::HookSuspension(): _initialized(false)
{
	UINT_PTR currentValue = (UINT_PTR) TlsGetValue(g_dwHooksSuspendedTLS);
	TlsSetValue(g_dwHooksSuspendedTLS, (LPVOID) (currentValue + 1));
	_initialized = true;
}

HookSuspension::~HookSuspension()
{
	if (_initialized)
	{
		UINT_PTR currentValue = (UINT_PTR) TlsGetValue(g_dwHooksSuspendedTLS);
		TlsSetValue(g_dwHooksSuspendedTLS, (LPVOID) (currentValue - 1));
	}
}

UINT_PTR g_uTimer = 0;

void CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	if (g_uTimer != 0)
	{
		KillTimer(0, g_uTimer);
		g_uTimer = 0;
	}
	ITfThreadMgr* tm = nullptr;
	CoCreateInstance(CLSID_TF_ThreadMgr, nullptr, CLSCTX_INPROC_SERVER, IID_ITfThreadMgr, (void**) &tm);
	if (tm != nullptr)
	{
		ITfDocumentMgr* dm = nullptr;
		tm->GetFocus(&dm);
		if (dm != nullptr)
		{
			OutputDebugString(TEXT("got document manager\n"));
			ITfContext* ctx = nullptr;
			dm->GetBase(&ctx);
			if (ctx != nullptr)
			{
				ITfInsertAtSelection* ias = nullptr;
				ctx->QueryInterface(IID_ITfInsertAtSelection, (void**) &ias);
				if (ias != nullptr)
				{
					MSCTFHooks_HookInsertAtSelection(ias);
					OutputDebugString(TEXT("hooked ITfInsertAtSelection\n"));
					ias->Release();
				}
				ctx->Release();
			}
			dm->Release();
		}
		tm->Release();
	}
}

void InitInproc()
{
	if (g_dwHooksSuspendedTLS == TLS_OUT_OF_INDEXES)
		g_dwHooksSuspendedTLS = TlsAlloc();
	MSCTFHooks_Init();
	WordHooks_Init();
}

void ShutdownInproc()
{
	if (g_uTimer != 0)
	{
		KillTimer(0, g_uTimer);
		g_uTimer = 0;
	}
	MSCTFHooks_Shutdown();
	WordHooks_Shutdown();
	if (g_dwHooksSuspendedTLS != TLS_OUT_OF_INDEXES)
	{
		TlsFree(g_dwHooksSuspendedTLS);
		g_dwHooksSuspendedTLS = TLS_OUT_OF_INDEXES;
	}
}

extern "C" void CALLBACK WinEventProc(HWINEVENTHOOK hook, DWORD event, HWND hwnd, long objid, long child, DWORD idThread, DWORD time)
{
	ThreadIn ti;
	if (idThread != GetCurrentThreadId())
	{
		return;
	}
	if (!g_bActive)
	{
		Attach();
	}
	if (!g_bActive)
	{
		return;
	}
	HWND hwndFocus = GetFocus();
	if (hwndFocus != nullptr)
	{
		TCHAR className[256] = TEXT("");
		GetClassName(hwndFocus, className, ARRAYSIZE(className));
		if (_tcsicmp(className, TEXT("_wwg")) == 0)
		{
			IDispatch* disp = nullptr;
			HRESULT hr = AccessibleObjectFromWindow(hwnd, OBJID_NATIVEOM, IID_IDispatch, (void**) &disp);
			if (hr == S_OK && disp != nullptr)
			{
				WordHooks_HookIDispatchInvoke(disp);
				disp->Release();
			}
		}
	}
	if (g_uTimer != 0)
	{
		KillTimer(0, g_uTimer);
		g_uTimer = 0;
	}
	g_uTimer = SetTimer(0, 0, 100, TimerProc);
}

static HWINEVENTHOOK g_hForegroundEventHook = nullptr;
static HWINEVENTHOOK g_hFocusEventHook = nullptr;

bool InstallHooks()
{
	g_hForegroundEventHook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, g_hDll, WinEventProc, 0, 0, WINEVENT_INCONTEXT);
	if (g_hForegroundEventHook == nullptr)
	{
		return false;
	}
	g_hFocusEventHook = SetWinEventHook(EVENT_OBJECT_FOCUS, EVENT_OBJECT_FOCUS, g_hDll, WinEventProc, 0, 0, WINEVENT_INCONTEXT);
	if (g_hFocusEventHook == nullptr)
	{
		return false;
	}
	return true;
}

void RemoveHooks()
{
	if (g_hFocusEventHook != nullptr)
	{
		UnhookWinEvent(g_hFocusEventHook);
		g_hFocusEventHook = nullptr;
	}
	if (g_hForegroundEventHook != nullptr)
	{
		UnhookWinEvent(g_hForegroundEventHook);
		g_hForegroundEventHook = nullptr;
	}
}

BOOL APIENTRY DllMain(HMODULE hDll, ULONG ulReason, LPVOID lpRsv)
{
	BOOL Result = TRUE;

	if (ulReason == DLL_PROCESS_ATTACH)
	{
		g_hDll = hDll;
		InitializeCriticalSection(&g_Lock);
		g_bLockInitialized = true;
		MH_Initialize();
		DisableThreadLibraryCalls(hDll);
	}
	else if (ulReason == DLL_PROCESS_DETACH)
	{
		g_bUnloading = true;
		g_bActive = false;
		MH_Uninitialize();
		if (g_bLockInitialized)
		{
			g_bLockInitialized = false;
			DeleteCriticalSection(&g_Lock);
		}
	}
	return Result;
}
