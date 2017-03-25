/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

bool IsMasterRunning();
void SendTextInsertedEvent(HWND hwnd, LONG startPosition, LPCWSTR text, LONG cchText);
void SendTextDeletedEvent(HWND hwnd, LONG startPosition, LPCWSTR text, LONG cchText);
