// Copyright © 2022-2023 Daylon Wilkins & James Cor
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/*
* Copyright (c) 2014-2021, NVIDIA CORPORATION. All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
* DEALINGS IN THE SOFTWARE.
*/

// Adapted from https://github.com/NVIDIAGameWorks/donut/blob/main/include/donut/core/log.h

#ifndef ENGINE_LOG_HPP
#define ENGINE_LOG_HPP

#include <functional>

namespace engine::log {
	enum class Severity {
		None = 0,
		Debug,
		Info,
		Warning,
		Error,
		Fatal
	};

	typedef std::function<void(Severity, const char*)> Callback;

	void SetMinSeverity(Severity severity);
	void SetCallback(Callback func);
	Callback GetCallback();
	void ResetCallback();
	void SetErrorMessageCaption(const char* caption);

	void Message(Severity severity, const char* fmt...);
	void Debug(const char* fmt...);
	void Info(const char* fmt...);
	void Warning(const char* fmt...);
	void Error(const char* fmt...);
	void Fatal(const char* fmt...);
}

#endif //ENGINE_LOG_HPP
