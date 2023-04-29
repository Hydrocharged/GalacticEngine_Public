// Copyright © 2022-2023 Daylon Wilkins & James Cor
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifdef PLATFORM_WIN32

#include <engine/backend/windows.hpp>

static_assert(XUSER_MAX_COUNT == 4, "More/Less than 4 controllers are supported");

bool engine::Application::PlatformImplementation::InitializeInput() {
	const char* xinput_dll_names[] = {
		"xinput1_4.dll",   // Windows 8+
		"xinput1_3.dll",   // DirectX SDK
		"xinput9_1_0.dll", // Windows Vista, Windows 7
		"xinput1_2.dll",   // DirectX SDK
		"xinput1_1.dll"    // DirectX SDK
	};
	for (int n = 0; n < _countof(xinput_dll_names); ++n) {
		if (HMODULE dll = ::LoadLibraryA(xinput_dll_names[n])) {
			Data->XInputDLL = dll;
			Data->XInputGetCapabilities = (PFN_XInputGetCapabilities)::GetProcAddress(dll, "XInputGetCapabilities");
			Data->XInputGetState = (PFN_XInputGetState)::GetProcAddress(dll, "XInputGetState");
			break;
		}
	}

	for (DWORD i = 0; i < XUSER_MAX_COUNT; i++) {
		XINPUT_CAPABILITIES capabilities = {};
		if ((Data->XInputGetCapabilities(i, XINPUT_FLAG_GAMEPAD, &capabilities) == ERROR_SUCCESS)) {
			if (!Data->GamepadSetters[i]) {
				Data->GamepadSetters[i] = application->inputHandler->ConnectGamepad();
			}
		} else if (Data->GamepadSetters[i]) {
			application->inputHandler->DisconnectDevice(Data->GamepadSetters[i]->GetID());
			Data->GamepadSetters[i].reset(nullptr);
		}
	}

	Data->MouseSetter = application->inputHandler->ConnectMouse();
	Data->KeyboardSetter = application->inputHandler->ConnectKeyboard();
	return true;
}

void SetClientRect(RECT& rc, HWND windowHandle) {
	GetWindowRect(windowHandle, &rc);
	int borderSize = GetSystemMetrics(SM_CYSIZEFRAME) + 2 * GetSystemMetrics(SM_CYEDGE);
	rc.top += GetSystemMetrics(SM_CYCAPTION) + borderSize;
	rc.left += borderSize;
	rc.right -= borderSize;
	rc.bottom -= borderSize;
}

bool engine::Application::PlatformImplementation::UpdateInput() {
	auto elapsedTime = application->commonImpl->guiBackend->GetElapsedTime();
	for (int i = 0; i < _countof(Data->GamepadSetters); i++) {
		auto gamepadSetter = Data->GamepadSetters[i].get();
		if (gamepadSetter) {
			XINPUT_STATE state;
			ZeroMemory(&state, sizeof(XINPUT_STATE));

			if (Data->XInputGetState(i, &state) == ERROR_SUCCESS) {
				// Reference: https://learn.microsoft.com/en-us/windows/win32/api/xinput/ns-xinput-xinput_gamepad
				XINPUT_GAMEPAD gp = state.Gamepad;
				gamepadSetter->SetStick(engine::input::Gamepad::Stick::Left,
										(((float)gp.sThumbLX + 32768.0f) / 32767.5f) - 1.0f,
										(((float)gp.sThumbLY + 32768.0f) / 32767.5f) - 1.0f);
				gamepadSetter->SetStick(engine::input::Gamepad::Stick::Right,
										(((float)gp.sThumbRX + 32768.0f) / 32767.5f) - 1.0f,
										(((float)gp.sThumbRY + 32768.0f) / 32767.5f) - 1.0f);
				gamepadSetter->SetTrigger(engine::input::Gamepad::Trigger::Left, (float)gp.bLeftTrigger / 255.0f);
				gamepadSetter->SetTrigger(engine::input::Gamepad::Trigger::Right, (float)gp.bRightTrigger / 255.0f);
				gamepadSetter->SetButton(engine::input::Gamepad::Button::A, XINPUT_GAMEPAD_A & gp.wButtons, elapsedTime);
				gamepadSetter->SetButton(engine::input::Gamepad::Button::B, XINPUT_GAMEPAD_B & gp.wButtons, elapsedTime);
				gamepadSetter->SetButton(engine::input::Gamepad::Button::X, XINPUT_GAMEPAD_X & gp.wButtons, elapsedTime);
				gamepadSetter->SetButton(engine::input::Gamepad::Button::Y, XINPUT_GAMEPAD_Y & gp.wButtons, elapsedTime);
				gamepadSetter->SetButton(engine::input::Gamepad::Button::LShoulder, XINPUT_GAMEPAD_LEFT_SHOULDER & gp.wButtons, elapsedTime);
				gamepadSetter->SetButton(engine::input::Gamepad::Button::LStick, XINPUT_GAMEPAD_LEFT_THUMB & gp.wButtons, elapsedTime);
				gamepadSetter->SetButton(engine::input::Gamepad::Button::RShoulder, XINPUT_GAMEPAD_RIGHT_SHOULDER & gp.wButtons, elapsedTime);
				gamepadSetter->SetButton(engine::input::Gamepad::Button::RStick, XINPUT_GAMEPAD_RIGHT_THUMB & gp.wButtons, elapsedTime);
				gamepadSetter->SetButton(engine::input::Gamepad::Button::DPadLeft, XINPUT_GAMEPAD_DPAD_LEFT & gp.wButtons, elapsedTime);
				gamepadSetter->SetButton(engine::input::Gamepad::Button::DPadRight, XINPUT_GAMEPAD_DPAD_RIGHT & gp.wButtons, elapsedTime);
				gamepadSetter->SetButton(engine::input::Gamepad::Button::DPadUp, XINPUT_GAMEPAD_DPAD_UP & gp.wButtons, elapsedTime);
				gamepadSetter->SetButton(engine::input::Gamepad::Button::DPadDown, XINPUT_GAMEPAD_DPAD_DOWN & gp.wButtons, elapsedTime);
				gamepadSetter->SetButton(engine::input::Gamepad::Button::Start, XINPUT_GAMEPAD_START & gp.wButtons, elapsedTime);
				gamepadSetter->SetButton(engine::input::Gamepad::Button::Options, XINPUT_GAMEPAD_BACK & gp.wButtons, elapsedTime);
			}
		}
	}

	if (!Data->ReceivedMouseMove) {
		Data->MouseSetter->SetDelta(0, 0);
	}
	Data->ReceivedMouseMove = false;

	auto mouseCaptureState = application->inputHandler->GetMouseCaptureState();
	if (mouseCaptureState == engine::input::Mouse::CaptureState::Hard) {
		const RECT& rc = Data->ClientRect;
		SetCursorPos((rc.right - rc.left) / 2 + rc.left, (rc.bottom - rc.top) / 2 + rc.top);
	}
	return true;
}

void engine::input::Handler::SetMouseCaptureState(Mouse::CaptureState state) {
	if (mouseCaptureState == state) {
		return;
	}
	if (state == engine::input::Mouse::CaptureState::Hard && mouseCaptureState != engine::input::Mouse::CaptureState::Hard) {
		ShowCursor(false);
	} else if (state != engine::input::Mouse::CaptureState::Hard && mouseCaptureState == engine::input::Mouse::CaptureState::Hard) {
		ShowCursor(true);
	}
	mouseCaptureState = state;

	if (state == engine::input::Mouse::CaptureState::None) {
		ClipCursor(nullptr);
		return;
	}

	SetClientRect(application->platImpl->Data->ClientRect, application->platImpl->Data->WindowHandle);
	ClipCursor(&application->platImpl->Data->ClientRect);
}

#endif //PLATFORM_WIN32
