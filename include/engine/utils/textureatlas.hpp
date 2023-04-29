// Copyright © 2022-2023 Daylon Wilkins & James Cor
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef ENGINE_UTILS_TEXTUREATLAS_HPP
#define ENGINE_UTILS_TEXTUREATLAS_HPP

#include <engine/log/log.hpp>
#include <bit>
#include <unordered_set>
#include <vector>
#include <stack>

namespace engine::utils {
	template<std::uint16_t AtlasSize = 4096, std::uint16_t MaxTextureSize = 1024, std::uint16_t MinTextureSize = 64>
	class TextureAtlas {
	public:
		class Chunklet {
		public:
			std::uint16_t ChunkID = 0;
			std::uint16_t ChunkletID = 0;
			std::uint16_t X = 0;
			std::uint16_t Y = 0;
			std::uint16_t Width = 1;
			std::uint16_t Height = 1;
			std::uint16_t Size = MinTextureSize;
		};

		class Chunk {
		public:
			std::uint16_t ChunkID = 0;
			std::vector<std::uint16_t> AvailableChunkletIDs;
		};

		class ChunkManager {
		public:
			std::uint16_t Size = 0;
			std::vector<std::uint16_t> PartialChunks;
			// No need to know if a full Chunk belongs to the Manager, as the assignment can be used to figure that out
		};

		TextureAtlas();
		~TextureAtlas();

		Chunklet Allocate(std::uint16_t width, std::uint16_t height);
		void Deallocate(Chunklet chunklet);

	private:
		static const std::uint16_t maxBitWidth = std::bit_width(MaxTextureSize);
		static const std::uint16_t minBitWidth = std::bit_width(MinTextureSize);
		static const std::uint16_t chunkCount = ((std::uint64_t)AtlasSize * AtlasSize) / ((std::uint64_t)MaxTextureSize * MaxTextureSize);
		static const std::uint16_t chunksPerRow = AtlasSize / MaxTextureSize;

		void getChunkletCoordinates(std::uint16_t chunkletSize, std::uint16_t chunkID, std::uint16_t chunkletID, std::uint16_t& x, std::uint16_t& y);

		Chunk* chunks;
		ChunkManager* managers;
		std::uint8_t* chunkAssignments;
		std::stack<std::uint16_t> freeChunks;

		// Assert that each size is a power of 2
		static_assert(AtlasSize == std::bit_ceil(AtlasSize));
		static_assert(MaxTextureSize == std::bit_ceil(MaxTextureSize));
		static_assert(MinTextureSize == std::bit_ceil(MinTextureSize));
	};

	template<std::uint16_t AtlasSize, std::uint16_t MaxTextureSize, std::uint16_t MinTextureSize>
	TextureAtlas<AtlasSize, MaxTextureSize, MinTextureSize>::TextureAtlas() {
		chunks = new Chunk[chunkCount]();
		for (std::uint16_t i = 0; i < chunkCount; i++) {
			chunks[i].ChunkID = i;
			freeChunks.push(chunkCount - i - 1);
		}
		std::uint16_t managerCount = (maxBitWidth - minBitWidth) + 1;
		managers = new ChunkManager[managerCount]();
		for (std::uint16_t width = minBitWidth; width <= maxBitWidth; width++) {
			managers[width - minBitWidth].Size = 1 << (width - 1);
		}
		chunkAssignments = new std::uint8_t[chunkCount];
	}

	template<std::uint16_t AtlasSize, std::uint16_t MaxTextureSize, std::uint16_t MinTextureSize>
	TextureAtlas<AtlasSize, MaxTextureSize, MinTextureSize>::~TextureAtlas() {
		delete[] chunks;
		delete[] managers;
		delete[] chunkAssignments;
	}

	template<std::uint16_t AtlasSize, std::uint16_t MaxTextureSize, std::uint16_t MinTextureSize>
	typename TextureAtlas<AtlasSize, MaxTextureSize, MinTextureSize>::Chunklet TextureAtlas<AtlasSize, MaxTextureSize, MinTextureSize>::Allocate(std::uint16_t width, std::uint16_t height) {
		std::uint16_t size = std::bit_ceil(std::max(width, height));
		if (size > MaxTextureSize) {
			engine::log::Fatal("Texture atlas supports a maximum size of %u but was given %ux%u", MaxTextureSize, width, height);
		} else if (size < MinTextureSize) {
			size = MinTextureSize;
		}
		std::uint8_t managerIdx = std::bit_width(size) - minBitWidth;
		ChunkManager& manager = managers[managerIdx];
		if (!manager.PartialChunks.empty()) {
			Chunk& chunk = chunks[manager.PartialChunks.back()];
			auto chunkletID = chunk.AvailableChunkletIDs.back();
			chunk.AvailableChunkletIDs.pop_back();
			if (chunk.AvailableChunkletIDs.empty()) {
				manager.PartialChunks.pop_back();
			}
			std::uint16_t x;
			std::uint16_t y;
			getChunkletCoordinates(manager.Size, chunk.ChunkID, chunkletID, x, y);
			return Chunklet{
				.ChunkID = chunk.ChunkID,
				.ChunkletID = chunkletID,
				.X = x,
				.Y = y,
				.Width = width,
				.Height = height,
				.Size = size,
			};
		} else if (!freeChunks.empty()) {
			Chunk& chunk = chunks[freeChunks.top()];
			freeChunks.pop();
			chunkAssignments[chunk.ChunkID] = managerIdx;
			auto numOfChunklets = MaxTextureSize / size;
			numOfChunklets *= numOfChunklets;
			chunk.AvailableChunkletIDs.reserve(numOfChunklets);
			for (auto i = numOfChunklets - 1; i >= 0; i--) {
				chunk.AvailableChunkletIDs.push_back(i);
			}
			manager.PartialChunks.push_back(chunk.ChunkID);
			return Allocate(width, height);
		} else {
			engine::log::Fatal("Texture atlas has run out of free chunks");
			return Chunklet{};
		}
	}

	template<std::uint16_t AtlasSize, std::uint16_t MaxTextureSize, std::uint16_t MinTextureSize>
	void TextureAtlas<AtlasSize, MaxTextureSize, MinTextureSize>::Deallocate(Chunklet chunklet) {
		ChunkManager& manager = managers[chunkAssignments[chunklet.ChunkID]];
		Chunk& chunk = chunks[chunklet.ChunkID];
		auto numOfChunklets = MaxTextureSize / manager.Size;
		numOfChunklets *= numOfChunklets;
		chunk.AvailableChunkletIDs.push_back(chunklet.ChunkletID);
		if (chunk.AvailableChunkletIDs.size() == numOfChunklets) {
			freeChunks.push(chunklet.ChunkID);
			chunk.AvailableChunkletIDs.clear();
			for (auto it = manager.PartialChunks.begin(); it < manager.PartialChunks.end(); it++) {
				if (*it == chunklet.ChunkID) {
					manager.PartialChunks.erase(it);
					break;
				}
			}
		} else if (chunk.AvailableChunkletIDs.size() == 1) {
			// Was empty, so we need to add to the partial
			manager.PartialChunks.push_back(chunklet.ChunkID);
		}
	}

	template<std::uint16_t AtlasSize, std::uint16_t MaxTextureSize, std::uint16_t MinTextureSize>
	void TextureAtlas<AtlasSize, MaxTextureSize, MinTextureSize>::getChunkletCoordinates(std::uint16_t chunkletSize, std::uint16_t chunkID, std::uint16_t chunkletID, std::uint16_t& x, std::uint16_t& y) {
		auto chunkX = MaxTextureSize * (chunkID % chunksPerRow);
		auto chunkY = MaxTextureSize * (chunkID / chunksPerRow);
		auto chunkletsPerRow = MaxTextureSize / chunkletSize;
		x = chunkX + (chunkletSize * (chunkletID % chunkletsPerRow));
		y = chunkY + (chunkletSize * (chunkletID / chunkletsPerRow));
	}
}

#endif //ENGINE_UTILS_TEXTUREATLAS_HPP
