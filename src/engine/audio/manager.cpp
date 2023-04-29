// Copyright © 2022-2023 Daylon Wilkins & James Cor
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <engine/audio/audio.hpp>
#include <engine/log/log.hpp>
#include <engine/strings/strings.hpp>

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio/miniaudio.h>
#include <iostream>
#include <unordered_set>

// VFS Implementation --------------------------------------------------------------------------------------------------

struct maVFS {
	ma_vfs_callbacks callbacks;
	engine::fs::IFileSystem* fileSystem;
};

ma_result onOpen(ma_vfs* vfs, const char* filePath, ma_uint32 openMode, ma_vfs_file* vfsFile) {
	auto fileSystem = reinterpret_cast<maVFS*>(vfs)->fileSystem;
	bool shouldStream = filePath[0] == '1';
	filePath = filePath + 1;
	if (!shouldStream) {
		auto file = fileSystem->ReadFile(filePath);
		if (!file) {
			return MA_ERROR;
		}
		*vfsFile = reinterpret_cast<ma_vfs_file>(file.release());
	} else {
		auto file = fileSystem->StreamFile(filePath);
		if (!file) {
			return MA_ERROR;
		}
		*vfsFile = reinterpret_cast<ma_vfs_file>(file.release());
	}
	return MA_SUCCESS;
}

ma_result onOpenW(ma_vfs* vfs, const wchar_t* filePath, ma_uint32 openMode, ma_vfs_file* vfsFile) {
	auto fileSystem = reinterpret_cast<maVFS*>(vfs)->fileSystem;
	auto convertedPath = engine::strings::FromWide(filePath);
	bool shouldStream = convertedPath[0] == '1';
	convertedPath = convertedPath.substr(1);
	if (!shouldStream) {
		auto file = fileSystem->ReadFile(convertedPath);
		if (!file) {
			return MA_ERROR;
		}
		*vfsFile = reinterpret_cast<ma_vfs_file>(file.release());
	} else {
		auto file = fileSystem->StreamFile(convertedPath);
		if (!file) {
			return MA_ERROR;
		}
		*vfsFile = reinterpret_cast<ma_vfs_file>(file.release());
	}
	return MA_SUCCESS;
}

ma_result onClose(ma_vfs* vfs, ma_vfs_file file) {
	auto identifiable = reinterpret_cast<engine::fs::IBlobIdentifiable*>(file);
	switch (identifiable->Type()) {
		case engine::fs::IBlobIdentifiable::BlobType::Blob:
			delete (static_cast<engine::fs::IBlob*>(file));
			break;
		case engine::fs::IBlobIdentifiable::BlobType::StreamBlob:
			delete (static_cast<engine::fs::IStreamBlob*>(file));
			break;
		default:
			engine::log::Fatal("unknown IBlobIdentifiable encountered when attempting to close audio file");
	}
	return MA_SUCCESS;
}

ma_result onRead(ma_vfs* vfs, ma_vfs_file file, void* destination, size_t sizeInBytes, size_t* pBytesRead) {
	auto identifiable = reinterpret_cast<engine::fs::IBlobIdentifiable*>(file);
	switch (identifiable->Type()) {
		case engine::fs::IBlobIdentifiable::BlobType::Blob: {
			auto blobFile = static_cast<engine::fs::IBlob*>(file);
			if (sizeInBytes > blobFile->Size()) {
				sizeInBytes = blobFile->Size();
			}
			*pBytesRead = sizeInBytes;
			memcpy(destination, blobFile->Data(), sizeInBytes);
			break;
		}
		case engine::fs::IBlobIdentifiable::BlobType::StreamBlob: {
			auto streamFile = static_cast<engine::fs::IStreamBlob*>(file);
			auto nextBlob = streamFile->Next(sizeInBytes);
			if (nextBlob) {
				*pBytesRead = nextBlob->Size();
				memcpy(destination, nextBlob->Data(), nextBlob->Size());
			} else {
				*pBytesRead = 0;
			}
			break;
		}
		default:
			engine::log::Fatal("unknown IBlobIdentifiable encountered when attempting to read audio file");
	}
	return MA_SUCCESS;
}

ma_result onWrite(ma_vfs* vfs, ma_vfs_file file, const void* source, size_t sizeInBytes, size_t* bytesWritten) {
	engine::log::Error("Writing audio files has not yet been implemented");
	return MA_ERROR;
}

ma_result onSeek(ma_vfs* vfs, ma_vfs_file file, ma_int64 offset, ma_seek_origin origin) {
	auto identifiable = reinterpret_cast<engine::fs::IBlobIdentifiable*>(file);
	switch (identifiable->Type()) {
		case engine::fs::IBlobIdentifiable::BlobType::Blob: {
			break;
		}
		case engine::fs::IBlobIdentifiable::BlobType::StreamBlob: {
			auto streamFile = static_cast<engine::fs::IStreamBlob*>(file);
			if (origin != ma_seek_origin_start) {
				offset += (ma_int64)streamFile->Position();
			}
			if (offset != streamFile->Position()) {
				streamFile->Seek(offset);
			}
			break;
		}
		default:
			engine::log::Fatal("unknown IBlobIdentifiable encountered when attempting to seek audio file");
	}
	return MA_SUCCESS;
}

ma_result onTell(ma_vfs* vfs, ma_vfs_file file, ma_int64* cursor) {
	auto identifiable = reinterpret_cast<engine::fs::IBlobIdentifiable*>(file);
	switch (identifiable->Type()) {
		case engine::fs::IBlobIdentifiable::BlobType::Blob: {
			*cursor = 0;
			break;
		}
		case engine::fs::IBlobIdentifiable::BlobType::StreamBlob: {
			auto streamFile = static_cast<engine::fs::IStreamBlob*>(file);
			*cursor = (ma_int64)streamFile->Position();
			break;
		}
		default:
			engine::log::Fatal("unknown IBlobIdentifiable encountered when attempting to read audio file seek position");
	}
	return MA_SUCCESS;
}

ma_result onInfo(ma_vfs* vfs, ma_vfs_file file, ma_file_info* info) {
	auto identifiable = reinterpret_cast<engine::fs::IBlobIdentifiable*>(file);
	switch (identifiable->Type()) {
		case engine::fs::IBlobIdentifiable::BlobType::Blob: {
			auto blobFile = static_cast<engine::fs::IBlob*>(file);
			info->sizeInBytes = blobFile->Size();
			break;
		}
		case engine::fs::IBlobIdentifiable::BlobType::StreamBlob: {
			auto streamFile = static_cast<engine::fs::IStreamBlob*>(file);
			info->sizeInBytes = streamFile->Size();
			break;
		}
		default:
			engine::log::Fatal("unknown IBlobIdentifiable encountered when attempting to read audio file info");
	}
	return MA_SUCCESS;
}

// Private Sound Group Implementation ----------------------------------------------------------------------------------
class engine::audio::Manager::soundGroupImpl {
public:
	soundGroupImpl(ma_engine* baseEngine, ma_uint32 flags, SoundGroup parent, ma_sound_group* parentGroup);
	~soundGroupImpl();
	void SetVolume(float volume);
	std::unique_ptr<ma_sound_group> MaSoundGroup;
	SoundGroup Parent;
	std::unordered_set<SoundGroup> Children;
};

engine::audio::Manager::soundGroupImpl::soundGroupImpl(ma_engine* baseEngine, ma_uint32 flags, SoundGroup parent, ma_sound_group* parentGroup) {
	MaSoundGroup = std::make_unique<ma_sound_group>();
	ma_result result = ma_sound_group_init(baseEngine,
										   flags,
										   parentGroup,
										   MaSoundGroup.get());
	if (result != MA_SUCCESS) {
		engine::log::Fatal("Failed to initialize sound group.");
	}

	this->Parent = parent;
}
engine::audio::Manager::soundGroupImpl::~soundGroupImpl() {
	ma_sound_group_uninit(MaSoundGroup.get());
};

void engine::audio::Manager::soundGroupImpl::SetVolume(float volume) {
	ma_sound_group_set_volume(MaSoundGroup.get(), volume);
}

// Private Manager Implementation --------------------------------------------------------------------------------------

class engine::audio::Manager::privateImpl {
public:
	privateImpl(engine::fs::IFileSystem* vfs);
	~privateImpl();

	std::unique_ptr<ma_engine> miniAudioEngine;
	std::unique_ptr<maVFS> miniAudioVFS;
};

engine::audio::Manager::privateImpl::privateImpl(engine::fs::IFileSystem* vfs) {
	miniAudioEngine = std::make_unique<ma_engine>();
	miniAudioVFS = std::make_unique<maVFS>();
	miniAudioVFS->fileSystem = vfs;
	miniAudioVFS->callbacks = ma_vfs_callbacks{
		.onOpen = onOpen,
		.onOpenW = onOpenW,
		.onClose = onClose,
		.onRead = onRead,
		.onWrite = onWrite,
		.onSeek = onSeek,
		.onTell = onTell,
		.onInfo = onInfo,
	};
	ma_engine_config engineConfig = ma_engine_config_init();
	engineConfig.pResourceManagerVFS = miniAudioVFS.get();

	ma_result result = ma_engine_init(&engineConfig, miniAudioEngine.get());
	if (result != MA_SUCCESS) {
		engine::log::Fatal("Failed to initialize audio.");
	}
}

engine::audio::Manager::privateImpl::~privateImpl() {
	ma_engine_uninit(miniAudioEngine.get());
}

// Public Manager Implementation ---------------------------------------------------------------------------------------

engine::audio::Manager::Manager(engine::fs::IFileSystem* vfs) {
	impl = std::make_unique<privateImpl>(vfs);
	soundGroups[0] = new soundGroupImpl(impl->miniAudioEngine.get(), 0, 0, nullptr);
	listener = std::unique_ptr<Listener>(new Listener(impl->miniAudioEngine.get(), glm::vec3(0)));
}

engine::audio::Manager::~Manager() {
	deleteSoundGroup(0);
	assert(soundGroups.empty());
};

std::unique_ptr<engine::audio::Sound> engine::audio::Manager::LoadMusic(
	const std::filesystem::path& filePath,
	SoundGroup soundGroup,
	bool streamFromFile,
	bool loop) {
	std::unique_ptr<Sound> sound(new Sound(impl->miniAudioEngine.get()));
	ma_sound_group* maSoundGroup;
	auto soundGroupImplIter = soundGroups.find(soundGroup);
	if (soundGroupImplIter == soundGroups.end()) {
		engine::log::Debug("Failed to find sound group %d. Assigning to master", soundGroup);
		maSoundGroup = soundGroups[0]->MaSoundGroup.get();
	} else {
		maSoundGroup = soundGroupImplIter->second->MaSoundGroup.get();
	}
	if (!sound->init(filePath,
					 maSoundGroup,
					 streamFromFile,
					 false,
					 loop)) {
		return nullptr;
	}
	return std::move(sound);
}

std::unique_ptr<engine::audio::Sound> engine::audio::Manager::LoadSoundEffect(
	const std::filesystem::path& filePath,
	SoundGroup soundGroup,
	bool streamFromFile,
	bool loop) {
	std::unique_ptr<Sound> sound(new Sound(impl->miniAudioEngine.get()));
	ma_sound_group* maSoundGroup;
	auto soundGroupImplIter = soundGroups.find(soundGroup);
	if (soundGroupImplIter == soundGroups.end()) {
		engine::log::Debug("Failed to find sound group %d. Assigning to master", soundGroup);
		maSoundGroup = soundGroups[0]->MaSoundGroup.get();
	} else {
		maSoundGroup = soundGroupImplIter->second->MaSoundGroup.get();
	}
	if (!sound->init(filePath,
					 maSoundGroup,
					 streamFromFile,
					 false,
					 loop)) {
		return nullptr;
	}
	return std::move(sound);
}

std::unique_ptr<engine::audio::PositionalSound> engine::audio::Manager::LoadPositionalSoundEffect(
	const std::filesystem::path& filePath,
	SoundGroup soundGroup,
	bool streamFromFile,
	bool loop) {
	std::unique_ptr<PositionalSound> sound(new PositionalSound(impl->miniAudioEngine.get(), glm::vec3(0)));
	ma_sound_group* maSoundGroup;
	auto soundGroupImplIter = soundGroups.find(soundGroup);
	if (soundGroupImplIter == soundGroups.end()) {
		engine::log::Debug("Failed to find sound group %d. Assigning to master", soundGroup);
		maSoundGroup = soundGroups[0]->MaSoundGroup.get();
	} else {
		maSoundGroup = soundGroupImplIter->second->MaSoundGroup.get();
	}
	if (!sound->init(filePath,
					 maSoundGroup,
					 streamFromFile,
					 true,
					 loop)) {
		return nullptr;
	}
	return std::move(sound);
}

engine::audio::Listener* engine::audio::Manager::GetListener() {
	return this->listener.get();
}

engine::audio::SoundGroup engine::audio::Manager::GetMasterSoundGroup() {
	return 0;
}

engine::audio::SoundGroup engine::audio::Manager::CreateSoundGroup(engine::audio::SoundGroup parentGroup) {
	ma_sound_group* baseParentGroup = nullptr;
	auto parentImpl = soundGroups.find(parentGroup);
	if (parentImpl != soundGroups.end()) {
		baseParentGroup = parentImpl->second->MaSoundGroup.get();
	} else {
		baseParentGroup = soundGroups.find(0)->second->MaSoundGroup.get();
	}

	auto sgImpl = new soundGroupImpl(impl->miniAudioEngine.get(), 0, parentGroup, baseParentGroup);
	int sgID = nextSoundGroupID++;
	soundGroups[sgID] = sgImpl;
	parentImpl->second->Children.insert(sgID);

	return sgID;
}

void engine::audio::Manager::deleteSoundGroupChildren(engine::audio::SoundGroup soundGroup) {
	// don't delete if not found
	auto group = soundGroups.find(soundGroup);
	if (group == soundGroups.end()) {
		return;
	}

	// delete children
	for (SoundGroup child: group->second->Children) {
		deleteSoundGroupChildren(child);
	}

	// clear memory
	delete group->second;
	soundGroups.erase(soundGroup);
}

void engine::audio::Manager::deleteSoundGroup(engine::audio::SoundGroup soundGroup) {
	// don't delete if not found
	auto group = soundGroups.find(soundGroup);
	if (group == soundGroups.end()) {
		return;
	}

	// delete children
	for (SoundGroup child: group->second->Children) {
		deleteSoundGroupChildren(child);
	}

	// remove from parent
	auto parentGroup = soundGroups.find(group->second->Parent);
	parentGroup->second->Children.erase(soundGroup);

	// clear memory
	delete group->second;
	soundGroups.erase(soundGroup);
}

void engine::audio::Manager::DeleteSoundGroup(engine::audio::SoundGroup soundGroup) {
	// don't delete master
	if (soundGroup == 0) {
		return;
	}

	deleteSoundGroup(soundGroup);
}

void engine::audio::Manager::SetVolume(float volume, engine::audio::SoundGroup soundGroup) {
	auto group = soundGroups.find(soundGroup);
	if (group == soundGroups.end()) {
		return;
	}
	group->second->SetVolume(volume);
}
