// Copyright © 2022-2023 Daylon Wilkins & James Cor
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifdef PLATFORM_WIN32

#include <engine/backend/windows.hpp>

// We must include the Win32 implementation since it's not included anywhere else.
#include <backends/imgui_impl_win32.cpp>

bool engine::Application::PlatformImplementation::ImGuiInitialize() {
	ImGui_ImplWin32_Init(Data->WindowHandle);
	return true;
}

bool engine::Application::PlatformImplementation::ImGuiNewFrame() {
	ImGui_ImplWin32_NewFrame();
	return true;
}

void engine::Application::PlatformImplementation::ImGuiShutdown() {
	ImGui_ImplWin32_Shutdown();
}

#endif //PLATFORM_WIN32
