// Copyright © 2022-2023 Daylon Wilkins & James Cor
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <engine/audio/audio.hpp>
#include <miniaudio/miniaudio.h>
#include <limits>

engine::audio::Listener::Listener(void* engine, glm::vec3 initPos) {
	baseEngine = engine;
	this->SetPosition(initPos, 0.0f);
}

engine::audio::Listener::~Listener() = default;

void engine::audio::Listener::SetPosition(glm::vec3 pos, float deltaTime) {
	glm::vec3 vel(0);
	if (deltaTime > std::numeric_limits<float>::epsilon()) {
		vel = (pos - this->pos) / deltaTime;
	}
	this->pos = pos;
	auto maEngine = reinterpret_cast<ma_engine*>(this->baseEngine);
	ma_engine_listener_set_position(maEngine, 0, pos.x, pos.y, pos.z);
	ma_engine_listener_set_velocity(maEngine, 0, vel.x, vel.y, vel.z);
}

void engine::audio::Listener::SetDirection(glm::vec3 dir) {
	ma_engine_listener_set_direction(reinterpret_cast<ma_engine*>(this->baseEngine), 0, dir.x, dir.y, dir.z);
}