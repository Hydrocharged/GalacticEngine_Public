// Copyright © 2022-2023 Daylon Wilkins & James Cor
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifdef PLATFORM_WIN32

#include <engine/backend/windows.hpp>
#include <windowsx.h>

engine::Application::PlatformImplementation::PlatformCallbacks* callbackImpl = nullptr;
engine::Application::CommonImplementation* commonImpl = nullptr;
Rml::Context* rmlContext = nullptr;
engine::input::Mouse::Setter* mouseSetter = nullptr;
engine::input::Keyboard::Setter* keyboardSetter = nullptr;

// This is defined by the "backend/windows/imgui.cpp" file, so we put an external reference here
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

inline bool IsVkDown(WPARAM vk) {
	return (::GetKeyState((int)vk) & 0x8000) != 0;
}

engine::input::Keyboard::Key ToInputKey(WPARAM vk) {
	switch (vk) {
		case VK_TAB:
			return engine::input::Keyboard::Key::Tab;
		case VK_LEFT:
			return engine::input::Keyboard::Key::ArrowLeft;
		case VK_RIGHT:
			return engine::input::Keyboard::Key::ArrowRight;
		case VK_UP:
			return engine::input::Keyboard::Key::ArrowUp;
		case VK_DOWN:
			return engine::input::Keyboard::Key::ArrowDown;
		case VK_PRIOR:
			return engine::input::Keyboard::Key::PageUp;
		case VK_NEXT:
			return engine::input::Keyboard::Key::PageDown;
		case VK_HOME:
			return engine::input::Keyboard::Key::Home;
		case VK_END:
			return engine::input::Keyboard::Key::End;
		case VK_INSERT:
			return engine::input::Keyboard::Key::Insert;
		case VK_DELETE:
			return engine::input::Keyboard::Key::Delete;
		case VK_BACK:
			return engine::input::Keyboard::Key::Backspace;
		case VK_SPACE:
			return engine::input::Keyboard::Key::Space;
		case VK_RETURN:
			return engine::input::Keyboard::Key::Enter;
		case VK_ESCAPE:
			return engine::input::Keyboard::Key::Escape;
		case VK_OEM_7:
			return engine::input::Keyboard::Key::Quote;
		case VK_OEM_COMMA:
			return engine::input::Keyboard::Key::Comma;
		case VK_OEM_MINUS:
			return engine::input::Keyboard::Key::Dash;
		case VK_OEM_PERIOD:
			return engine::input::Keyboard::Key::Period;
		case VK_OEM_2:
			return engine::input::Keyboard::Key::ForwardSlash;
		case VK_OEM_1:
			return engine::input::Keyboard::Key::Semicolon;
		case VK_OEM_PLUS:
			return engine::input::Keyboard::Key::Equals;
		case VK_OEM_4:
			return engine::input::Keyboard::Key::LeftBrace;
		case VK_OEM_5:
			return engine::input::Keyboard::Key::Backslash;
		case VK_OEM_6:
			return engine::input::Keyboard::Key::RightBrace;
		case VK_OEM_3:
			return engine::input::Keyboard::Key::Backtick;
		case VK_CAPITAL:
			return engine::input::Keyboard::Key::CapsLock;
		case VK_SCROLL:
			return engine::input::Keyboard::Key::ScrollLock;
		case VK_NUMLOCK:
			return engine::input::Keyboard::Key::NumLock;
		case VK_SNAPSHOT:
			return engine::input::Keyboard::Key::PrintScreen;
		case VK_PAUSE:
			return engine::input::Keyboard::Key::PauseBreak;
		case VK_LSHIFT:
			return engine::input::Keyboard::Key::LeftShift;
		case VK_LCONTROL:
			return engine::input::Keyboard::Key::LeftCtrl;
		case VK_LMENU:
			return engine::input::Keyboard::Key::LeftAlt;
		case VK_RSHIFT:
			return engine::input::Keyboard::Key::RightShift;
		case VK_RCONTROL:
			return engine::input::Keyboard::Key::RightCtrl;
		case VK_RMENU:
			return engine::input::Keyboard::Key::RightAlt;
		case '0':
			return engine::input::Keyboard::Key::Number0;
		case '1':
			return engine::input::Keyboard::Key::Number1;
		case '2':
			return engine::input::Keyboard::Key::Number2;
		case '3':
			return engine::input::Keyboard::Key::Number3;
		case '4':
			return engine::input::Keyboard::Key::Number4;
		case '5':
			return engine::input::Keyboard::Key::Number5;
		case '6':
			return engine::input::Keyboard::Key::Number6;
		case '7':
			return engine::input::Keyboard::Key::Number7;
		case '8':
			return engine::input::Keyboard::Key::Number8;
		case '9':
			return engine::input::Keyboard::Key::Number9;
		case 'A':
			return engine::input::Keyboard::Key::A;
		case 'B':
			return engine::input::Keyboard::Key::B;
		case 'C':
			return engine::input::Keyboard::Key::C;
		case 'D':
			return engine::input::Keyboard::Key::D;
		case 'E':
			return engine::input::Keyboard::Key::E;
		case 'F':
			return engine::input::Keyboard::Key::F;
		case 'G':
			return engine::input::Keyboard::Key::G;
		case 'H':
			return engine::input::Keyboard::Key::H;
		case 'I':
			return engine::input::Keyboard::Key::I;
		case 'J':
			return engine::input::Keyboard::Key::J;
		case 'K':
			return engine::input::Keyboard::Key::K;
		case 'L':
			return engine::input::Keyboard::Key::L;
		case 'M':
			return engine::input::Keyboard::Key::M;
		case 'N':
			return engine::input::Keyboard::Key::N;
		case 'O':
			return engine::input::Keyboard::Key::O;
		case 'P':
			return engine::input::Keyboard::Key::P;
		case 'Q':
			return engine::input::Keyboard::Key::Q;
		case 'R':
			return engine::input::Keyboard::Key::R;
		case 'S':
			return engine::input::Keyboard::Key::S;
		case 'T':
			return engine::input::Keyboard::Key::T;
		case 'U':
			return engine::input::Keyboard::Key::U;
		case 'V':
			return engine::input::Keyboard::Key::V;
		case 'W':
			return engine::input::Keyboard::Key::W;
		case 'X':
			return engine::input::Keyboard::Key::X;
		case 'Y':
			return engine::input::Keyboard::Key::Y;
		case 'Z':
			return engine::input::Keyboard::Key::Z;
		case VK_F1:
			return engine::input::Keyboard::Key::F1;
		case VK_F2:
			return engine::input::Keyboard::Key::F2;
		case VK_F3:
			return engine::input::Keyboard::Key::F3;
		case VK_F4:
			return engine::input::Keyboard::Key::F4;
		case VK_F5:
			return engine::input::Keyboard::Key::F5;
		case VK_F6:
			return engine::input::Keyboard::Key::F6;
		case VK_F7:
			return engine::input::Keyboard::Key::F7;
		case VK_F8:
			return engine::input::Keyboard::Key::F8;
		case VK_F9:
			return engine::input::Keyboard::Key::F9;
		case VK_F10:
			return engine::input::Keyboard::Key::F10;
		case VK_F11:
			return engine::input::Keyboard::Key::F11;
		case VK_F12:
		default:
			return engine::input::Keyboard::Key::F12;
	}
}

Rml::Input::KeyIdentifier ToRmlKey(WPARAM vk) {
	switch (vk) {
		case VK_TAB:
			return Rml::Input::KI_TAB;
		case VK_LEFT:
			return Rml::Input::KI_LEFT;
		case VK_RIGHT:
			return Rml::Input::KI_RIGHT;
		case VK_UP:
			return Rml::Input::KI_UP;
		case VK_DOWN:
			return Rml::Input::KI_DOWN;
		case VK_PRIOR:
			return Rml::Input::KI_PRIOR;
		case VK_NEXT:
			return Rml::Input::KI_NEXT;
		case VK_HOME:
			return Rml::Input::KI_HOME;
		case VK_END:
			return Rml::Input::KI_END;
		case VK_INSERT:
			return Rml::Input::KI_INSERT;
		case VK_DELETE:
			return Rml::Input::KI_DELETE;
		case VK_BACK:
			return Rml::Input::KI_BACK;
		case VK_SPACE:
			return Rml::Input::KI_SPACE;
		case VK_RETURN:
			return Rml::Input::KI_RETURN;
		case VK_ESCAPE:
			return Rml::Input::KI_ESCAPE;
		case VK_OEM_7:
			return Rml::Input::KI_OEM_7;
		case VK_OEM_COMMA:
			return Rml::Input::KI_OEM_COMMA;
		case VK_OEM_MINUS:
			return Rml::Input::KI_OEM_MINUS;
		case VK_OEM_PERIOD:
			return Rml::Input::KI_OEM_PERIOD;
		case VK_OEM_2:
			return Rml::Input::KI_OEM_2;
		case VK_OEM_1:
			return Rml::Input::KI_OEM_1;
		case VK_OEM_PLUS:
			return Rml::Input::KI_OEM_PLUS;
		case VK_OEM_4:
			return Rml::Input::KI_OEM_4;
		case VK_OEM_5:
			return Rml::Input::KI_OEM_5;
		case VK_OEM_6:
			return Rml::Input::KI_OEM_6;
		case VK_OEM_3:
			return Rml::Input::KI_OEM_3;
		case VK_CAPITAL:
			return Rml::Input::KI_CAPITAL;
		case VK_SCROLL:
			return Rml::Input::KI_SCROLL;
		case VK_NUMLOCK:
			return Rml::Input::KI_NUMLOCK;
		case VK_SNAPSHOT:
			return Rml::Input::KI_SNAPSHOT;
		case VK_PAUSE:
			return Rml::Input::KI_PAUSE;
		case VK_LSHIFT:
			return Rml::Input::KI_LSHIFT;
		case VK_LCONTROL:
			return Rml::Input::KI_LCONTROL;
		case VK_LMENU:
			return Rml::Input::KI_LMENU;
		case VK_RSHIFT:
			return Rml::Input::KI_RSHIFT;
		case VK_RCONTROL:
			return Rml::Input::KI_RCONTROL;
		case VK_RMENU:
			return Rml::Input::KI_RMENU;
		case '0':
			return Rml::Input::KI_0;
		case '1':
			return Rml::Input::KI_1;
		case '2':
			return Rml::Input::KI_2;
		case '3':
			return Rml::Input::KI_3;
		case '4':
			return Rml::Input::KI_4;
		case '5':
			return Rml::Input::KI_5;
		case '6':
			return Rml::Input::KI_6;
		case '7':
			return Rml::Input::KI_7;
		case '8':
			return Rml::Input::KI_8;
		case '9':
			return Rml::Input::KI_9;
		case 'A':
			return Rml::Input::KI_A;
		case 'B':
			return Rml::Input::KI_B;
		case 'C':
			return Rml::Input::KI_C;
		case 'D':
			return Rml::Input::KI_D;
		case 'E':
			return Rml::Input::KI_E;
		case 'F':
			return Rml::Input::KI_F;
		case 'G':
			return Rml::Input::KI_G;
		case 'H':
			return Rml::Input::KI_H;
		case 'I':
			return Rml::Input::KI_I;
		case 'J':
			return Rml::Input::KI_J;
		case 'K':
			return Rml::Input::KI_K;
		case 'L':
			return Rml::Input::KI_L;
		case 'M':
			return Rml::Input::KI_M;
		case 'N':
			return Rml::Input::KI_N;
		case 'O':
			return Rml::Input::KI_O;
		case 'P':
			return Rml::Input::KI_P;
		case 'Q':
			return Rml::Input::KI_Q;
		case 'R':
			return Rml::Input::KI_R;
		case 'S':
			return Rml::Input::KI_S;
		case 'T':
			return Rml::Input::KI_T;
		case 'U':
			return Rml::Input::KI_U;
		case 'V':
			return Rml::Input::KI_V;
		case 'W':
			return Rml::Input::KI_W;
		case 'X':
			return Rml::Input::KI_X;
		case 'Y':
			return Rml::Input::KI_Y;
		case 'Z':
			return Rml::Input::KI_Z;
		case VK_F1:
			return Rml::Input::KI_F1;
		case VK_F2:
			return Rml::Input::KI_F2;
		case VK_F3:
			return Rml::Input::KI_F3;
		case VK_F4:
			return Rml::Input::KI_F4;
		case VK_F5:
			return Rml::Input::KI_F5;
		case VK_F6:
			return Rml::Input::KI_F6;
		case VK_F7:
			return Rml::Input::KI_F7;
		case VK_F8:
			return Rml::Input::KI_F8;
		case VK_F9:
			return Rml::Input::KI_F9;
		case VK_F10:
			return Rml::Input::KI_F10;
		case VK_F11:
			return Rml::Input::KI_F11;
		case VK_F12:
			return Rml::Input::KI_F12;
		default:
			return Rml::Input::KI_F24;
	}
}

void SetRmlModifier(int& modifier, WPARAM vk, bool isDown) {
	if (isDown) {
		switch (vk) {
			case VK_SHIFT:
				modifier |= Rml::Input::KeyModifier::KM_SHIFT;
				return;
			case VK_CONTROL:
				modifier |= Rml::Input::KeyModifier::KM_CTRL;
				return;
			case VK_MENU:
				modifier |= Rml::Input::KeyModifier::KM_ALT;
				return;
			case VK_CAPITAL:
				modifier |= Rml::Input::KeyModifier::KM_CAPSLOCK;
				return;
			case VK_SCROLL:
				modifier |= Rml::Input::KeyModifier::KM_SCROLLLOCK;
				return;
			case VK_NUMLOCK:
				modifier |= Rml::Input::KeyModifier::KM_NUMLOCK;
				return;
			default:
				return;
		}
	} else {
		switch (vk) {
			case VK_SHIFT:
				modifier &= ~Rml::Input::KeyModifier::KM_SHIFT;
				return;
			case VK_CONTROL:
				modifier &= ~Rml::Input::KeyModifier::KM_CTRL;
				return;
			case VK_MENU:
				modifier &= ~Rml::Input::KeyModifier::KM_ALT;
				return;
			case VK_CAPITAL:
				modifier &= ~Rml::Input::KeyModifier::KM_CAPSLOCK;
				return;
			case VK_SCROLL:
				modifier &= ~Rml::Input::KeyModifier::KM_SCROLLLOCK;
				return;
			case VK_NUMLOCK:
				modifier &= ~Rml::Input::KeyModifier::KM_NUMLOCK;
				return;
			default:
				return;
		}
	}
}

engine::Application::PlatformImplementation::PlatformCallbacks::PlatformCallbacks(engine::Application::CommonImplementation* commonImpl) {
	::commonImpl = commonImpl;
	::callbackImpl = this;
}

engine::Application::PlatformImplementation::PlatformCallbacks::~PlatformCallbacks() = default;

LRESULT CALLBACK engine::Application::PlatformImplementation::PlatformCallbacks::MessageProcCallback(HWND wnd, UINT message, WPARAM wParam, LPARAM lParam) {
	return callbackImpl->MessageProc(wnd, message, wParam, lParam);
}

LRESULT Win32ImGuiHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (ImGui::GetCurrentContext() == nullptr) {
		return 0;
	}
	auto res = ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam);
	const ImGuiIO& io = ImGui::GetIO();
	switch (msg) {
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONDBLCLK:
		case WM_XBUTTONDOWN:
		case WM_XBUTTONDBLCLK:
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		case WM_XBUTTONUP:
		case WM_MOUSEWHEEL:
		case WM_MOUSEHWHEEL:
			return io.WantCaptureMouse ? 1 : 0;
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYUP:
		case WM_CHAR:
			return io.WantCaptureKeyboard ? 1 : 0;
		default:
			return res;
	}
}

LRESULT CALLBACK engine::Application::PlatformImplementation::PlatformCallbacks::MessageProc(HWND wnd, UINT message, WPARAM wParam, LPARAM lParam) {
	if (Win32ImGuiHandler(wnd, message, wParam, lParam)) {
		return 0;
	}
	static bool isTrackingMouse = false;
	static int rmlModifiers = 0;

	switch (message) {
		case WM_PAINT: {
			PAINTSTRUCT ps;
			BeginPaint(wnd, &ps);
			EndPaint(wnd, &ps);
			return 0;
		}
		case WM_SIZE: // Window size has been changed
			::commonImpl->HandleResize(LOWORD(lParam), HIWORD(lParam));
			return 0;

		case WM_CLOSE:
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		case WM_GETMINMAXINFO: {
			LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
			lpMMI->ptMinTrackSize.x = engine::graphics::MinimumWindowWidth;
			lpMMI->ptMinTrackSize.y = engine::graphics::MinimumWindowHeight;
			return 0;
		}
		case WM_DEVICECHANGE: {
			auto data = ::commonImpl->application->platImpl->Data.get();
			if (data->XInputGetCapabilities) {
				for (DWORD i = 0; i < XUSER_MAX_COUNT; i++) {
					XINPUT_CAPABILITIES capabilities = {};
					if ((data->XInputGetCapabilities(i, XINPUT_FLAG_GAMEPAD, &capabilities) == ERROR_SUCCESS)) {
						if (!data->GamepadSetters[i]) {
							data->GamepadSetters[i] = ::commonImpl->application->inputHandler->ConnectGamepad();
						}
					} else if (data->GamepadSetters[i]) {
						::commonImpl->application->inputHandler->DisconnectDevice(data->GamepadSetters[i]->GetID());
						data->GamepadSetters[i].reset(nullptr);
					}
				}
			}
		}
		case WM_MOUSEMOVE: {
			// Must call TrackMouseEvent to receive WM_MOUSELEAVE events
			::commonImpl->application->platImpl->Data->ReceivedMouseMove = true;
			if (!isTrackingMouse) {
				TRACKMOUSEEVENT tme = {
					.cbSize = sizeof(TRACKMOUSEEVENT),
					.dwFlags = TME_LEAVE,
					.hwndTrack = ::commonImpl->application->platImpl->Data->WindowHandle,
					.dwHoverTime = 0};
				::TrackMouseEvent(&tme);
				isTrackingMouse = true;
			}
			int mouseX = GET_X_LPARAM(lParam);
			int mouseY = GET_Y_LPARAM(lParam);
			if (::rmlContext && ::rmlContext->ProcessMouseMove(mouseX, mouseY, rmlModifiers)) {
				::mouseSetter->SetPosition((float)mouseX, (float)mouseY);
				if (::commonImpl->application->inputHandler->GetMouseCaptureState() == engine::input::Mouse::CaptureState::Hard) {
					auto dX = float(mouseX - int(engine::graphics::GetWindowWidth() / 2));
					auto dY = float(mouseY - int(engine::graphics::GetWindowHeight() / 2));
					::mouseSetter->SetDelta(dX, dY);
				}
			}
			return 0;
		}
		case WM_MOUSELEAVE:
			isTrackingMouse = false;
			if (::rmlContext && !::rmlContext->ProcessMouseLeave()) {
				return 0;
			}
			return DefWindowProc(wnd, message, wParam, lParam);
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
			if (::rmlContext && !::rmlContext->ProcessMouseButtonDown(0, rmlModifiers)) {
				return 0;
			}
			::mouseSetter->SetButton(engine::input::Mouse::Button::Left, true, ::commonImpl->guiBackend->GetElapsedTime());
			return 0;
		case WM_RBUTTONDOWN:
		case WM_RBUTTONDBLCLK:
			if (::rmlContext && !::rmlContext->ProcessMouseButtonDown(1, rmlModifiers)) {
				return 0;
			}
			::mouseSetter->SetButton(engine::input::Mouse::Button::Right, true, ::commonImpl->guiBackend->GetElapsedTime());
			return 0;
		case WM_MBUTTONDOWN:
		case WM_MBUTTONDBLCLK:
			if (::rmlContext && !::rmlContext->ProcessMouseButtonDown(2, rmlModifiers)) {
				return 0;
			}
			::mouseSetter->SetButton(engine::input::Mouse::Button::Middle, true, ::commonImpl->guiBackend->GetElapsedTime());
			return 0;
		case WM_LBUTTONUP:
			if (::rmlContext && !::rmlContext->ProcessMouseButtonUp(0, rmlModifiers)) {
				return 0;
			}
			::mouseSetter->SetButton(engine::input::Mouse::Button::Left, false, ::commonImpl->guiBackend->GetElapsedTime());
			return 0;
		case WM_RBUTTONUP:
			if (::rmlContext && !::rmlContext->ProcessMouseButtonUp(1, rmlModifiers)) {
				return 0;
			}
			::mouseSetter->SetButton(engine::input::Mouse::Button::Right, false, ::commonImpl->guiBackend->GetElapsedTime());
			return 0;
		case WM_MBUTTONUP:
			if (::rmlContext && !::rmlContext->ProcessMouseButtonUp(2, rmlModifiers)) {
				return 0;
			}
			::mouseSetter->SetButton(engine::input::Mouse::Button::Middle, false, ::commonImpl->guiBackend->GetElapsedTime());
			return 0;
		case WM_MOUSEWHEEL:
		case WM_MOUSEHWHEEL: {
			float wheelDelta = -((float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA);
			if (::rmlContext && !::rmlContext->ProcessMouseWheel(wheelDelta, rmlModifiers)) {
				return 0;
			}
			::mouseSetter->SetScrollWheel(wheelDelta);
			return 0;
		}
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP: {
			bool isDown = (message == WM_KEYDOWN || message == WM_SYSKEYDOWN);
			if (wParam < 256) {
				int vk = (int)wParam;
				SetRmlModifier(rmlModifiers, vk, isDown);
				// We have to manually handle ALT+F4 as it's not being sent as a message for some reason
				if (vk == VK_F4 && (rmlModifiers & Rml::Input::KeyModifier::KM_ALT) == Rml::Input::KeyModifier::KM_ALT && isDown) {
					PostQuitMessage(0);
					return 0;
				}

				if (isDown && !::rmlContext->ProcessKeyDown(ToRmlKey(vk), rmlModifiers)) {
					return 0;
				} else if (!isDown && !::rmlContext->ProcessKeyUp(ToRmlKey(vk), rmlModifiers)) {
					return 0;
				}

				// Submit individual left/right modifier events
				if (vk == VK_SHIFT) {
					// Important: Shift keys tend to get stuck when pressed together, missing key-up events are corrected in ImGui_ImplWin32_ProcessKeyEventsWorkarounds()
					if (IsVkDown(VK_LSHIFT) == isDown) {
						::keyboardSetter->SetKey(engine::input::Keyboard::Key::LeftShift, isDown, ::commonImpl->guiBackend->GetElapsedTime());
					}
					if (IsVkDown(VK_RSHIFT) == isDown) {
						::keyboardSetter->SetKey(engine::input::Keyboard::Key::RightShift, isDown, ::commonImpl->guiBackend->GetElapsedTime());
					}
				} else if (vk == VK_CONTROL) {
					if (IsVkDown(VK_LCONTROL) == isDown) {
						::keyboardSetter->SetKey(engine::input::Keyboard::Key::LeftCtrl, isDown, ::commonImpl->guiBackend->GetElapsedTime());
					}
					if (IsVkDown(VK_RCONTROL) == isDown) {
						::keyboardSetter->SetKey(engine::input::Keyboard::Key::RightCtrl, isDown, ::commonImpl->guiBackend->GetElapsedTime());
					}
				} else if (vk == VK_MENU) {
					if (IsVkDown(VK_LMENU) == isDown) {
						::keyboardSetter->SetKey(engine::input::Keyboard::Key::LeftAlt, isDown, ::commonImpl->guiBackend->GetElapsedTime());
					}
					if (IsVkDown(VK_RMENU) == isDown) {
						::keyboardSetter->SetKey(engine::input::Keyboard::Key::RightAlt, isDown, ::commonImpl->guiBackend->GetElapsedTime());
					}
				} else {
					::keyboardSetter->SetKey(ToInputKey(vk), isDown, ::commonImpl->guiBackend->GetElapsedTime());
				}
			}
			return 0;
		}
		case WM_CHAR: {
			if (((rmlModifiers & (Rml::Input::KeyModifier::KM_CTRL | Rml::Input::KeyModifier::KM_ALT))) == 0) {
				if (::IsWindowUnicode(::commonImpl->application->platImpl->Data->WindowHandle)) {
					if (wParam > 0 && wParam < 0x10000 &&
						wParam != 0x00000008 /*Backspace*/ && wParam != 0x0000001b /*Escape*/ &&
						!::rmlContext->ProcessTextInput((Rml::Character)wParam)) {
						return 0;
					}
				} else {
					wchar_t wch = 0;
					::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (char*)&wParam, 1, &wch, 1);
					if (wch != 0x00000008 /*Backspace*/ && wch != 0x0000001b /*Escape*/ && !::rmlContext->ProcessTextInput((Rml::Character)wch)) {
						return 0;
					}
				}
			}
			return DefWindowProc(wnd, message, wParam, lParam);
		}
		case WM_SETFOCUS: {
			::commonImpl->application->inputHandler->SetMouseCaptureState(::commonImpl->application->platImpl->Data->LastState);
			return DefWindowProc(wnd, message, wParam, lParam);
		}
		case WM_KILLFOCUS: {
			::commonImpl->application->platImpl->Data->LastState = ::commonImpl->application->inputHandler->GetMouseCaptureState();
			::commonImpl->application->inputHandler->SetMouseCaptureState(engine::input::Mouse::CaptureState::None);
			return DefWindowProc(wnd, message, wParam, lParam);
		}
		default:
			return DefWindowProc(wnd, message, wParam, lParam);
	}
}

bool engine::Application::PlatformImplementation::ProcessMessages() {
	if (::rmlContext == nullptr) {
		::rmlContext = ::commonImpl->application->rmlContext;
		::mouseSetter = Data->MouseSetter.get();
		::keyboardSetter = Data->KeyboardSetter.get();
	}
	MSG msg = {nullptr};
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (WM_QUIT == msg.message) {
			return false;
		}
	}
	return WM_QUIT != msg.message;
}

#endif //PLATFORM_WIN32
