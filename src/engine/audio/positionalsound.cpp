// Copyright © 2022-2023 Daylon Wilkins & James Cor
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <engine/audio/audio.hpp>
#include <miniaudio/miniaudio.h>
#include <limits>

engine::audio::PositionalSound::PositionalSound(void* engine, glm::vec3 initPos) : Sound(engine) {
	ma_sound_set_positioning(reinterpret_cast<ma_sound*>(this->baseSound), ma_positioning_relative);
	this->SetPosition(initPos, 0.0f);
}

engine::audio::PositionalSound::~PositionalSound() = default;

void engine::audio::PositionalSound::SetPosition(glm::vec3 pos, float deltaTime) {
	setPosition(pos, deltaTime);
}