/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <windows.h>

class Library
{
public:
	Library(): _h(0)
	{
	}
	Library(HMODULE h): _h(h)
	{
	}
	void release()
	{
		if (_h)
		{
			FreeLibrary(_h);
			_h = 0;
		}
	}
	~Library()
	{
		release();
	}
	Library& operator=(HMODULE h)
	{
		release();
		_h = h;
		return *this;
	}
	HMODULE get()
	{
		return _h;
	}
	operator HMODULE()
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
	Library(const Library&);
	Library& operator=(const Library&);
	HMODULE _h;
};
