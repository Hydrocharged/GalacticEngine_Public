// Copyright © 2022-2023 Daylon Wilkins & James Cor
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <engine/audio/audio.hpp>
#include <engine/log/log.hpp>
#include <miniaudio/miniaudio.h>

engine::audio::Sound::Sound(void* engine) {
	baseEngine = engine;
}

engine::audio::Sound::~Sound() {
	if (baseSound) {
		auto maSound = reinterpret_cast<ma_sound*>(baseSound);
		ma_sound_uninit(maSound);
		delete (maSound);
	}
}

bool engine::audio::Sound::init(
	const std::filesystem::path& filePath,
	void* soundGroup,
	bool streamFromFile,
	bool spatialized,
	bool loop) {

	auto maEngine = reinterpret_cast<ma_engine*>(baseEngine);
	auto maSound = new ma_sound();
	baseSound = reinterpret_cast<void*>(maSound);
	file = filePath.generic_string();

	ma_uint32 flags = 0;
	std::string modifiedFilePathStr;
	if (streamFromFile) {
		flags = MA_SOUND_FLAG_ASYNC | MA_SOUND_FLAG_STREAM;
		modifiedFilePathStr = "1" + file;
	} else {
		modifiedFilePathStr = "0" + file;
	}
	auto maSoundGroup = reinterpret_cast<ma_sound_group*>(soundGroup);
	ma_result result = ma_sound_init_from_file(maEngine,
											   modifiedFilePathStr.c_str(),
											   flags,
											   maSoundGroup,
											   nullptr,
											   maSound);
	if (result != MA_SUCCESS) {
		engine::log::Error("Failed to load sound \"%s\".", file.c_str());
		return false;
	}

	ma_sound_set_spatialization_enabled(maSound, spatialized);
	ma_sound_set_looping(maSound, loop);

	return true;
}

void engine::audio::Sound::Play() {
	if (ma_sound_start(reinterpret_cast<ma_sound*>(baseSound)) != MA_SUCCESS) {
		engine::log::Error("Failed to play sound \"%s\".", file.c_str());
	}
}

void engine::audio::Sound::Pause() {
	if (ma_sound_stop(reinterpret_cast<ma_sound*>(baseSound)) != MA_SUCCESS) {
		engine::log::Error("Failed to pause sound \"%s\".", file.c_str());
	}
}

void engine::audio::Sound::Stop() {
	Pause();
	ma_sound_seek_to_pcm_frame(reinterpret_cast<ma_sound*>(baseSound), 0);
}

void engine::audio::Sound::SetVolume(float volume) {
	ma_sound_set_volume(reinterpret_cast<ma_sound*>(baseSound), volume);
}

void engine::audio::Sound::setPosition(glm::vec3 pos, float deltaTime) {
	glm::vec3 vel(0);
	glm::vec3 dir(0);
	if (deltaTime > std::numeric_limits<float>::epsilon()) {
		vel = (pos - this->pos) / deltaTime;
		if (glm::length(vel) > std::numeric_limits<float>::epsilon()) {
			dir = glm::normalize(vel);
		}
	}
	this->pos = pos;
	auto maSound = reinterpret_cast<ma_sound*>(this->baseSound);
	ma_sound_set_position(maSound, pos.x, pos.y, pos.z);
	ma_sound_set_direction(maSound, dir.x, dir.y, dir.z);
	ma_sound_set_velocity(maSound, vel.x, vel.y, vel.z);
}
