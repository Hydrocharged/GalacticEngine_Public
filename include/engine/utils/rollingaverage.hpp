// Copyright © 2022-2023 Daylon Wilkins & James Cor
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef ENGINE_UTILS_AVERAGE_HPP
#define ENGINE_UTILS_AVERAGE_HPP

#include <type_traits>

namespace engine::utils {
	template<typename T, typename = std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>>>
	class RollingAverage {
	public:
		RollingAverage(std::size_t size) {
			values = new T[size];
			totalSize = size;
			for (std::size_t i = 0; i < size; i++) {
				values[i] = 0;
			}
		}
		~RollingAverage() {
			delete[] values;
		}

		void Update(T val) {
			values[index] = val;
			index = (index + 1) % totalSize;
		}
		T GetCurrentAverage() {
			double sum = 0;
			for (std::size_t i = 0; i < totalSize; i++) {
				sum += (double)values[i];
			}
			return (T)(sum / (double)totalSize);
		}

	private:
		T* values;
		std::size_t index = 0;
		std::size_t totalSize = 0;
	};
}

#endif //ENGINE_UTILS_AVERAGE_HPP
