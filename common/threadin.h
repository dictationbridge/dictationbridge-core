/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <windows.h>

extern "C" volatile LONG g_ThreadsIn;

class ThreadIn
{
public:
	ThreadIn()
	{
		InterlockedIncrement(&g_ThreadsIn);
	}
	~ThreadIn()
	{
		InterlockedDecrement(&g_ThreadsIn);
	}
private:
	ThreadIn(const ThreadIn&);
	ThreadIn& operator=(const ThreadIn&);
};
