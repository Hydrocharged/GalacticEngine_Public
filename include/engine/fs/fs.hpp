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

// Adapted from https://github.com/NVIDIAGameWorks/donut/blob/main/include/donut/core/vfs/VFS.h

#ifndef ENGINE_FS_FS_HPP
#define ENGINE_FS_FS_HPP

#include <memory>
#include <string>
#include <filesystem>
#include <functional>
#include <vector>

namespace engine::fs {
	namespace status {
		constexpr int OK = 0;
		constexpr int Failed = -1;
		constexpr int PathNotFound = -2;
		constexpr int NotImplemented = -3;
	}

	typedef const std::function<void(std::string_view)>& EnumerateCallback;

	inline std::function<void(std::string_view)> EnumerateToVector(std::vector<std::string>& v) {
		return [&v](std::string_view s) { v.emplace_back(std::string(s)); };
	}

	// IBlobIdentifiable is the base that all blobs are subclassed from. Returns the type of blob, so that it may be
	// cast to its appropriate type.
	class IBlobIdentifiable {
	public:
		enum class BlobType {
			Blob,
			StreamBlob,
		};
		virtual BlobType Type() = 0;
	};

	// A blob is a package for untyped data, typically read from a file.
	class IBlob : public IBlobIdentifiable {
	public:
		virtual ~IBlob() = default;
		[[nodiscard]] virtual const void* Data() const = 0;
		[[nodiscard]] virtual size_t Size() const = 0;

		static bool IsEmpty(const IBlob& blob) {
			return blob.Data() == nullptr || blob.Size() == 0;
		}

		BlobType Type() final { return BlobType::Blob; }
	};

	// A stream blob is a package for streaming untyped data, typically read from a file.
	class IStreamBlob : public IBlobIdentifiable {
	public:
		virtual ~IStreamBlob() = default;
		virtual void Reset() = 0;
		virtual void Seek(size_t position) = 0;
		[[nodiscard]] virtual const std::string& Name() const = 0;
		[[nodiscard]] virtual size_t Size() const = 0;
		[[nodiscard]] virtual size_t Position() const = 0;
		[[nodiscard]] virtual bool HasMore() const = 0;

		// Returns the next chunk of data, up to the given blob size.
		// Returns nullptr if no more chunks may be read.
		[[nodiscard]] virtual std::unique_ptr<IBlob> Next(size_t blobSize) = 0;

		BlobType Type() final { return BlobType::StreamBlob; }
	};

	// Specific blob implementation that owns the data and frees it when deleted.
	class Blob : public IBlob {
	private:
		void* blobData;
		size_t blobSize;

	public:
		Blob(void* data, size_t size);
		~Blob() override;
		[[nodiscard]] const void* Data() const override;
		[[nodiscard]] size_t Size() const override;
	};

	// Specific stream blob implementation that owns the stream and frees it when deleted.
	class StreamBlob : public IStreamBlob {
	private:
		std::ifstream* filestream;
		std::string streamName;
		size_t streamSize;
		size_t streamIndex;

	public:
		StreamBlob(std::string name, std::ifstream* stream, size_t size);
		~StreamBlob() override;
		void Reset() override;
		void Seek(size_t position) override;
		[[nodiscard]] const std::string& Name() const override;
		[[nodiscard]] size_t Size() const override;
		[[nodiscard]] size_t Position() const override;
		[[nodiscard]] bool HasMore() const override;
		[[nodiscard]] std::unique_ptr<IBlob> Next(size_t blobSize) override;
	};

	// Basic interface for the virtual file system.
	class IFileSystem {
	public:
		virtual ~IFileSystem() = default;

		// Check if a folder exists.
		virtual bool FolderExists(const std::filesystem::path& name) = 0;

		// Check if a file exists.
		virtual bool FileExists(const std::filesystem::path& name) = 0;

		// Read the file's size.
		virtual size_t FileSize(const std::filesystem::path& name) = 0;

		// Read the entire file.
		// Returns nullptr if the file cannot be read.
		virtual std::unique_ptr<IBlob> ReadFile(const std::filesystem::path& name) = 0;

		// Stream the file.
		// Returns nullptr if the file cannot be read.
		virtual std::unique_ptr<IStreamBlob> StreamFile(const std::filesystem::path& name) = 0;

		// Write the entire file.
		// Returns false if the file cannot be written.
		virtual bool WriteFile(const std::filesystem::path& name, const void* data, size_t size) = 0;

		// Search for files with any of the provided `extensions` in `path`.
		// Extensions should not include any wildcard characters.
		// Returns the number of files found, or a negative number on errors - see engine::fs::status.
		// The file names, relative to the 'path', are passed to 'callback' in no particular order.
		virtual int EnumerateFiles(const std::filesystem::path& path, const std::vector<std::string>& extensions, EnumerateCallback callback, bool allowDuplicates = false) = 0;

		// Search for directories in 'path'.
		// Returns the number of directories found, or a negative number on errors - see engine::fs::status.
		// The directory names, relative to the 'path', are passed to 'callback' in no particular order.
		virtual int EnumerateDirectories(const std::filesystem::path& path, EnumerateCallback callback, bool allowDuplicates = false) = 0;
	};

	// An implementation of the virtual file system that directly maps to the OS files.
	class NativeFileSystem : public IFileSystem {
	public:
		bool FolderExists(const std::filesystem::path& name) override;
		bool FileExists(const std::filesystem::path& name) override;
		size_t FileSize(const std::filesystem::path& name) override;
		std::unique_ptr<IBlob> ReadFile(const std::filesystem::path& name) override;
		std::unique_ptr<IStreamBlob> StreamFile(const std::filesystem::path& name) override;
		bool WriteFile(const std::filesystem::path& name, const void* data, size_t size) override;
		int EnumerateFiles(const std::filesystem::path& path, const std::vector<std::string>& extensions, EnumerateCallback callback, bool allowDuplicates = false) override;
		int EnumerateDirectories(const std::filesystem::path& path, EnumerateCallback callback, bool allowDuplicates = false) override;
	private:
		int enumerateNativeFiles(const char* pattern, bool directories, EnumerateCallback callback);
	};

	// A layer that represents some path in the underlying file system as an entire FS.
	// Effectively, just prepends the provided base path to every file name and passes
	// the requests to the underlying FS.
	class RelativeFileSystem : public IFileSystem {
	private:
		std::shared_ptr<IFileSystem> underlyingFS;
		std::filesystem::path basePath;
	public:
		RelativeFileSystem(std::shared_ptr<IFileSystem> fs, const std::filesystem::path& basePath);

		[[nodiscard]] const std::filesystem::path& GetBasePath() const { return basePath; }

		bool FolderExists(const std::filesystem::path& name) override;
		bool FileExists(const std::filesystem::path& name) override;
		size_t FileSize(const std::filesystem::path& name) override;
		std::unique_ptr<IBlob> ReadFile(const std::filesystem::path& name) override;
		std::unique_ptr<IStreamBlob> StreamFile(const std::filesystem::path& name) override;
		bool WriteFile(const std::filesystem::path& name, const void* data, size_t size) override;
		int EnumerateFiles(const std::filesystem::path& path, const std::vector<std::string>& extensions, EnumerateCallback callback, bool allowDuplicates = false) override;
		int EnumerateDirectories(const std::filesystem::path& path, EnumerateCallback callback, bool allowDuplicates = false) override;
	};

	// A virtual file system that allows mounting, or attaching, other FS objects to paths.
	// Does not have any file systems by default.
	class RootFileSystem : public IFileSystem {
	private:
		std::vector<std::pair<std::string, std::shared_ptr<IFileSystem>>> mountPoints;

		bool findMountPoint(const std::filesystem::path& path, std::filesystem::path* pRelativePath, IFileSystem** ppFS);
	public:
		void Mount(const std::filesystem::path& path, std::shared_ptr<IFileSystem> fs);
		void Mount(const std::filesystem::path& path, const std::filesystem::path& nativePath);
		bool Unmount(const std::filesystem::path& path);

		bool FolderExists(const std::filesystem::path& name) override;
		bool FileExists(const std::filesystem::path& name) override;
		size_t FileSize(const std::filesystem::path& name) override;
		std::unique_ptr<IBlob> ReadFile(const std::filesystem::path& name) override;
		std::unique_ptr<IStreamBlob> StreamFile(const std::filesystem::path& name) override;
		bool WriteFile(const std::filesystem::path& name, const void* data, size_t size) override;
		int EnumerateFiles(const std::filesystem::path& path, const std::vector<std::string>& extensions, EnumerateCallback callback, bool allowDuplicates = false) override;
		int EnumerateDirectories(const std::filesystem::path& path, EnumerateCallback callback, bool allowDuplicates = false) override;
	};

	std::filesystem::path GetDirectoryWithExecutable();
}

#endif //ENGINE_FS_FS_HPP
