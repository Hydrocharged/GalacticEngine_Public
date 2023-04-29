// Copyright © 2022-2023 Daylon Wilkins & James Cor
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <engine/application.hpp>
#include <engine/backend/common.hpp>

double engine::Application::GetElapsedTime() {
	return commonImpl->guiBackend->GetElapsedTime();
}
