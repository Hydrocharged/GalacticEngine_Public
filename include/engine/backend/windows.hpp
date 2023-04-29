// Copyright © 2022-2023 Daylon Wilkins & James Cor
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifdef PLATFORM_WIN32
#ifndef ENGINE_BACKEND_WINDOWS_HPP
#define ENGINE_BACKEND_WINDOWS_HPP
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <engine/backend/common.hpp>
#include <Windows.h>
#include <xinput.h>

typedef DWORD (WINAPI* PFN_XInputGetCapabilities)(DWORD, DWORD, XINPUT_CAPABILITIES*);
typedef DWORD (WINAPI* PFN_XInputGetState)(DWORD, XINPUT_STATE*);

class engine::Application::PlatformImplementation::PlatformCallbacks {
public:
	PlatformCallbacks(engine::Application::CommonImplementation* commonImpl);
	~PlatformCallbacks();

	static LRESULT CALLBACK MessageProcCallback(HWND, UINT, WPARAM, LPARAM);
	LRESULT CALLBACK MessageProc(HWND, UINT, WPARAM, LPARAM);
};

struct engine::Application::PlatformImplementation::PlatformData {
public:
	int ShowCmd;
	RECT ClientRect;
	engine::input::Mouse::CaptureState LastState = engine::input::Mouse::CaptureState::None;
	bool ReceivedMouseMove = false;

	HANDLE WaitTimer;
	int WaitSchedulerPeriodMs;
	INT64 WaitQpcPerSecond;

	HWND WindowHandle;
	HINSTANCE InstanceHandle;
	HMODULE XInputDLL;
	PFN_XInputGetCapabilities XInputGetCapabilities;
	PFN_XInputGetState XInputGetState;
	std::unique_ptr<engine::input::Gamepad::Setter> GamepadSetters[4] = {nullptr, nullptr, nullptr, nullptr};
	std::unique_ptr<engine::input::Mouse::Setter> MouseSetter;
	std::unique_ptr<engine::input::Keyboard::Setter> KeyboardSetter;
};

#endif //ENGINE_BACKEND_WINDOWS_HPP
#endif //PLATFORM_WIN32
