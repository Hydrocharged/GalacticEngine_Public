// Copyright © 2022-2023 Daylon Wilkins & James Cor
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <engine/graphics/manager.hpp>

engine::graphics::Scene::Scene(engine::graphics::View* targetView) {
	view = targetView->Filament();
	scene = GlobalManager->GetEngine()->createScene();
	view->setScene(scene);
}

engine::graphics::Scene::Scene(filament::View* targetView) {
	view = targetView;
	scene = GlobalManager->GetEngine()->createScene();
	view->setScene(scene);
}

engine::graphics::Scene::~Scene() {
	view->setScene(nullptr);
	GlobalManager->GetEngine()->destroy(scene);
}

filament::Scene* engine::graphics::Scene::Filament() {
	return scene;
}
