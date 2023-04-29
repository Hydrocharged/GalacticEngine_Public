// Copyright © 2022-2023 Daylon Wilkins & James Cor
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifdef PLATFORM_WIN32

#include <engine/backend/windows.hpp>
#include <engine/strings/strings.hpp>

bool engine::Application::PlatformImplementation::InitializeWindow() {
	auto appOptions = application->StartOptions();
	std::wstring title = engine::strings::ToWide(appOptions.Title);
	// Register our window class
	WNDCLASSEX wcex = {sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW, Callbacks->MessageProcCallback,
					   0L, 0L, Data->InstanceHandle, nullptr, nullptr, nullptr,
					   nullptr, L"GalacticEngine", nullptr};
	RegisterClassEx(&wcex);

	// Create a window
	RECT rc = {0, 0, (LONG)appOptions.Width, (LONG)appOptions.Height};
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	Data->WindowHandle = CreateWindow(L"GalacticEngine", title.c_str(),
									  WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
									  rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, Data->InstanceHandle, nullptr);
	if (!Data->WindowHandle) {
		MessageBox(nullptr, L"Cannot create window", L"Error", MB_OK | MB_ICONERROR);
		return false;
	}
	ShowWindow(Data->WindowHandle, Data->ShowCmd);
	UpdateWindow(Data->WindowHandle);
	SetFocus(Data->WindowHandle);
	return true;
}

void* engine::Application::PlatformImplementation::GetWindowHandle() const {
	return Data->WindowHandle;
}

#endif //PLATFORM_WIN32
