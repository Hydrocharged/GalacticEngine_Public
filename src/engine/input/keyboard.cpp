// Copyright © 2022-2023 Daylon Wilkins & James Cor
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <engine/input/input.hpp>
#include <string>

std::string keyStrs[] = {
	"F1",
	"F2",
	"F3",
	"F4",
	"F5",
	"F6",
	"F7",
	"F8",
	"F9",
	"F10",
	"F11",
	"F12",
	"PrintScreen",
	"ScrollLock",
	"PauseBreak",
	"NumLock",
	"Number1",
	"Number2",
	"Number3",
	"Number4",
	"Number5",
	"Number6",
	"Number7",
	"Number8",
	"Number9",
	"Number0",
	"Backtick",
	"Tilde",
	"Exclamation",
	"At",
	"Pound",
	"Dollar",
	"Percent",
	"Caret",
	"Ampersand",
	"Star",
	"LeftParen",
	"RightParen",
	"Dash",
	"Underscore",
	"Equals",
	"Plus",
	"Backspace",
	"Tab",
	"CapsLock",
	"LeftShift",
	"RightShift",
	"LeftCtrl",
	"RightCtrl",
	"LeftAlt",
	"RightAlt",
	"Enter",
	"Space",
	"Escape",
	"LeftBrace",
	"RightBrace",
	"LeftCurlyBrace",
	"RightCurlyBrace",
	"Backslash",
	"ForwardSlash",
	"Pipe",
	"Semicolon",
	"Colon",
	"Quote",
	"DoubleQuote",
	"Comma",
	"Period",
	"QuestionMark",
	"LessThan",
	"GreaterThan",
	"A",
	"B",
	"C",
	"D",
	"E",
	"F",
	"G",
	"H",
	"I",
	"J",
	"K",
	"L",
	"M",
	"N",
	"O",
	"P",
	"Q",
	"R",
	"S",
	"T",
	"U",
	"V",
	"W",
	"X",
	"Y",
	"Z",
	"Insert",
	"Delete",
	"Home",
	"End",
	"PageUp",
	"PageDown",
	"ArrowUp",
	"ArrowDown",
	"ArrowLeft",
	"ArrowRight",
};

// Setter --------------------------------------------------------------------------------------------------------------

engine::input::Keyboard::Setter::Setter(std::shared_ptr<Keyboard> keyboard) {
	this->keyboard = std::move(keyboard);
}

engine::input::Keyboard::Setter::~Setter() = default;

std::shared_ptr<engine::input::Keyboard> engine::input::Keyboard::Setter::GetKeyboard() {
	return keyboard;
}

engine::input::Device::ID engine::input::Keyboard::Setter::GetID() const {
	return keyboard->GetID();
}

void engine::input::Keyboard::Setter::SetKey(Key key, bool isDown, double currentTime) {
	keyState& state = keyboard->keys[static_cast<std::underlying_type<Key>::type>(key)];
	if (state.state.IsDown != isDown) {
		if (!isDown) {
			keyboard->pressedKeys.push_back(key);
		}
		state.state.IsDown = isDown;
		state.state.StartTime = currentTime;
		for (const auto& callback: state.callbacks) {
			callback(key, state.state);
		}
	}
}

// Keyboard ------------------------------------------------------------------------------------------------------------

engine::input::Keyboard::Keyboard(Handler* handler, std::uint16_t id) : Device(handler, Device::Type::Keyboard, id) {
	keys.resize(128);
	std::fill(keys.begin(), keys.end(), Keyboard::keyState{});
}

engine::input::Keyboard::~Keyboard() = default;

engine::input::KeyState engine::input::Keyboard::GetKey(Key key) const {
	return keys[static_cast<std::underlying_type<Key>::type>(key)].state;
}

bool engine::input::Keyboard::IsKeyDown(Key key) const {
	return keys[static_cast<std::underlying_type<Key>::type>(key)].state.IsDown;
}

bool engine::input::Keyboard::IsKeyUp(Key key) const {
	return !keys[static_cast<std::underlying_type<Key>::type>(key)].state.IsDown;
}

bool engine::input::Keyboard::IsKeyPressed(Key key) const {
	for (const auto& pressedKey: pressedKeys) {
		if (key == pressedKey) {
			return true;
		}
	}
	return false;
}

void engine::input::Keyboard::AddCallback(Key key, KeyCallback callback) {
	keys[static_cast<std::underlying_type<Key>::type>(key)].callbacks.push_back(std::move(callback));
}

void engine::input::Keyboard::ClearCallbacks(Key key) {
	keys[static_cast<std::underlying_type<Key>::type>(key)].callbacks.clear();
}

void engine::input::Keyboard::update() {
	pressedKeys.clear();
}
