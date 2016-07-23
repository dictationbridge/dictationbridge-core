/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "threadin.h"
#include "lasterror.h"

bool HooksActive();

class HookSuspension
{
public:
	HookSuspension();
	~HookSuspension();
private:
	HookSuspension(const HookSuspension&);
	HookSuspension& operator=(const HookSuspension&);
	bool _initialized;
};

class ApiHookBody
{
public:
	ApiHookBody()
	{
	}
private:
	LastErrorSave _les;
	HookSuspension _hs;
	ApiHookBody(const ApiHookBody&);
	ApiHookBody& operator=(const ApiHookBody&);
};
