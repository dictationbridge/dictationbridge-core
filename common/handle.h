/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <windows.h>

class Handle
{
public:
	Handle(): _h(0)
	{
	}
	Handle(HANDLE h): _h(h)
	{
	}
	void release()
	{
		if (_h)
		{
			CloseHandle(_h);
			_h = 0;
		}
	}
	~Handle()
	{
		release();
	}
	Handle& operator=(HANDLE h)
	{
		release();
		_h = h;
		return *this;
	}
	HANDLE get()
	{
		return _h;
	}
	operator HANDLE()
	{
		return _h;
	}
	operator bool() const
	{
		return (_h != 0);
	}
	bool operator!() const
	{
		return (_h == 0);
	}
private:
	Handle(const Handle&);
	Handle& operator=(const Handle&);
	HANDLE _h;
};
