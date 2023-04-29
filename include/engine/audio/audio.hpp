// Copyright © 2022-2023 Daylon Wilkins & James Cor
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef ENGINE_AUDIO_AUDIO_HPP
#define ENGINE_AUDIO_AUDIO_HPP

#include <engine/fs/fs.hpp>
#include <glm/glm.hpp>
#include <unordered_map>

namespace engine::audio {
	class Sound {
	public:
		~Sound();
		void Play();
		void Pause();
		void Stop();
		void SetVolume(float volume);
	protected:
		friend class Manager;

		void* baseSound = nullptr;
		void* baseEngine = nullptr;
		glm::vec3 pos;

		Sound(void* engine);
		bool init(const std::filesystem::path& filePath,
				  void* soundGroup,
				  bool streamFromFile,
				  bool spatialized,
				  bool looping);
		void setPosition(glm::vec3 pos, float deltaTime);

	private:
		std::string file;
	};

	class PositionalSound : public Sound {
	public:
		~PositionalSound();
		// SetPosition will move the sound to play at given position, and apply any velocity and direction changes.
		void SetPosition(glm::vec3 pos, float deltaTime);
	private:
		friend class Manager;

		PositionalSound(void* engine, glm::vec3 initPos);
	};

	class Listener {
	public:
		~Listener();
		void SetPosition(glm::vec3 pos, float deltaTime);
		void SetDirection(glm::vec3 dir);
	private:
		friend class Manager;

		Listener(void* engine, glm::vec3 initPos);

		void* baseEngine = nullptr;

		glm::vec3 pos;
	};

	typedef int SoundGroup;

	class Manager {
	public:
		Manager(engine::fs::IFileSystem* vfs);
		~Manager();
		// LoadMusic loads the given music file.
		// Can specify soundGroup (default master).
		// Can specify if it should stream music from file (default true).
		// Can specify if it should loop (default true).
		// Will not have spatialization enabled.
		// Returns null on error.
		std::unique_ptr<Sound> LoadMusic(
			const std::filesystem::path& filePath,
			SoundGroup soundGroup = 0,
			bool streamFromFile = true,
			bool loop = true);
		// LoadSoundEffect loads the given sound effect file.
		// Can specify soundGroup (default master).
		// Can specify if it should stream music from file (default false).
		// Can specify if it should loop (default false).
		// Will not have spatialization enabled.
		// Returns null on error.
		std::unique_ptr<Sound> LoadSoundEffect(
			const std::filesystem::path& filePath,
			SoundGroup soundGroup = 0,
			bool streamFromFile = false,
			bool loop = false);
		// LoadPositionalSoundEffect loads the given positional sound effect file.
		// Can specify soundGroup (default master).
		// Can specify if it should stream music from file (default false).
		// Can specify if it should loop (default false).
		// Will have spatialization enabled, and start off at (0, 0, 0).
		// Returns null on error.
		std::unique_ptr<PositionalSound> LoadPositionalSoundEffect(
			const std::filesystem::path& filePath,
			SoundGroup soundGroup = 0,
			bool streamFromFile = false,
			bool loop = false);

		Listener* GetListener();

		// GetMasterSoundGroup returns the master sound group (0).
		static SoundGroup GetMasterSoundGroup();
		// CreateSoundGroup will create a sound group with the provided parent sound group.
		// If no parent sound group is provided, it will default to using the master sound group (0) as parent.
		SoundGroup CreateSoundGroup(SoundGroup parentGroup = 0);
		// DeleteSoundGroup will delete the provided sound group.
		// You cannot delete the master sound group (0); no error is returned, just nothing happens.
		void DeleteSoundGroup(SoundGroup soundGroup);
		// SetVolume sets the volume level of the sound group using a linear scale (0 = 0%, 1 = 100%, >= 1 = amplification).
		// If no sound group is provided, it will default to the master sound group (0).
		void SetVolume(float volume, SoundGroup soundGroup = 0);

	private:
		class privateImpl;

		class soundGroupImpl;

		std::unique_ptr<privateImpl> impl;
		std::unordered_map<int, soundGroupImpl*> soundGroups;
		std::unique_ptr<engine::audio::Listener> listener;
		int nextSoundGroupID = 1; // 0 is taken up by master
		void deleteSoundGroup(SoundGroup soundGroup);
		void deleteSoundGroupChildren(SoundGroup soundGroup);
	};
}

#endif //ENGINE_AUDIO_AUDIO_HPP
