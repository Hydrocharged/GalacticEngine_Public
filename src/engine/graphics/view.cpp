// Copyright © 2022-2023 Daylon Wilkins & James Cor
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <engine/graphics/manager.hpp>

engine::graphics::View::View() {
	view = GlobalManager->GetEngine()->createView();
	GlobalManager->RegisterView(this);
}

engine::graphics::View::~View() {
	GlobalManager->UnregisterView(this);
	GlobalManager->GetEngine()->destroy(view);
}

filament::View* engine::graphics::View::Filament() {
	return view;
}

bool engine::graphics::View::IsEnabled() {
	return enabled;
}

void engine::graphics::View::SetEnabled(bool e) {
	this->enabled = e;
}
