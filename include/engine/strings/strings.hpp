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

// Adapted from https://github.com/NVIDIAGameWorks/donut/blob/main/include/donut/core/string_utils.h

#ifndef ENGINE_STRINGS_STRINGS_HPP
#define ENGINE_STRINGS_STRINGS_HPP

#include <array>
#include <cctype>
#include <charconv>
#include <codecvt>
#include <locale>
#include <optional>
#include <regex>
#include <string>
#include <string_view>
#include <vector>

#ifdef _MSC_VER
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#endif

namespace engine::strings {
	inline size_t Length(const char* s) {
		if (s == nullptr) {
			return 0;
		}
		size_t strLen = 0;
		while (s[strLen] != '\0') {
			strLen++;
		}
		return strLen;
	}

	inline bool EqualCaseInsensitive(const std::string& a, const std::string& b) {
		return strcasecmp(a.c_str(), b.c_str()) == 0;
	}

	template<typename T>
	bool EqualCaseInsensitive(const T& a, const T& b) {
		return a.size() == b.size() &&
			   std::equal(a.begin(), a.end(), b.begin(), b.end(),
						  [](char a, char b) { return std::tolower(a) == std::tolower(b); });
	}

	template<typename T>
	bool EqualCaseInsensitive(const T& a, const T& b, size_t n) {
		return a.size() >= n && b.size() >= n &&
			   std::equal(a.begin(), a.begin() + n, b.begin(), b.begin() + n,
						  [](char a, char b) { return std::tolower(a) == std::tolower(b); });
	}

	inline bool StartsWith(const std::string_view& value, const std::string_view& beginning) {
		if (beginning.size() > value.size()) {
			return false;
		}

		return std::equal(beginning.begin(), beginning.end(), value.begin());
	}

	inline bool EndsWith(const std::string_view& value, const std::string_view& ending) {
		if (ending.size() > value.size()) {
			return false;
		}

		return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
	}

	inline void LeftTrim(std::string_view& s) {
		s.remove_prefix(std::distance(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
			return !isspace(ch);
		})));
	}

	inline void LeftTrim(std::string& s) {
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
			return !std::isspace(ch);
		}));
	}

	inline void RightTrim(std::string_view& s) {
		s.remove_suffix(std::distance(std::find_if(s.rbegin(), s.rend(), [](int ch) {
			return !isspace(ch);
		}).base(), s.end()));
	}

	inline void RightTrim(std::string& s) {
		s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
			return !std::isspace(ch);
		}).base(), s.end());
	}

	template<typename T>
	void Trim(T& s) {
		LeftTrim(s);
		RightTrim(s);
	}

	inline void LeftTrim(std::string_view& s, int character) {
		s.remove_prefix(std::distance(s.begin(), std::find_if(s.begin(), s.end(), [&](int ch) {
			return ch != character;
		})));
	}

	inline void LeftTrim(std::string& s, int character) {
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), [&](int ch) {
			return ch != character;
		}));
	}

	inline void RightTrim(std::string_view& s, int character) {
		s.remove_suffix(std::distance(std::find_if(s.rbegin(), s.rend(), [&](int ch) {
			return ch != character;
		}).base(), s.end()));
	}

	inline void RightTrim(std::string& s, int character) {
		s.erase(std::find_if(s.rbegin(), s.rend(), [&](int ch) {
			return ch != character;
		}).base(), s.end());
	}

	template<typename T>
	void Trim(T& s, int character) {
		LeftTrim(s, character);
		RightTrim(s, character);
	}

	inline void ToLower(std::string& s) {
		std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
	}

	inline void ToUpper(std::string& s) {
		std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::toupper(c); });
	}

	inline std::wstring ToWide(std::string& s) {
#ifdef PLATFORM_WIN32
		return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().from_bytes(s);
#else //PLATFORM_WIN32
		return std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(s);
#endif //PLATFORM_WIN32
	}

	inline std::wstring ToWide(const char* s) {
#ifdef PLATFORM_WIN32
		return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().from_bytes(s);
#else //PLATFORM_WIN32
		return std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(s);
#endif //PLATFORM_WIN32
	}

	inline std::string FromWide(std::wstring& s) {
#ifdef PLATFORM_WIN32
		return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().to_bytes(s);
#else //PLATFORM_WIN32
		return std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(s);
#endif //PLATFORM_WIN32
	}

	inline std::string FromWide(const wchar_t* s) {
#ifdef PLATFORM_WIN32
		return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().to_bytes(s);
#else //PLATFORM_WIN32
		return std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(s);
#endif //PLATFORM_WIN32
	}

	inline std::vector<std::string> Split(const std::string& s, const char* regex = "[\\s+,|:]") {
		std::regex rx(regex);
		std::sregex_token_iterator it{s.begin(), s.end(), rx, -1}, end;

		std::vector<std::string> tokens;
		for (; it != end; ++it) {
			if (!it->str().empty()) {
				tokens.push_back(*it);
			}
		}
		return tokens;
	}

	inline std::vector<std::string_view> Split(std::string_view const s, const char* regex = "[\\s+,|:]") {
		std::regex rx(regex);
		std::cregex_token_iterator it{s.data(), s.data() + s.size(), rx, -1}, end;

		std::vector<std::string_view> tokens;
		for (; it != end; ++it) {
			if (it->length() > 0) {
				tokens.push_back({it->first, (size_t)it->length()});
			}
		}
		return tokens;
	}

	template<typename T>
	bool IsTrue(const T& s) {
		return engine::strings::EqualCaseInsensitive(s, T("true")) || engine::strings::EqualCaseInsensitive(s, T("on")) ||
			   engine::strings::EqualCaseInsensitive(s, T("yes")) || engine::strings::EqualCaseInsensitive(s, T("1"));
	}

	template<typename T>
	bool IsFalse(const T& s) {
		return engine::strings::EqualCaseInsensitive(s, T("false")) || engine::strings::EqualCaseInsensitive(s, T("off")) ||
			   engine::strings::EqualCaseInsensitive(s, T("no")) || engine::strings::EqualCaseInsensitive(s, T("0"));
	}

	inline std::optional<bool> ToBool(std::string_view s) {
		Trim(s);
		if (IsTrue(s)) { return true; }
		if (IsFalse(s)) { return false; }
		return std::nullopt;
	}

	template<typename T>
	T ToNumber(const std::string& s) { return (T)std::stoi(s, nullptr, 0); }

	template<typename T>
	std::optional<T> FromString(const std::string& s) {
		T value;
		try { value = ToNumber<T>(s); }
		catch (std::invalid_argument&) { return std::nullopt; }
		catch (std::out_of_range&) { return std::nullopt; }
		return value;
	}

	template<typename T>
	std::optional<T> Parse(std::string_view s) {
		Trim(s);
		if (engine::strings::EqualCaseInsensitive(s, std::string_view("0x"), 2)) {
			// as of C++17, std::from_chars does handle hex, so fall back on strings
			return FromString<T>(std::string(s));
		} else {
			// as of C++17, std::from_chars returns an error when parsing integers
			// with a '+' sign prefix
			LeftTrim(s, '+');
			T value;
			if (auto [p, ec] = std::from_chars(s.data(), s.data() + s.size(), value); ec == std::errc()) {
				return value;
			}
		}
		return std::optional<T>();
	}

	template<typename T>
	std::optional<T> Parse(const std::string& s) {
		return Parse<T>(std::string_view(s));
	}

	template<typename T>
	std::optional<T> ParseVector(std::string_view s) {
		std::regex rx("[\\s+,|:]");
		std::cregex_token_iterator it{s.data(), s.data() + s.size(), rx, -1}, last;

		T value;
		uint8_t dim = 0;
		for (; it != last; ++it) {
			if (it->length() == 0) {
				continue;
			}
			if (dim >= T::DIM) {
				return std::optional<T>();
			}
			if (auto v = Parse<decltype(value.x)>(std::string_view({it->first, (size_t)it->length()}))) {
				value[dim++] = *v;
			} else {
				return std::optional<T>();
			}
		}
		return dim == T::DIM ? value : std::optional<T>();
	}

	template<typename T>
	std::optional<T> ParseVector(const std::string& s) {
		return ParseVector<T>(std::string_view(s));
	}

	template<>
	long ToNumber(const std::string& s);
	template<>
	float ToNumber(const std::string& s);
	template<>
	double ToNumber(const std::string& s);

	template<>
	std::optional<bool> Parse<bool>(std::string_view s);
	template<>
	std::optional<float> Parse<float>(std::string_view s);
	template<>
	std::optional<double> Parse<double>(std::string_view s);

	template<>
	std::optional<std::string_view> Parse<std::string_view>(std::string_view s);
	template<>
	std::optional<std::string> Parse<std::string>(std::string_view s);
}

#endif //ENGINE_STRINGS_STRINGS_HPP
