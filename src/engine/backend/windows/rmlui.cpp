// Copyright © 2022-2023 Daylon Wilkins & James Cor
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifdef PLATFORM_WIN32

#include <engine/backend/windows.hpp>
#include <engine/strings/strings.hpp>

class engine::gui::Backend::PlatformImplementation {
public:
	HWND hwnd = nullptr;
	double frequency = {};
	LARGE_INTEGER startup = {};
	HCURSOR cursorDefault = nullptr;
	HCURSOR cursorMove = nullptr;
	HCURSOR cursorPointer = nullptr;
	HCURSOR cursorResize = nullptr;
	HCURSOR cursorCross = nullptr;
	HCURSOR cursorText = nullptr;
	HCURSOR cursorUnavailable = nullptr;
};

engine::gui::Backend::Backend() = default;

engine::gui::Backend::~Backend() = default;

void engine::gui::Backend::Initialize(void* data) {
	platImpl = std::make_unique<engine::gui::Backend::PlatformImplementation>();
	platImpl->hwnd = *reinterpret_cast<HWND*>(data);

	LARGE_INTEGER ticksPerSecond;
	QueryPerformanceFrequency(&ticksPerSecond);
	QueryPerformanceCounter(&platImpl->startup);

	platImpl->frequency = 1.0 / (double)ticksPerSecond.QuadPart;

	// Load cursors
	platImpl->cursorDefault = LoadCursor(nullptr, IDC_ARROW);
	platImpl->cursorMove = LoadCursor(nullptr, IDC_SIZEALL);
	platImpl->cursorPointer = LoadCursor(nullptr, IDC_HAND);
	platImpl->cursorResize = LoadCursor(nullptr, IDC_SIZENWSE);
	platImpl->cursorCross = LoadCursor(nullptr, IDC_CROSS);
	platImpl->cursorText = LoadCursor(nullptr, IDC_IBEAM);
	platImpl->cursorUnavailable = LoadCursor(nullptr, IDC_NO);
}

double engine::gui::Backend::GetElapsedTime() {
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	return double(counter.QuadPart - platImpl->startup.QuadPart) * platImpl->frequency;
}

void engine::gui::Backend::SetMouseCursor(const Rml::String& cursorName) {
	//TODO: figure out why this doesn't seem to be working
	if (platImpl->hwnd) {
		HCURSOR cursorHandle = nullptr;
		if (cursorName.empty() || cursorName == "arrow") {
			cursorHandle = platImpl->cursorDefault;
		} else if (cursorName == "move") {
			cursorHandle = platImpl->cursorMove;
		} else if (cursorName == "pointer") {
			cursorHandle = platImpl->cursorPointer;
		} else if (cursorName == "resize") {
			cursorHandle = platImpl->cursorResize;
		} else if (cursorName == "cross") {
			cursorHandle = platImpl->cursorCross;
		} else if (cursorName == "text") {
			cursorHandle = platImpl->cursorText;
		} else if (cursorName == "unavailable") {
			cursorHandle = platImpl->cursorUnavailable;
		}

		if (cursorHandle) {
			SetCursor(cursorHandle);
			SetClassLongPtrA(platImpl->hwnd, GCLP_HCURSOR, (LONG_PTR)cursorHandle);
		}
	}
}

void engine::gui::Backend::SetClipboardText(const Rml::String& textUtf8) {
	if (platImpl->hwnd) {
		if (!OpenClipboard(platImpl->hwnd)) {
			return;
		}
		EmptyClipboard();
		const std::wstring text = engine::strings::ToWide(textUtf8.c_str());
		const size_t size = sizeof(wchar_t) * (text.size() + 1);
		HGLOBAL clipboardData = GlobalAlloc(GMEM_FIXED, size);
		memcpy(clipboardData, text.data(), size);
		if (SetClipboardData(CF_UNICODETEXT, clipboardData) == nullptr) {
			CloseClipboard();
			GlobalFree(clipboardData);
		} else {
			CloseClipboard();
		}
	}
}

void engine::gui::Backend::GetClipboardText(Rml::String& text) {
	if (platImpl->hwnd) {
		if (!OpenClipboard(platImpl->hwnd)) {
			return;
		}
		HANDLE clipboardData = GetClipboardData(CF_UNICODETEXT);
		if (clipboardData == nullptr) {
			CloseClipboard();
			return;
		}
		auto clipboardText = static_cast<const wchar_t*>(GlobalLock(clipboardData));
		if (clipboardText) {
			text = engine::strings::FromWide(clipboardText);
		}
		GlobalUnlock(clipboardData);
		CloseClipboard();
	}
}

void engine::gui::Backend::ActivateKeyboard(Rml::Vector2f caretPosition, float lineHeight) {}

bool engine::Application::PlatformImplementation::SetGUIBackend() {
	application->commonImpl->guiBackend->Initialize(&Data->WindowHandle);
	application->commonImpl->guiRenderer->Initialize(application);
	Rml::SetSystemInterface(application->commonImpl->guiBackend.get());
	Rml::SetRenderInterface(application->commonImpl->guiRenderer.get());
	return true;
}

#endif //PLATFORM_WIN32
