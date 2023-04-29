// Copyright © 2022-2023 Daylon Wilkins & James Cor
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/*-
 * Copyright (c) 2013 Cosku Acay, http://www.coskuacay.com
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

// Adapted from https://github.com/cacay/MemoryPool/blob/master/C-11/MemoryPool.h
// and https://github.com/cacay/MemoryPool/blob/master/C-11/MemoryPool.tcc

#ifndef ENGINE_UTILS_MEMORYPOOL_HPP
#define ENGINE_UTILS_MEMORYPOOL_HPP

#include <cstdint>
#include <utility>

namespace engine::utils {
	template<typename T, size_t BlockSize = 4096>
	class MemoryPool {
	public:
		MemoryPool() noexcept;
		MemoryPool(const MemoryPool& memoryPool) noexcept;
		MemoryPool(MemoryPool&& memoryPool) noexcept;
		template<class U>
		MemoryPool(const MemoryPool<U>& memoryPool) noexcept;

		~MemoryPool() noexcept;

		MemoryPool& operator=(const MemoryPool& memoryPool) = delete;
		MemoryPool& operator=(MemoryPool&& memoryPool) noexcept;

		T* Address(T& x) const noexcept;
		const T* Address(const T& x) const noexcept;

		T* Allocate();
		void Deallocate(T* p);

		size_t MaxSize() const noexcept;

		template<class U, class... Args>
		void Construct(U* p, Args&& ... args);
		template<class U>
		void Destroy(U* p);

		template<class... Args>
		T* New(Args&& ... args);
		void Delete(T* p);

	private:
		typedef T value_type;
		typedef T* pointer;
		typedef T& reference;
		typedef const T* const_pointer;
		typedef const T& const_reference;
		typedef size_t size_type;
		typedef ptrdiff_t difference_type;

		union Slot_ {
			T element;
			Slot_* next;
		};

		typedef char* data_pointer_;
		typedef Slot_ slot_type_;
		typedef Slot_* slot_pointer_;

		slot_pointer_ currentBlock_;
		slot_pointer_ currentSlot_;
		slot_pointer_ lastSlot_;
		slot_pointer_ freeSlots_;

		size_t padPointer(data_pointer_ p, size_t align) const noexcept;
		void allocateBlock();

		static_assert(BlockSize >= 2 * sizeof(slot_type_), "BlockSize too small");
	};
}

using engine::utils::MemoryPool;

template<typename T, size_t BlockSize>
inline typename MemoryPool<T, BlockSize>::size_type MemoryPool<T, BlockSize>::padPointer(data_pointer_ p, size_type align) const noexcept {
	uintptr_t result = reinterpret_cast<uintptr_t>(p);
	return ((align - result) % align);
}

template<typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::MemoryPool() noexcept {
	currentBlock_ = nullptr;
	currentSlot_ = nullptr;
	lastSlot_ = nullptr;
	freeSlots_ = nullptr;
}

template<typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::MemoryPool(const MemoryPool& memoryPool) noexcept : MemoryPool() {}

template<typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::MemoryPool(MemoryPool&& memoryPool) noexcept {
	currentBlock_ = memoryPool.currentBlock_;
	memoryPool.currentBlock_ = nullptr;
	currentSlot_ = memoryPool.currentSlot_;
	lastSlot_ = memoryPool.lastSlot_;
	freeSlots_ = memoryPool.freeSlots;
}

template<typename T, size_t BlockSize>
template<class U>
MemoryPool<T, BlockSize>::MemoryPool(const MemoryPool<U>& memoryPool) noexcept : MemoryPool() {}

template<typename T, size_t BlockSize>
MemoryPool<T, BlockSize>& MemoryPool<T, BlockSize>::operator=(MemoryPool&& memoryPool) noexcept {
	if (this != &memoryPool) {
		std::swap(currentBlock_, memoryPool.currentBlock_);
		currentSlot_ = memoryPool.currentSlot_;
		lastSlot_ = memoryPool.lastSlot_;
		freeSlots_ = memoryPool.freeSlots;
	}
	return *this;
}

template<typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::~MemoryPool() noexcept {
	slot_pointer_ curr = currentBlock_;
	while (curr != nullptr) {
		slot_pointer_ prev = curr->next;
		operator delete(reinterpret_cast<void*>(curr));
		curr = prev;
	}
}

template<typename T, size_t BlockSize>
inline typename MemoryPool<T, BlockSize>::pointer MemoryPool<T, BlockSize>::Address(reference x) const noexcept {
	return &x;
}

template<typename T, size_t BlockSize>
inline typename MemoryPool<T, BlockSize>::const_pointer MemoryPool<T, BlockSize>::Address(const_reference x) const noexcept {
	return &x;
}

template<typename T, size_t BlockSize>
void MemoryPool<T, BlockSize>::allocateBlock() {
	// Allocate space for the new block and store a pointer to the previous one
	data_pointer_ newBlock = reinterpret_cast<data_pointer_>
	(operator new(BlockSize));
	reinterpret_cast<slot_pointer_>(newBlock)->next = currentBlock_;
	currentBlock_ = reinterpret_cast<slot_pointer_>(newBlock);
	// Pad block body to satisfy the alignment requirements for elements
	data_pointer_ body = newBlock + sizeof(slot_pointer_);
	size_type bodyPadding = padPointer(body, alignof(slot_type_));
	currentSlot_ = reinterpret_cast<slot_pointer_>(body + bodyPadding);
	lastSlot_ = reinterpret_cast<slot_pointer_>
	(newBlock + BlockSize - sizeof(slot_type_) + 1);
}

template<typename T, size_t BlockSize>
inline typename MemoryPool<T, BlockSize>::pointer MemoryPool<T, BlockSize>::Allocate() {
	if (freeSlots_ != nullptr) {
		pointer result = reinterpret_cast<pointer>(freeSlots_);
		freeSlots_ = freeSlots_->next;
		return result;
	} else {
		if (currentSlot_ >= lastSlot_) {
			allocateBlock();
		}
		return reinterpret_cast<pointer>(currentSlot_++);
	}
}

template<typename T, size_t BlockSize>
inline void MemoryPool<T, BlockSize>::Deallocate(pointer p) {
	if (p != nullptr) {
		reinterpret_cast<slot_pointer_>(p)->next = freeSlots_;
		freeSlots_ = reinterpret_cast<slot_pointer_>(p);
	}
}

template<typename T, size_t BlockSize>
inline typename MemoryPool<T, BlockSize>::size_type MemoryPool<T, BlockSize>::MaxSize() const noexcept {
	size_type maxBlocks = -1 / BlockSize;
	return (BlockSize - sizeof(data_pointer_)) / sizeof(slot_type_) * maxBlocks;
}

template<typename T, size_t BlockSize>
template<class U, class... Args>
inline void MemoryPool<T, BlockSize>::Construct(U* p, Args&& ... args) {
	new(p) U(std::forward<Args>(args)...);
}

template<typename T, size_t BlockSize>
template<class U>
inline void MemoryPool<T, BlockSize>::Destroy(U* p) {
	p->~U();
}

template<typename T, size_t BlockSize>
template<class... Args>
inline typename MemoryPool<T, BlockSize>::pointer MemoryPool<T, BlockSize>::New(Args&& ... args) {
	pointer result = Allocate();
	Construct<value_type>(result, std::forward<Args>(args)...);
	return result;
}

template<typename T, size_t BlockSize>
inline void MemoryPool<T, BlockSize>::Delete(pointer p) {
	if (p != nullptr) {
		p->~value_type();
		Deallocate(p);
	}
}

#endif //ENGINE_UTILS_MEMORYPOOL_HPP
