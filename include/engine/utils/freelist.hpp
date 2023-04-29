// Copyright © 2022-2023 Daylon Wilkins & James Cor
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef ENGINE_UTILS_FREELIST_HPP
#define ENGINE_UTILS_FREELIST_HPP

#include <engine/utils/memorypool.hpp>
#include <engine/log/log.hpp>
#include <typeindex>

namespace engine::utils {
	template<typename T, size_t MemoryPoolBlockSize = 4096, typename SizeT = size_t>
	class FreeList {
	public:
		FreeList(SizeT initialCount);
		~FreeList();

		struct Section {
		public:
			SizeT Index;
			SizeT Count;

			size_t SizeOfUnderlyingData();
		};

		Section Allocate(SizeT count);
		void Deallocate(Section section);
		// Returns a pointer to the data represented by this section. As the FreeList may change the location of its
		// internal data, users should not hold onto the returned pointer, as they may become invalid.
		T* SectionData(Section section);
		// Returns a pointer to the beginning of the underlying data.
		T* UnderlyingData();
		// Returns a pointer to the beginning of the underlying data.
		T* UnderlyingData(SizeT& numberOfElements);
		// Returns the total size, in bytes, of the underlying data.
		size_t SizeOfUnderlyingData();
		// Returns the total number of elements. This includes elements that have NOT been allocated.
		size_t NumberOfElements();
		// Returns the type of the underlying data.
		std::type_index GetUnderlyingType();

		// The size of a single element. Equivalent to sizeof(T).
		static constexpr size_t SizeOfElement = sizeof(T);

	private:
		struct LinkedSection {
		public:
			Section Section;
			LinkedSection* Next;
		};

		LinkedSection* head;
		engine::utils::MemoryPool<LinkedSection, MemoryPoolBlockSize> pool;
		T* data;
		SizeT totalCount;
	};

	template<size_t MemoryPoolBlockSize = 4096, typename SizeT = size_t>
	class FreeListNonBacking {
	public:
		FreeListNonBacking();
		FreeListNonBacking(SizeT capacity);
		~FreeListNonBacking();

		struct Section {
			SizeT Index;
			SizeT Count;
		};

		Section Allocate(SizeT count);
		void Deallocate(Section section);

	private:
		struct LinkedSection {
		public:
			Section Section;
			LinkedSection* Next;
		};

		LinkedSection* head;
		engine::utils::MemoryPool<LinkedSection, MemoryPoolBlockSize> pool;
	};
}

template<typename T, size_t MemoryPoolBlockSize, typename SizeT>
engine::utils::FreeList<T, MemoryPoolBlockSize, SizeT>::FreeList(SizeT initialCount) {
	head = pool.Allocate();
	head->Section.Index = 0;
	head->Section.Count = initialCount;
	head->Next = nullptr;
	data = new T[initialCount];
	totalCount = initialCount;
}

template<typename T, size_t MemoryPoolBlockSize, typename SizeT>
engine::utils::FreeList<T, MemoryPoolBlockSize, SizeT>::~FreeList() {
	LinkedSection* next = head->Next;
	pool.Deallocate(head);
	while (next != nullptr) {
		head = next;
		next = head->Next;
		pool.Deallocate(head);
	}
	delete[] data;
}

template<typename T, size_t MemoryPoolBlockSize, typename SizeT>
typename engine::utils::FreeList<T, MemoryPoolBlockSize, SizeT>::Section engine::utils::FreeList<T, MemoryPoolBlockSize, SizeT>::Allocate(SizeT count) {
	if (count == 0) [[unlikely]] {
		return Section{};
	}
	while (true) {
		LinkedSection* previous = nullptr;
		LinkedSection* current = head;
		while (current != nullptr) {
			if (current->Section.Count >= count) {
				Section section{
					.Index = current->Section.Index,
					.Count = count
				};
				current->Section.Index += count;
				current->Section.Count -= count;
				if (current->Section.Count == 0) {
					if (previous != nullptr) {
						previous->Next = current->Next;
						pool.Deallocate(current);
					} else if (current->Next != nullptr) {
						head = current->Next;
						pool.Deallocate(current);
					}
				}
				return section;
			}
			previous = current;
			current = previous->Next;
		}

		previous->Section.Count += totalCount;
		T* newData = new T[totalCount * 2];
		memcpy(newData, data, sizeof(T) * totalCount);
		delete[] data;
		data = newData;
		totalCount *= 2;
	}
}

template<typename T, size_t MemoryPoolBlockSize, typename SizeT>
void engine::utils::FreeList<T, MemoryPoolBlockSize, SizeT>::Deallocate(const engine::utils::FreeList<T, MemoryPoolBlockSize, SizeT>::Section section) {
	LinkedSection* previous = nullptr;
	LinkedSection* current = head;
	while (current != nullptr && section.Index > current->Section.Index) {
		previous = current;
		current = previous->Next;
	}

	if (previous == nullptr) {
		if (section.Index + section.Count == current->Section.Index) {
			current->Section.Index = section.Index;
			current->Section.Count += section.Count;
		} else {
			head = pool.Allocate();
			head->Section = section;
			head->Next = current;
		}
	} else if (current == nullptr) {
		if (previous->Section.Index + section.Count == section.Index) {
			previous->Section.Count += section.Count;
		} else {
			auto next = pool.Allocate();
			next->Section = section;
			previous->Next = next;
		}
	} else if (section.Index + section.Count == current->Section.Index) {
		current->Section.Index = section.Index;
		current->Section.Count += section.Count;
	} else if (previous->Section.Index + previous->Section.Count == section.Index) {
		previous->Section.Count += section.Count;
	} else {
		auto next = pool.Allocate();
		next->Section = section;
		next->Next = current;
		previous->Next = next;
	}
}

template<typename T, size_t MemoryPoolBlockSize, typename SizeT>
T* engine::utils::FreeList<T, MemoryPoolBlockSize, SizeT>::SectionData(const engine::utils::FreeList<T, MemoryPoolBlockSize, SizeT>::Section section) {
	return &data[section.Index];
}

template<typename T, size_t MemoryPoolBlockSize, typename SizeT>
T* engine::utils::FreeList<T, MemoryPoolBlockSize, SizeT>::UnderlyingData() {
	return data;
}

template<typename T, size_t MemoryPoolBlockSize, typename SizeT>
T* engine::utils::FreeList<T, MemoryPoolBlockSize, SizeT>::UnderlyingData(SizeT& numberOfElements) {
	*numberOfElements = totalCount;
	return data;
}

template<typename T, size_t MemoryPoolBlockSize, typename SizeT>
size_t engine::utils::FreeList<T, MemoryPoolBlockSize, SizeT>::SizeOfUnderlyingData() {
	return sizeof(T) * totalCount;
}

template<typename T, size_t MemoryPoolBlockSize, typename SizeT>
size_t engine::utils::FreeList<T, MemoryPoolBlockSize, SizeT>::NumberOfElements() {
	return totalCount;
}

template<typename T, size_t MemoryPoolBlockSize, typename SizeT>
std::type_index engine::utils::FreeList<T, MemoryPoolBlockSize, SizeT>::GetUnderlyingType() {
	return typeid(T);
}

template<typename T, size_t MemoryPoolBlockSize, typename SizeT>
size_t engine::utils::FreeList<T, MemoryPoolBlockSize, SizeT>::Section::SizeOfUnderlyingData() {
	return sizeof(T) * Count;
}

template<size_t MemoryPoolBlockSize, typename SizeT>
engine::utils::FreeListNonBacking<MemoryPoolBlockSize, SizeT>::FreeListNonBacking() {
	head = pool.Allocate();
	head->Section.Index = 0;
	head->Section.Count = std::numeric_limits<SizeT>::max();
	head->Next = nullptr;
}

template<size_t MemoryPoolBlockSize, typename SizeT>
engine::utils::FreeListNonBacking<MemoryPoolBlockSize, SizeT>::FreeListNonBacking(SizeT capacity) {
	head = pool.Allocate();
	head->Section.Index = 0;
	head->Section.Count = capacity;
	head->Next = nullptr;
}

template<size_t MemoryPoolBlockSize, typename SizeT>
engine::utils::FreeListNonBacking<MemoryPoolBlockSize, SizeT>::~FreeListNonBacking() {
	LinkedSection* next = head->Next;
	pool.Deallocate(head);
	while (next != nullptr) {
		head = next;
		next = head->Next;
		pool.Deallocate(head);
	}
}

template<size_t MemoryPoolBlockSize, typename SizeT>
typename engine::utils::FreeListNonBacking<MemoryPoolBlockSize, SizeT>::Section engine::utils::FreeListNonBacking<MemoryPoolBlockSize, SizeT>::Allocate(SizeT count) {
	if (count <= 0) [[unlikely]] {
		return Section{};
	}
	LinkedSection* previous = nullptr;
	LinkedSection* current = head;
	while (current != nullptr) {
		if (current->Section.Count >= count) {
			Section section{
				.Index = current->Section.Index,
				.Count = count
			};
			current->Section.Index += count;
			current->Section.Count -= count;
			if (current->Section.Count == 0) {
				if (previous != nullptr) {
					previous->Next = current->Next;
					pool.Deallocate(current);
				} else if (current->Next != nullptr) {
					head = current->Next;
					pool.Deallocate(current);
				}
			}
			return section;
		}
		previous = current;
		current = previous->Next;
	}
	engine::log::Fatal("FreeListNonBacking has run out of sections to allocate");
	return Section{};
}

template<size_t MemoryPoolBlockSize, typename SizeT>
void engine::utils::FreeListNonBacking<MemoryPoolBlockSize, SizeT>::Deallocate(const engine::utils::FreeListNonBacking<MemoryPoolBlockSize, SizeT>::Section section) {
	LinkedSection* previous = nullptr;
	LinkedSection* current = head;
	while (current != nullptr && section.Index > current->Section.Index) {
		previous = current;
		current = previous->Next;
	}

	if (previous == nullptr) {
		if (section.Index + section.Count == current->Section.Index) {
			current->Section.Index = section.Index;
			current->Section.Count += section.Count;
		} else {
			head = pool.Allocate();
			head->Section = section;
			head->Next = current;
		}
	} else if (current == nullptr) {
		if (previous->Section.Index + section.Count == section.Index) {
			previous->Section.Count += section.Count;
		} else {
			auto next = pool.Allocate();
			next->Section = section;
			previous->Next = next;
		}
	} else if (section.Index + section.Count == current->Section.Index) {
		current->Section.Index = section.Index;
		current->Section.Count += section.Count;
	} else if (previous->Section.Index + previous->Section.Count == section.Index) {
		previous->Section.Count += section.Count;
	} else {
		auto next = pool.Allocate();
		next->Section = section;
		next->Next = current;
		previous->Next = next;
	}
}

#endif //ENGINE_UTILS_FREELIST_HPP
