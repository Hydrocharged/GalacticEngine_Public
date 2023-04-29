// Copyright © 2022-2023 Daylon Wilkins & James Cor
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <engine/graphics/manager.hpp>

std::uint32_t engine::graphics::GetWindowWidth() {
	return GlobalManager->GetWindowWidth();
}

std::uint32_t engine::graphics::GetWindowHeight() {
	return GlobalManager->GetWindowHeight();
}

filament::Engine* engine::graphics::GetEngine() {
	return GlobalManager->GetEngine();
}

filament::Camera* engine::graphics::GetDefaultCamera() {
	return GlobalManager->GetDefaultCamera();
}

filament::Scene* engine::graphics::GetDefaultScene() {
	return GlobalManager->GetDefaultScene();
}

filament::View* engine::graphics::GetDefaultView() {
	return GlobalManager->GetDefaultView();
}

void engine::graphics::SetVSync(bool enabled) {
	GlobalManager->SetVSync(enabled);
}

void engine::graphics::SetFramerateLimit(int limit) {
	GlobalManager->SetFramerateLimit(limit);
}

bool engine::graphics::IsVSyncEnabled() {
	return GlobalManager->IsVSyncEnabled();
}

int engine::graphics::GetFramerateLimit() {
	return GlobalManager->GetFramerateLimit();
}
