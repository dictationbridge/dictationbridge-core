/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <windows.h>

class LastErrorSave
{
public:
	void update()
	{
		_lastError = GetLastError();
	}
	LastErrorSave()
	{
		update();
	}
	~LastErrorSave()
	{
		SetLastError(_lastError);
	}
private:
	LastErrorSave(const LastErrorSave&);
	LastErrorSave& operator=(const LastErrorSave&);
	DWORD _lastError;
};
