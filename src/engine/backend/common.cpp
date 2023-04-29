// Copyright © 2022-2023 Daylon Wilkins & James Cor
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <engine/backend/common.hpp>
#include <engine/physics/manager.hpp>
#include <engine/graphics/manager.hpp>
#include <engine/log/log.hpp>

engine::Application::Application() noexcept {
	commonImpl = std::make_unique<engine::Application::CommonImplementation>(this);
	inputHandler = std::make_unique<engine::input::Handler>(this);
}

engine::Application::~Application() {
	platImpl->ImGuiShutdown();
	// Explicitly remove commonImpl before platImpl to work around destructor timing
	commonImpl.reset();
	platImpl.reset();
}

engine::Application::CommonImplementation::CommonImplementation(Application* app) {
	this->application = app;
	guiBackend = std::make_unique<engine::gui::Backend>();
	guiRenderer = std::make_unique<engine::gui::Renderer>();
}

engine::Application::CommonImplementation::~CommonImplementation() {
	Rml::Shutdown();
	guiRenderer.reset();
	guiBackend.reset();
	engine::graphics::Terminate();
	engine::physics::Terminate();
}

bool engine::Application::CommonImplementation::Initialize() {
	// Create the window
	if (!application->platImpl->InitializeWindow()) {
		return false;
	}

	// Initialize renderer
	engine::graphics::Initialize(application->StartOptions(), application->platImpl->GetWindowHandle());

	// Initialize ImGUI
#ifndef ENABLE_IMGUI_INI
	ImGui::GetIO().IniFilename = nullptr;
#endif
	if (!application->platImpl->ImGuiInitialize()) {
		return false;
	}

	// Initialize RmlUi
	if (!application->platImpl->SetGUIBackend()) {
		return false;
	}
	if (!Rml::Initialise()) {
		engine::log::Error("Unable to initialize GUI");
		return false;
	}
	application->rmlContext = Rml::CreateContext("main", Rml::Vector2i((int)graphics::GlobalManager->GetWindowWidth(), (int)graphics::GlobalManager->GetWindowHeight()));
	if (!application->rmlContext) {
		engine::log::Error("Unable to create GUI context");
		return false;
	}

	// Initialize physics
	engine::physics::Initialize(application);

	// Initialize audio
	application->audioManager = std::make_unique<engine::audio::Manager>(new engine::fs::NativeFileSystem());

	// Initialize input
	if (!application->platImpl->InitializeInput()) {
		return false;
	}

	// Initialize the application
	if (!application->Initialize()) {
		engine::log::Error("Failed to initialize application");
		return false;
	}
	return true;
}

bool engine::Application::CommonImplementation::UpdateLoop() {
	// The frame start time is used to check how long a frame has taken, which will determine if we need to limit the
	// framerate. When the framerate is too high, unintended behavior occurs, so we force a cap.
	double frameStartTime = guiBackend->GetElapsedTime();

	application->inputHandler->Update();
	if (!application->platImpl->ProcessMessages()) {
		return false;
	}
	if (!application->platImpl->UpdateInput()) {
		return false;
	}

	// The delta time reports the time since the last check, so that events may properly update their logic
	double currentTime = guiBackend->GetElapsedTime();
	double deltaTime = currentTime - lastRecordedTime;
	lastRecordedTime = currentTime;

	engine::graphics::GlobalManager->NewFrame(deltaTime);
	if (!application->platImpl->ImGuiNewFrame()) {
		return false;
	}
	guiRenderer->FrameBegin();
	if (!application->Update(deltaTime)) {
		return false;
	}
	engine::physics::Update(deltaTime);
	if (!application->rmlContext->Update()) {
		return false;
	}
	if (!application->Draw(deltaTime)) {
		return false;
	}
	if (!application->rmlContext->Render()) {
		return false;
	}
	guiRenderer->FrameEnd();
	engine::graphics::GlobalManager->EndFrame();
	application->platImpl->WaitFor(engine::graphics::GlobalManager->GetMinimumFrameTime() - (guiBackend->GetElapsedTime() - frameStartTime));
	return true;
}

void engine::Application::CommonImplementation::HandleResize(std::uint32_t width, std::uint32_t height) {
	// This may be called before the manager has been created (on initial window creation), so we need to check that first
	if (engine::graphics::GlobalManager) {
		engine::graphics::GlobalManager->HandleResize(width, height);
		application->rmlContext->SetDimensions(Rml::Vector2i((int)width, (int)height));
	}
}
