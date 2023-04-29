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

// Adapted from https://github.com/NVIDIAGameWorks/donut/blob/main/src/core/log.cpp

/*
The following is a table mapping each token to its expected type and output

Token      Output                                                  Example
%d         Signed decimal integer                                  392
%i         Signed decimal integer (same as %d)                     -392
%u         Unsigned decimal integer                                7235
%o         Unsigned octal                                          610
%x         Unsigned hexadecimal integer                            7fa
%X         Unsigned hexadecimal integer (uppercase)                7FA
%f         Decimal floating point, lowercase                       392.65
%F         Decimal floating point, uppercase                       392.65
%e         Scientific notation (mantissa/exponent), lowercase      3.9265e+2
%E         Scientific notation (mantissa/exponent), uppercase      3.9265E+2
%g         Use the shortest representation: %e or %f               392.65
%G         Use the shortest representation: %E or %F               392.65
%a         Hexadecimal floating point, lowercase                   -0xc.90fep-2
%A         Hexadecimal floating point, uppercase                   -0XC.90FEP-2
%c         Character                                               a
%s         String of characters                                    abc
%p         Pointer address                                         b8000000
*/

#include <engine/log/log.hpp>
#include <cstdio>
#include <cstdarg>
#include <iterator>
#include <mutex>

#ifdef PLATFORM_WIN32
#include <Windows.h>
#endif

namespace engine::log {
	static constexpr size_t messageBufferSize = 4096;
	static std::string errorMessageCaption = "Error";
	static std::mutex logMutex;

	void DefaultCallback(Severity severity, const char* message) {
		const char* severityText = "";
		switch (severity) {
			case Severity::Debug:
				severityText = "DEBUG";
				break;
			case Severity::Info:
				severityText = "INFO";
				break;
			case Severity::Warning:
				severityText = "WARNING";
				break;
			case Severity::Error:
				severityText = "ERROR";
				break;
			case Severity::Fatal:
				severityText = "FATAL";
				break;
			default:
				break;
		}

		char buf[messageBufferSize];
		snprintf(buf, std::size(buf), "%s: %s", severityText, message);

		{
			std::lock_guard<std::mutex> lockGuard(logMutex);

			if (severity >= Severity::Error) {
				fprintf(stderr, "%s\n", buf);
#ifdef PLATFORM_WIN32
				MessageBoxA(0, buf, errorMessageCaption.c_str(), MB_ICONERROR);
#endif
			} else if (severity == Severity::Warning) {
				fprintf(stderr, "%s\n", buf);
			} else {
				fprintf(stdout, "%s\n", buf);
			}
		}

		if (severity == Severity::Fatal) {
			abort();
		}
	}

	void SetErrorMessageCaption(const char* caption) {
		errorMessageCaption = (caption) ? caption : "";
	}

	static Callback logCallback = &DefaultCallback;
#if !defined(NDEBUG) || defined(_DEBUG)
	static Severity minSeverity = Severity::Debug;
#else
	static Severity minSeverity = Severity::Info;
#endif

	void SetMinSeverity(Severity severity) {
		minSeverity = severity;
	}

	void SetCallback(Callback func) {
		logCallback = func;
	}

	Callback GetCallback() {
		return logCallback;
	}

	void ResetCallback() {
		logCallback = &DefaultCallback;
	}

	void Message(Severity severity, const char* fmt...) {
		if (static_cast<int>(minSeverity) > static_cast<int>(severity)) {
			return;
		}
		char buffer[messageBufferSize];
		va_list args;
			va_start(args, fmt);
		vsnprintf(buffer, std::size(buffer), fmt, args);
		logCallback(severity, buffer);
			va_end(args);
	}

	void Debug(const char* fmt...) {
		if (static_cast<int>(minSeverity) > static_cast<int>(Severity::Debug)) {
			return;
		}
		char buffer[messageBufferSize];
		va_list args;
			va_start(args, fmt);
		vsnprintf(buffer, std::size(buffer), fmt, args);
		logCallback(Severity::Debug, buffer);
			va_end(args);
	}

	void Info(const char* fmt...) {
		if (static_cast<int>(minSeverity) > static_cast<int>(Severity::Info)) {
			return;
		}
		char buffer[messageBufferSize];
		va_list args;
			va_start(args, fmt);
		vsnprintf(buffer, std::size(buffer), fmt, args);
		logCallback(Severity::Info, buffer);
			va_end(args);
	}

	void Warning(const char* fmt...) {
		if (static_cast<int>(minSeverity) > static_cast<int>(Severity::Warning)) {
			return;
		}
		char buffer[messageBufferSize];
		va_list args;
			va_start(args, fmt);
		vsnprintf(buffer, std::size(buffer), fmt, args);
		logCallback(Severity::Warning, buffer);
			va_end(args);
	}

	void Error(const char* fmt...) {
		if (static_cast<int>(minSeverity) > static_cast<int>(Severity::Error)) {
			return;
		}
		char buffer[messageBufferSize];
		va_list args;
			va_start(args, fmt);
		vsnprintf(buffer, std::size(buffer), fmt, args);
		logCallback(Severity::Error, buffer);
			va_end(args);
	}

	void Fatal(const char* fmt...) {
		char buffer[messageBufferSize];
		va_list args;
			va_start(args, fmt);
		vsnprintf(buffer, std::size(buffer), fmt, args);
		logCallback(Severity::Fatal, buffer);
			va_end(args);
	}
}
