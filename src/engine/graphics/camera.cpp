// Copyright © 2022-2023 Daylon Wilkins & James Cor
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <engine/graphics/manager.hpp>

engine::graphics::Camera::Camera(engine::graphics::View* targetView) : Camera(targetView->Filament()) {}

engine::graphics::Camera::Camera(filament::View* targetView) {
	constexpr double cameraNear = 0.1;
	constexpr double cameraFar = 1000.0;

	view = targetView;
	::utils::EntityManager::get().create(1, &entity);
	camera = GlobalManager->GetEngine()->createCamera(entity);
	camera->setExposure(16.0f, 1 / 125.0f, 100.0f);
	camera->setProjection(80.0f, double(GlobalManager->GetWindowWidth()) / double(GlobalManager->GetWindowHeight()), cameraNear, cameraFar, filament::Camera::Fov::VERTICAL);
	view->setCamera(camera);
}

engine::graphics::Camera::~Camera() {
	auto engine = GlobalManager->GetEngine();
	view->setCamera(nullptr);
	engine->destroyCameraComponent(entity);
	engine->destroy(entity);
}

filament::Camera* engine::graphics::Camera::Filament() {
	return camera;
}
