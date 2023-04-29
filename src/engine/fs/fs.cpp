// Copyright © 2022-2023 Daylon Wilkins & James Cor
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/*
* Copyright (c) 2014-2021, NVIDIA CORPORATION. All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
* DEALINGS IN THE SOFTWARE.
*/

// Adapted from https://github.com/NVIDIAGameWorks/donut/blob/main/src/core/vfs/VFS.cpp

#define NOMINMAX
#include <engine/fs/fs.hpp>
#include <engine/strings/strings.hpp>
#include <engine/log/log.hpp>
#include <fstream>
#include <utility>
#include <sstream>

using namespace engine::fs;

Blob::Blob(void* data, size_t size) : blobData(data), blobSize(size) {}

Blob::~Blob() {
	if (blobData) {
		free(blobData);
		blobData = nullptr;
	}
	blobSize = 0;
}

const void* Blob::Data() const {
	return blobData;
}

size_t Blob::Size() const {
	return blobSize;
}

StreamBlob::StreamBlob(std::string name, std::ifstream* stream, size_t size) :
	streamName(std::move(name)), filestream(stream), streamSize(size), streamIndex(0) {
	filestream->seekg(0, std::ios::beg);
}

StreamBlob::~StreamBlob() {
	if (filestream) {
		filestream->close();
		delete (filestream);
	}
}

void StreamBlob::Reset() {
	Seek(0);
}

void StreamBlob::Seek(size_t position) {
	filestream->seekg((long long)position, std::ios::beg);
	streamIndex = position;
}

const std::string& StreamBlob::Name() const {
	return streamName;
}

size_t StreamBlob::Size() const {
	return streamSize;
}

size_t StreamBlob::Position() const {
	return streamIndex;
}

bool StreamBlob::HasMore() const {
	return streamIndex < streamSize;
}

std::unique_ptr<IBlob> StreamBlob::Next(size_t blobSize) {
	size_t remaining = streamSize - streamIndex;
	if (blobSize > remaining) {
		blobSize = remaining;
	}
	if (blobSize == 0) {
		return nullptr;
	}

	char* data = static_cast<char*>(malloc(blobSize));
	if (data == nullptr) {
		engine::log::Fatal("failed to allocate %d bytes for streaming blob:\n%s", blobSize, streamName.c_str());
	}

	filestream->read(data, blobSize);
	if (!filestream->good()) {
		free(data);
		engine::log::Error("failed to read from streaming blob:\n%s", streamName.c_str());
		return nullptr;
	}

	streamIndex += blobSize;
	return std::make_unique<Blob>(data, blobSize);
}

bool NativeFileSystem::FolderExists(const std::filesystem::path& name) {
	return std::filesystem::exists(name) && std::filesystem::is_directory(name);
}

bool NativeFileSystem::FileExists(const std::filesystem::path& name) {
	return std::filesystem::exists(name) && std::filesystem::is_regular_file(name);
}

size_t NativeFileSystem::FileSize(const std::filesystem::path& name) {
	return (FileExists(name)) ? std::filesystem::file_size(name) : 0;
}

std::unique_ptr<IBlob> NativeFileSystem::ReadFile(const std::filesystem::path& name) {
	std::ifstream file(name, std::ios::binary);
	if (!file.is_open()) {
		engine::log::Error("unable to open file for reading:\n%ls", name.c_str());
		return nullptr;
	}

	file.seekg(0, std::ios::end);
	uint64_t size = file.tellg();
	file.seekg(0, std::ios::beg);

	if (size > 1099511627776) { // Max size of a terabyte
		engine::log::Error("file too large:\n%ls", name.c_str());
		return nullptr;
	}

	char* data = static_cast<char*>(malloc(size));
	if (data == nullptr) {
		engine::log::Fatal("failed to allocate %d bytes for file:\n%ls", size, name.c_str());
		return nullptr;
	}

	file.read(data, std::streamsize(size));
	if (!file.good()) {
		free(data);
		engine::log::Error("failed to read from file:\n%ls", name.c_str());
		return nullptr;
	}

	return std::make_unique<Blob>(data, size);
}

std::unique_ptr<IStreamBlob> NativeFileSystem::StreamFile(const std::filesystem::path& name) {
	std::ifstream* file = new std::ifstream(name, std::ios::binary);
	if (!file->is_open()) {
		delete file;
		engine::log::Error("unable to open file for reading:\n%ls", name.c_str());
		return nullptr;
	}

	file->seekg(0, std::ios::end);
	uint64_t size = file->tellg();

	if (size > 1099511627776) { // Max size of a terabyte
		file->close();
		delete file;
		engine::log::Error("file too large:\n%ls", name.c_str());
		return nullptr;
	}

	return std::make_unique<StreamBlob>(name.string(), file, size);
}

bool NativeFileSystem::WriteFile(const std::filesystem::path& name, const void* data, size_t size) {
	std::ofstream file(name, std::ios::binary);
	if (!file.is_open()) {
		engine::log::Error("unable to open file for writing:\n%ls", name.c_str());
		return false;
	}

	if (size > 0) {
		file.write(static_cast<const char*>(data), static_cast<std::streamsize>(size));
	}

	if (!file.good()) {
		engine::log::Error("failed to write to file:\n%ls", name.c_str());
		return false;
	}

	return true;
}

int NativeFileSystem::EnumerateFiles(const std::filesystem::path& path, const std::vector<std::string>& extensions, EnumerateCallback callback, bool allowDuplicates) {
	(void)allowDuplicates;

	if (extensions.empty()) {
		std::string pattern = (path / "*").generic_string();
		return enumerateNativeFiles(pattern.c_str(), false, callback);
	}

	int numEntries = 0;
	for (const auto& ext: extensions) {
		std::string pattern = (path / ("*" + ext)).generic_string();

		int result = enumerateNativeFiles(pattern.c_str(), false, callback);
		if (result < 0) {
			return result;
		}
		numEntries += result;
	}

	return numEntries;
}

int NativeFileSystem::EnumerateDirectories(const std::filesystem::path& path, EnumerateCallback callback, bool allowDuplicates) {
	(void)allowDuplicates;

	std::string pattern = (path / "*").generic_string();
	return enumerateNativeFiles(pattern.c_str(), true, callback);
}

int enumerateNativeFilesHelper(std::string currPath, std::vector<std::string>& pathVec, int i, bool directories, EnumerateCallback callback) {
	std::string pat = pathVec[i];
	if (pat == "." || pat == "..") {
		return enumerateNativeFilesHelper(currPath + "/" + pat, pathVec, i + 1, directories, callback);
	}
	pat = std::regex_replace(pat, std::regex("\\."), "\\.");
	pat = std::regex_replace(pat, std::regex("\\*"), ".*");

	int count = 0;

	// last subdirectory
	if (i == pathVec.size() - 1) {
		for (const auto& entry: std::filesystem::directory_iterator(currPath)) {
			if (entry.is_directory() != directories) {
				continue;
			}
			std::string entryStr = entry.path().string();
			entryStr = std::regex_replace(entryStr, std::regex("\\\\"), "/");
			if (std::regex_match(entryStr, std::regex(pat))) {
				callback(entryStr);
				count++;
			}
		}
		return count;
	}

	// recurse into directory
	for (const auto& entry: std::filesystem::directory_iterator(currPath)) {
		std::string entryStr = entry.path().string();
		entryStr = std::regex_replace(entryStr, std::regex("\\\\"), "/");
		entryStr = entryStr.substr(entryStr.find_last_of("/") + 1);
		if (entry.is_directory() && std::regex_match(entryStr, std::regex(pat))) {
			count += enumerateNativeFilesHelper(currPath + "/" + entryStr, pathVec, i + 1, directories, callback);
		}
	}
	return count;
}

int NativeFileSystem::enumerateNativeFiles(const char* pattern, bool directories, EnumerateCallback callback) {
	std::string tmp = std::string(pattern);
	std::vector<std::string> pathVec;
	while (true) {
		int pos = tmp.find_first_of("/");
		if (pos == std::string::npos) {
			break;
		}
		pathVec.push_back(tmp.substr(0, pos));
		tmp = tmp.substr(pos + 1);
	}
	if (tmp.size() > 0) {
		pathVec.push_back(tmp);
	}

	return enumerateNativeFilesHelper(".", pathVec, 0, directories, callback);
}

RelativeFileSystem::RelativeFileSystem(std::shared_ptr<IFileSystem> fs, const std::filesystem::path& basePath)
	: underlyingFS(std::move(fs)), basePath(basePath.lexically_normal()) {
}

bool RelativeFileSystem::FolderExists(const std::filesystem::path& name) {
	return underlyingFS->FolderExists(basePath / name.relative_path());
}

bool RelativeFileSystem::FileExists(const std::filesystem::path& name) {
	return underlyingFS->FileExists(basePath / name.relative_path());
}

size_t RelativeFileSystem::FileSize(const std::filesystem::path& name) {
	return underlyingFS->FileSize(basePath / name.relative_path());
}

std::unique_ptr<IBlob> RelativeFileSystem::ReadFile(const std::filesystem::path& name) {
	return underlyingFS->ReadFile(basePath / name.relative_path());
}

std::unique_ptr<IStreamBlob> RelativeFileSystem::StreamFile(const std::filesystem::path& name) {
	return underlyingFS->StreamFile(basePath / name.relative_path());
}

bool RelativeFileSystem::WriteFile(const std::filesystem::path& name, const void* data, size_t size) {
	return underlyingFS->WriteFile(basePath / name.relative_path(), data, size);
}

int RelativeFileSystem::EnumerateFiles(const std::filesystem::path& path, const std::vector<std::string>& extensions, EnumerateCallback callback, bool allowDuplicates) {
	return underlyingFS->EnumerateFiles(basePath / path.relative_path(), extensions, callback, allowDuplicates);
}

int RelativeFileSystem::EnumerateDirectories(const std::filesystem::path& path, EnumerateCallback callback, bool allowDuplicates) {
	return underlyingFS->EnumerateDirectories(basePath / path.relative_path(), callback, allowDuplicates);
}

void RootFileSystem::Mount(const std::filesystem::path& path, std::shared_ptr<IFileSystem> fs) {
	if (findMountPoint(path, nullptr, nullptr)) {
		engine::log::Fatal("unable to mount file system to path:\n%ls", path.c_str());
	}

	mountPoints.push_back(std::make_pair(path.lexically_normal().generic_string(), fs));
}

void engine::fs::RootFileSystem::Mount(const std::filesystem::path& path, const std::filesystem::path& nativePath) {
	Mount(path, std::make_shared<RelativeFileSystem>(std::make_shared<NativeFileSystem>(), nativePath));
}

bool RootFileSystem::Unmount(const std::filesystem::path& path) {
	std::string spath = path.lexically_normal().generic_string();
	for (size_t index = 0; index < mountPoints.size(); index++) {
		if (mountPoints[index].first == spath) {
			mountPoints.erase(mountPoints.begin() + index);
			return true;
		}
	}
	return false;
}

bool RootFileSystem::FolderExists(const std::filesystem::path& name) {
	std::filesystem::path relativePath;
	IFileSystem* fs = nullptr;
	if (findMountPoint(name, &relativePath, &fs)) {
		return fs->FolderExists(relativePath);
	}
	return false;
}

bool RootFileSystem::FileExists(const std::filesystem::path& name) {
	std::filesystem::path relativePath;
	IFileSystem* fs = nullptr;
	if (findMountPoint(name, &relativePath, &fs)) {
		return fs->FileExists(relativePath);
	}
	return false;
}

size_t RootFileSystem::FileSize(const std::filesystem::path& name) {
	std::filesystem::path relativePath;
	IFileSystem* fs = nullptr;
	if (findMountPoint(name, &relativePath, &fs)) {
		return fs->FileSize(relativePath);
	}
	return 0;
}

std::unique_ptr<IBlob> RootFileSystem::ReadFile(const std::filesystem::path& name) {
	std::filesystem::path relativePath;
	IFileSystem* fs = nullptr;
	if (findMountPoint(name, &relativePath, &fs)) {
		return fs->ReadFile(relativePath);
	}
	return nullptr;
}

std::unique_ptr<IStreamBlob> RootFileSystem::StreamFile(const std::filesystem::path& name) {
	std::filesystem::path relativePath;
	IFileSystem* fs = nullptr;
	if (findMountPoint(name, &relativePath, &fs)) {
		return fs->StreamFile(relativePath);
	}
	return nullptr;
}

bool RootFileSystem::WriteFile(const std::filesystem::path& name, const void* data, size_t size) {
	std::filesystem::path relativePath;
	IFileSystem* fs = nullptr;
	if (findMountPoint(name, &relativePath, &fs)) {
		return fs->WriteFile(relativePath, data, size);
	}
	return false;
}

int RootFileSystem::EnumerateFiles(const std::filesystem::path& path, const std::vector<std::string>& extensions, EnumerateCallback callback, bool allowDuplicates) {
	std::filesystem::path relativePath;
	IFileSystem* fs = nullptr;
	if (findMountPoint(path, &relativePath, &fs)) {
		return fs->EnumerateFiles(relativePath, extensions, callback, allowDuplicates);
	}
	return status::PathNotFound;
}

int RootFileSystem::EnumerateDirectories(const std::filesystem::path& path, EnumerateCallback callback, bool allowDuplicates) {
	std::filesystem::path relativePath;
	IFileSystem* fs = nullptr;
	if (findMountPoint(path, &relativePath, &fs)) {
		return fs->EnumerateDirectories(relativePath, callback, allowDuplicates);
	}
	return status::PathNotFound;
}

bool RootFileSystem::findMountPoint(const std::filesystem::path& path, std::filesystem::path* pRelativePath, IFileSystem** ppFS) {
	std::string spath = path.lexically_normal().generic_string();
	for (const auto& it: mountPoints) {
		if (spath.find(it.first, 0) == 0 && ((spath.length() == it.first.length()) || (spath[it.first.length()] == '/'))) {
			if (pRelativePath) {
				std::string relative = spath.substr(it.first.size() + 1);
				*pRelativePath = relative;
			}
			if (ppFS) {
				*ppFS = it.second.get();
			}
			return true;
		}
	}
	return false;
}