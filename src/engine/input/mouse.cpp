// Copyright © 2022-2023 Daylon Wilkins & James Cor
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <engine/input/input.hpp>
#include <engine/log/log.hpp>
#include <algorithm>

// Setter --------------------------------------------------------------------------------------------------------------

engine::input::Mouse::Setter::Setter(std::shared_ptr<Mouse> mouse) {
	this->mouse = std::move(mouse);
}

engine::input::Mouse::Setter::~Setter() = default;

std::shared_ptr<engine::input::Mouse> engine::input::Mouse::Setter::GetMouse() {
	return mouse;
}

engine::input::Device::ID engine::input::Mouse::Setter::GetID() const {
	return mouse->GetID();
}

void engine::input::Mouse::Setter::SetButton(Button button, bool isDown, double currentTime) {
	buttonState& state = mouse->buttons[static_cast<std::underlying_type<Button>::type>(button)];
	if (state.state.IsDown != isDown) {
		if (!isDown) {
			mouse->pressedButtons.push_back(button);
		}
		state.state.IsDown = isDown;
		state.state.StartTime = currentTime;
		for (const auto& callback: state.callbacks) {
			callback(button, state.state);
		}
	}
}

void engine::input::Mouse::Setter::SetDelta(float deltaX, float deltaY) {
	mouse->dX = deltaX;
	mouse->dY = deltaY;
}

void engine::input::Mouse::Setter::SetPosition(float x, float y) {
	SetDelta(x - mouse->posX, y - mouse->posY);
	mouse->posX = x;
	mouse->posY = y;
}

void engine::input::Mouse::Setter::SetScrollWheel(float value) {
	if (std::abs(value) > mouse->scrollWheel.Deadzone) {
		mouse->scrollWheel.Y = value - ((float)((value > 0) - (value < 0)) * mouse->scrollWheel.Deadzone);
	} else {
		mouse->scrollWheel.Y = 0.f;
	}
}

// Mouse ---------------------------------------------------------------------------------------------------------------

engine::input::Mouse::Mouse(Handler* handler, std::uint16_t id) : Device(handler, Device::Type::Mouse, id) {
	buttons.resize(3);
	std::fill(buttons.begin(), buttons.end(), Mouse::buttonState{});
}

engine::input::Mouse::~Mouse() = default;

engine::input::KeyState engine::input::Mouse::GetButton(Button button) const {
	return buttons[static_cast<std::underlying_type<Button>::type>(button)].state;
}

bool engine::input::Mouse::IsButtonDown(Button button) const {
	return buttons[static_cast<std::underlying_type<Button>::type>(button)].state.IsDown;
}

bool engine::input::Mouse::IsButtonUp(Button button) const {
	return !buttons[static_cast<std::underlying_type<Button>::type>(button)].state.IsDown;
}

bool engine::input::Mouse::IsButtonPressed(Button button) const {
	for (const auto& pressedButton: pressedButtons) {
		if (button == pressedButton) {
			return true;
		}
	}
	return false;
}

void engine::input::Mouse::GetPosition(float& x, float& y) const {
	x = posX;
	y = posY;
}

void engine::input::Mouse::GetDelta(float& deltaX, float& deltaY) const {
	deltaX = dX;
	deltaY = dY;
}

float engine::input::Mouse::GetScrollWheel() const {
	return scrollWheel.Y;
}

void engine::input::Mouse::SetScrollWheelDeadzone(float value) {
	scrollWheel.Deadzone = std::clamp(value, 0.f, 1.f);
}

void engine::input::Mouse::AddCallback(Button button, ButtonCallback callback) {
	buttons[static_cast<std::underlying_type<Button>::type>(button)].callbacks.push_back(std::move(callback));
}

void engine::input::Mouse::ClearCallbacks(Button button) {
	buttons[static_cast<std::underlying_type<Button>::type>(button)].callbacks.clear();
}

void engine::input::Mouse::update() {
	pressedButtons.clear();
}
