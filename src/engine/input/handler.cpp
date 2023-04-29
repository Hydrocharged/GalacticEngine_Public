// Copyright © 2022-2023 Daylon Wilkins & James Cor
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <engine/input/input.hpp>
#include <engine/log/log.hpp>

engine::input::Handler::Handler(engine::Application* application) : application(application) {}

engine::input::Handler::~Handler() {
	for (int i = (int)devices.size() - 1; i >= 0; i--) {
		auto device = devices[i];
		device->isConnected = false;
		devices.erase(devices.begin() + i);
		for (const auto& callback: device->onDisconnect) {
			callback(device->id);
		}
	}
}

std::shared_ptr<engine::input::Mouse> engine::input::Handler::GetDevice(MouseID id) {
	for (auto& device: devices) {
		if (device->id.Raw == id.ID.Raw) {
			return std::static_pointer_cast<engine::input::Mouse>(device);
		}
	}
	engine::log::Error("Device with ID '%u' not found", id.ID.Raw);
	return nullptr;
}

std::shared_ptr<engine::input::Keyboard> engine::input::Handler::GetDevice(KeyboardID id) {
	for (auto& device: devices) {
		if (device->id.Raw == id.ID.Raw) {
			return std::static_pointer_cast<engine::input::Keyboard>(device);
		}
	}
	engine::log::Error("Device with ID '%u' not found", id.ID.Raw);
	return nullptr;
}

std::shared_ptr<engine::input::Gamepad> engine::input::Handler::GetDevice(GamepadID id) {
	for (auto& device: devices) {
		if (device->id.Raw == id.ID.Raw) {
			return std::static_pointer_cast<engine::input::Gamepad>(device);
		}
	}
	engine::log::Error("Device with ID '%u' not found", id.ID.Raw);
	return nullptr;
}

std::unique_ptr<engine::input::Mouse::Setter> engine::input::Handler::ConnectMouse() {
	auto device = std::shared_ptr<engine::input::Device>(new engine::input::Mouse(this, nextId));
	nextId += 1;
	auto setter = std::unique_ptr<engine::input::Mouse::Setter>(
		new engine::input::Mouse::Setter(std::static_pointer_cast<Mouse>(device)));
	devices.push_back(std::move(device));
	return std::move(setter);
}

std::unique_ptr<engine::input::Keyboard::Setter> engine::input::Handler::ConnectKeyboard() {
	auto device = std::shared_ptr<engine::input::Device>(new engine::input::Keyboard(this, nextId));
	nextId += 1;
	auto setter = std::unique_ptr<engine::input::Keyboard::Setter>(
		new engine::input::Keyboard::Setter(std::static_pointer_cast<Keyboard>(device)));
	devices.push_back(std::move(device));
	return std::move(setter);
}

std::unique_ptr<engine::input::Gamepad::Setter> engine::input::Handler::ConnectGamepad() {
	auto device = std::shared_ptr<engine::input::Device>(new engine::input::Gamepad(this, nextId));
	nextId += 1;
	auto setter = std::unique_ptr<engine::input::Gamepad::Setter>(
		new engine::input::Gamepad::Setter(std::static_pointer_cast<Gamepad>(device)));
	devices.push_back(std::move(device));
	return std::move(setter);
}

std::vector<engine::input::Handler::MouseID> engine::input::Handler::ListConnectedMice() {
	std::vector<engine::input::Handler::MouseID> mice;
	for (const auto& device: devices) {
		if (device->id.GetType() == engine::input::Device::Type::Mouse) {
			mice.push_back(MouseID{.ID = device->id});
		}
	}
	return mice;
}

std::vector<engine::input::Handler::KeyboardID> engine::input::Handler::ListConnectedKeyboards() {
	std::vector<engine::input::Handler::KeyboardID> keyboards;
	for (const auto& device: devices) {
		if (device->id.GetType() == engine::input::Device::Type::Keyboard) {
			keyboards.push_back(KeyboardID{.ID = device->id});
		}
	}
	return keyboards;
}

std::vector<engine::input::Handler::GamepadID> engine::input::Handler::ListConnectedGamepads() {
	std::vector<engine::input::Handler::GamepadID> gamepads;
	for (const auto& device: devices) {
		if (device->id.GetType() == engine::input::Device::Type::Gamepad) {
			gamepads.push_back(GamepadID{.ID = device->id});
		}
	}
	return gamepads;
}

engine::input::Mouse::CaptureState engine::input::Handler::GetMouseCaptureState() {
	return mouseCaptureState;
}

void engine::input::Handler::DisconnectDevice(Device::ID id) {
	for (int i = 0; i < devices.size(); i++) {
		if (devices[i]->id.Raw == id.Raw) {
			auto device = devices[i];
			device->isConnected = false;
			devices.erase(devices.begin() + i);
			for (const auto& callback: device->onDisconnect) {
				callback(id);
			}
			return;
		}
	}
}

void engine::input::Handler::Update() {
	for (auto& device: devices) {
		device->update();
	}
}
