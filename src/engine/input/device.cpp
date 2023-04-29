// Copyright © 2022-2023 Daylon Wilkins & James Cor
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <engine/input/input.hpp>
#include <engine/log/log.hpp>

const std::uint32_t mouseFlag = 0b001 << 28;
const std::uint32_t keyboardFlag = 0b010 << 28;
const std::uint32_t gamepadFlag = 0b100 << 28;

engine::input::Device::Device(engine::input::Handler* handler, engine::input::Device::Type type, std::uint16_t id) {
	this->handler = handler;
	switch (type) {
		case engine::input::Device::Type::Mouse:
			this->id.Raw = mouseFlag + id;
			break;
		case engine::input::Device::Type::Keyboard:
			this->id.Raw = keyboardFlag + id;
			break;
		case engine::input::Device::Type::Gamepad:
			this->id.Raw = gamepadFlag + id;
			break;
		default:
			engine::log::Fatal("Unknown Device type encountered");
			this->id.Raw = 0;
	}
	this->isConnected = true;
}

engine::input::Device::~Device() = default;

engine::input::Handler* engine::input::Device::GetHandler() const {
	return handler;
}

engine::input::Device::ID engine::input::Device::GetID() const {
	return id;
}

engine::input::Device::Type engine::input::Device::GetType() const {
	return id.GetType();
}

bool engine::input::Device::IsConnected() const {
	return isConnected;
}

void engine::input::Device::AddCallback(DisconnectCallback callback) {
	onDisconnect.push_back(std::move(callback));
}

engine::input::Device::Type engine::input::Device::ID::GetType() const {
	switch (Raw & (0b111 << 28)) {
		case mouseFlag:
			return Type::Mouse;
		case keyboardFlag:
			return Type::Keyboard;
		case gamepadFlag:
			return Type::Gamepad;
		default:
			engine::log::Fatal("Unknown Device type encountered");
			return Type::Mouse;
	}
}
