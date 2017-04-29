/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <direct.h>
#include <windows.h>
#include <shellapi.h>
#include <tchar.h>

#include "handle.h"
#include "master.h"
#include "wow64.h"

typedef BOOL (WINAPI *TChangeWindowMessageFilter)(UINT, DWORD);

static const TCHAR* const MAIN_CLASS_NAME = TEXT("DictationBridgeMaster");

static HMODULE g_hDll = nullptr;
static ATOM g_atomClass = 0;
static HWND g_hwnd = nullptr;
static TTextInsertedCallback g_TextInsertedCallback = nullptr;
static TTextDeletedCallback g_TextDeletedCallback = nullptr;
static TCommandCallback g_CommandCallback = nullptr;
static TDebugLogCallback g_DebugLogCallback = nullptr;

static LRESULT CALLBACK MainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_COPYDATA:
		{
			const COPYDATASTRUCT& cds = *((const COPYDATASTRUCT*) lParam);
			if (g_DebugLogCallback != nullptr)
			{
				char msg[256];
				sprintf_s(msg, ARRAYSIZE(msg), "WM_COPYDATA: dwData=%d cbData=%d", int(cds.dwData), int(cds.cbData));
				g_DebugLogCallback(msg);
			}
			const BYTE* data = (const BYTE*) (cds.lpData);
			if (cds.dwData == 0 && g_TextInsertedCallback != nullptr)
			{
				HWND hwnd = nullptr;
				memcpy(&hwnd, data, sizeof(DWORD));
				LONG startPosition = 0;
				memcpy(&startPosition, data + sizeof(DWORD), sizeof(DWORD));
				auto textOffset = sizeof(DWORD) * 2;
				auto cchText = (cds.cbData - textOffset) / sizeof(WCHAR);
				WCHAR* text = new WCHAR[cchText + 1];
				memcpy(text, data + textOffset, cchText * sizeof(WCHAR));
				text[cchText] = L'\0';
				g_TextInsertedCallback(hwnd, startPosition, text);
				delete[] text;
			}
			if (cds.dwData == 2 && g_TextDeletedCallback != nullptr)
			{
				HWND hwnd = nullptr;
				memcpy(&hwnd, data, sizeof(DWORD));
				LONG startPosition = 0;
				memcpy(&startPosition, data + sizeof(DWORD), sizeof(DWORD));
				auto textOffset = sizeof(DWORD) * 2;
				auto cchText = (cds.cbData - textOffset) / sizeof(WCHAR);
				WCHAR* text = new WCHAR[cchText + 1];
				memcpy(text, data + textOffset, cchText * sizeof(WCHAR));
				text[cchText] = L'\0';
				g_TextDeletedCallback(hwnd, startPosition, text);
				delete[] text;
			}
			if (cds.dwData == 1 && g_CommandCallback != nullptr)
			{
				char* command = new char[cds.cbData + 1];
				memcpy(command, data, cds.cbData);
				command[cds.cbData] = '\0';
				g_CommandCallback(command);
				delete[] command;
			}
			return 1;
		}
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

static bool RegisterMainClass()
{
	WNDCLASSEX wcx = {sizeof(wcx),};
	wcx.lpfnWndProc = MainWndProc;
	wcx.hInstance = g_hDll;
	wcx.lpszClassName = MAIN_CLASS_NAME;
	g_atomClass = RegisterClassEx(&wcx);
	return g_atomClass != 0;
}

static bool CreateMainWindow()
{
	g_hwnd = CreateWindowEx(0, MAKEINTATOM(g_atomClass), TEXT("DictationBridge Master"), 0, 0, 0, 0, 0, HWND_MESSAGE, 0, g_hDll, 0);
	return g_hwnd != nullptr;
}

static void AllowIpc()
{
	HMODULE hmod = GetModuleHandle(TEXT("user32.dll"));
	TChangeWindowMessageFilter pChangeWindowMessageFilter =
		(TChangeWindowMessageFilter) GetProcAddress(hmod, "ChangeWindowMessageFilter");
	if (pChangeWindowMessageFilter)
	{
		pChangeWindowMessageFilter(WM_COPYDATA, 1);
	}
}

static bool StartHelperProcess(LPCTSTR exeName)
{
	TCHAR exePath[MAX_PATH];
	GetModuleFileName(g_hDll, exePath, MAX_PATH);
	TCHAR* p = _tcsrchr(exePath, _T('\\'));
	assert(p);
	p++;
	_tcscpy(p, exeName);
	// Explicitly inherit the parent process's privileges, most importantly the uiAccess flag.
	HANDLE token;
	if (!OpenProcessToken(GetCurrentProcess(), MAXIMUM_ALLOWED, &token))
	{
		return false;
	}
	STARTUPINFO si = {sizeof(si),};
	PROCESS_INFORMATION pi = {nullptr,};
	if (!CreateProcessAsUser(token, exePath, nullptr, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi))
	{
		CloseHandle(token);
		return false;
	}
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	CloseHandle(token);
	return true;
}

typedef void (WINAPI *TAttach)();

static bool LoadInprocDll(LPCTSTR dllName)
{
	TCHAR dllPath[MAX_PATH];
	GetModuleFileName(g_hDll, dllPath, MAX_PATH);
	TCHAR* p = _tcsrchr(dllPath, _T('\\'));
	assert(p);
	p++;
	_tcscpy(p, dllName);
	HMODULE hDll = LoadLibrary(dllPath);
	if (hDll == nullptr)
	{
		return false;
	}
	TAttach pAttach = (TAttach) GetProcAddress(hDll, "Attach");
	if (pAttach == nullptr)
	{
		FreeLibrary(hDll);
		return false;
	}
	pAttach();
	FreeLibrary(hDll);
	return true;
}

void WINAPI DBMaster_SetTextInsertedCallback(TTextInsertedCallback callback)
{
	g_TextInsertedCallback = callback;
}

void WINAPI DBMaster_SetTextDeletedCallback(TTextDeletedCallback callback)
{
	g_TextDeletedCallback = callback;
}

void WINAPI DBMaster_SetCommandCallback(TCommandCallback callback)
{
	g_CommandCallback = callback;
}

void WINAPI DBMaster_SetDebugLogCallback(TDebugLogCallback callback)
{
	g_DebugLogCallback = callback;
}

BOOL WINAPI DBMaster_Start()
{
	if (!RegisterMainClass())
	{
		return FALSE;
	}
	if (!CreateMainWindow())
	{
		return FALSE;
	}
	AllowIpc();
	if (!StartHelperProcess(TEXT("DictationBridgeInprocLoader32.exe")))
	{
		return FALSE;
	}
#ifndef _M_AMD64
	if (IsWow64())
	{
#endif
		if (!StartHelperProcess(TEXT("DictationBridgeInprocLoader64.exe")))
		{
			return FALSE;
		}
#ifndef _M_AMD64
	}
#endif
#ifdef _M_AMD64
	LoadInprocDll(TEXT("DictationBridgeInproc64.dll"));
#else
	LoadInprocDll(TEXT("DictationBridgeInproc32.dll"));
#endif
	return TRUE;
}

void WINAPI DBMaster_Stop()
{
	if (g_hwnd != nullptr)
	{
		DestroyWindow(g_hwnd);
		g_hwnd = nullptr;
	}
	if (g_atomClass != 0)
	{
		UnregisterClass(MAKEINTATOM(g_atomClass), g_hDll);
		g_atomClass = 0;
	}
}

BOOL APIENTRY DllMain(HMODULE hDll, ULONG ulReason, LPVOID lpRsv)
{
	BOOL Result = TRUE;
	if (ulReason == DLL_PROCESS_ATTACH)
	{
		g_hDll = hDll;
		DisableThreadLibraryCalls(hDll);
	}
	return Result;
}
