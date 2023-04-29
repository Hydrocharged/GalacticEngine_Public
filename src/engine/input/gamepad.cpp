// Copyright © 2022-2023 Daylon Wilkins & James Cor
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <engine/input/input.hpp>
#include <algorithm>
#include <glm/glm.hpp>

float mapRange(float value, float min, float max) {
	float sign = float((value > 0) - (value < 0));
	float temp = (value * sign) - min;
	return sign * (temp * (1.f / (max - min)));
}

// Setter --------------------------------------------------------------------------------------------------------------

engine::input::Gamepad::Setter::Setter(std::shared_ptr<Gamepad> gamepad) {
	this->gamepad = std::move(gamepad);
}

engine::input::Gamepad::Setter::~Setter() = default;

std::shared_ptr<engine::input::Gamepad> engine::input::Gamepad::Setter::GetGamepad() {
	return gamepad;
}

engine::input::Device::ID engine::input::Gamepad::Setter::GetID() const {
	return gamepad->GetID();
}

void engine::input::Gamepad::Setter::SetButton(Button button, bool isDown, double currentTime) {
	buttonState& state = gamepad->buttons[static_cast<std::underlying_type<Button>::type>(button)];
	if (state.state.IsDown != isDown) {
		if (!isDown) {
			gamepad->pressedButtons.push_back(button);
		}
		state.state.IsDown = isDown;
		state.state.StartTime = currentTime;
		for (const auto& callback: state.callbacks) {
			callback(button, state.state);
		}
	}
}

void engine::input::Gamepad::Setter::SetTrigger(Trigger trigger, float value) {
	AnalogState& state = gamepad->triggers[static_cast<std::underlying_type<Trigger>::type>(trigger)];
	if (std::abs(value) > state.Deadzone) {
		state.Y = mapRange(value, state.Deadzone, 1.f);
	} else {
		state.Y = 0.f;
	}
}

void engine::input::Gamepad::Setter::SetStick(Stick stick, float x, float y) {
	//TODO: increase the deadzone of one axis as the value of the other axis increases (allows for exact directional movement)
	AnalogState& state = gamepad->sticks[static_cast<std::underlying_type<Stick>::type>(stick)];
	auto stickXY = glm::vec2(x, y);
	auto length = glm::length(stickXY);
	if (length > state.Deadzone) {
		stickXY = glm::normalize(stickXY) * ((length - state.Deadzone) / (1.0f - state.Deadzone));
		length = glm::length(stickXY);
		if (length > 1.0f) {
			stickXY = stickXY / length;
		}
		state.X = stickXY.x;
		state.Y = stickXY.y;
	} else {
		state.X = 0.f;
		state.Y = 0.f;
	}
}

// Gamepad -------------------------------------------------------------------------------------------------------------

engine::input::Gamepad::Gamepad(Handler* handler, std::uint16_t id) : Device(handler, Device::Type::Gamepad, id) {
	buttons.resize(18);
	triggers.resize(2);
	sticks.resize(2);
	std::fill(buttons.begin(), buttons.end(), Gamepad::buttonState{});
	std::fill(triggers.begin(), triggers.end(), AnalogState{});
	std::fill(sticks.begin(), sticks.end(), AnalogState{});
}

engine::input::Gamepad::~Gamepad() = default;

engine::input::KeyState engine::input::Gamepad::GetButton(Button button) const {
	return buttons[static_cast<std::underlying_type<Button>::type>(button)].state;
}

bool engine::input::Gamepad::IsButtonDown(Button button) const {
	return buttons[static_cast<std::underlying_type<Button>::type>(button)].state.IsDown;
}

bool engine::input::Gamepad::IsButtonUp(Button button) const {
	return !buttons[static_cast<std::underlying_type<Button>::type>(button)].state.IsDown;
}

bool engine::input::Gamepad::IsButtonPressed(Button button) const {
	for (const auto& pressedButton: pressedButtons) {
		if (button == pressedButton) {
			return true;
		}
	}
	return false;
}

float engine::input::Gamepad::GetTrigger(Trigger trigger) const {
	return triggers[static_cast<std::underlying_type<Trigger>::type>(trigger)].Y;
}

void engine::input::Gamepad::GetStick(Stick stick, float& x, float& y) {
	auto& state = sticks[static_cast<std::underlying_type<Stick>::type>(stick)];
	x = state.X;
	y = state.Y;
}

void engine::input::Gamepad::SetDeadzone(Trigger trigger, float value) {
	auto& state = triggers[static_cast<std::underlying_type<Trigger>::type>(trigger)];
	state.Deadzone = std::clamp(value, 0.f, 1.f);
}

void engine::input::Gamepad::SetDeadzone(Stick stick, float value) {
	auto& state = sticks[static_cast<std::underlying_type<Stick>::type>(stick)];
	state.Deadzone = std::clamp(value, 0.f, 1.f);
}

void engine::input::Gamepad::AddCallback(Button button, ButtonCallback callback) {
	buttons[static_cast<std::underlying_type<Button>::type>(button)].callbacks.push_back(std::move(callback));
}

void engine::input::Gamepad::ClearCallbacks(Button button) {
	buttons[static_cast<std::underlying_type<Button>::type>(button)].callbacks.clear();
}

void engine::input::Gamepad::update() {
	pressedButtons.clear();
}
