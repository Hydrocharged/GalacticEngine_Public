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

// Adapted from https://github.com/NVIDIAGameWorks/donut/blob/main/src/core/string_utils.cpp

#include <engine/strings/strings.hpp>

namespace engine::strings {
	template<>
	std::optional<bool> FromString(const std::string& s) {
		return ToBool(s);
	}

	template<>
	std::optional<float> Parse(std::string_view s) {
		Trim(s);
		Trim(s, '+');

		char buf[32];
		buf[sizeof(buf) - 1] = 0;
		strncpy(buf, s.data(), std::min(s.size(), sizeof(buf) - 1));
		char* endptr = buf;
		float value = strtof(buf, &endptr);

		if (endptr == buf) {
			return std::optional<float>();
		}

		return value;
	}

	template<>
	std::optional<double> Parse(std::string_view s) {
		Trim(s);
		Trim(s, '+');

		char buf[32];
		buf[sizeof(buf) - 1] = 0;
		strncpy(buf, s.data(), std::min(s.size(), sizeof(buf) - 1));
		char* endptr = buf;
		double value = strtod(buf, &endptr);

		if (endptr == buf) {
			return std::optional<double>();
		}

		return value;
	}

	template<>
	std::optional<bool> Parse<bool>(std::string_view s) {
		return ToBool(s);
	}

	template<>
	std::optional<std::string_view> Parse<std::string_view>(std::string_view s) {
		Trim(s);
		Trim(s, '"');
		return s;
	}

	template<>
	std::optional<std::string> Parse<std::string>(std::string_view s) {
		if (auto r = Parse<std::string_view>(s)) {
			return std::string(*r);
		}
		return std::nullopt;
	}

	template<>
	long ToNumber(const std::string& s) {
		return std::stol(s, nullptr, 0);
	}

	template<>
	float ToNumber(const std::string& s) {
		return std::stof(s);
	}

	template<>
	double ToNumber(const std::string& s) {
		return std::stod(s);
	}
}
