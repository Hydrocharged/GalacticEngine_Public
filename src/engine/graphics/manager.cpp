// Copyright © 2022-2023 Daylon Wilkins & James Cor
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <engine/application.hpp>
#include <engine/graphics/manager.hpp>
#include <filament/include/filament/SwapChain.h>

namespace engine::graphics {
	Manager* GlobalManager = nullptr;
}

engine::graphics::Manager::Manager(const engine::ApplicationOptions& options, void* windowHandle) {
	// Create the engine
	width = std::max(options.Width, std::uint32_t(320));
	height = std::max(options.Height, std::uint32_t(240));
	engine = filament::Engine::create(filament::Engine::Backend::DEFAULT);
	auto featureLevel = std::min(filament::backend::FeatureLevel::FEATURE_LEVEL_3, engine->getSupportedFeatureLevel());
	engine->setActiveFeatureLevel(featureLevel);
	swapChain = engine->createSwapChain(windowHandle);
	renderer = engine->createRenderer();
	renderer->setClearOptions(filament::Renderer::ClearOptions{
		.clearColor{0.0f, 0.0f, 0.0f, 0.0f},
		.clearStencil = 0,
		.clear = true,
		.discard = true,
	});

	// Set up ImGui
	imguiView = engine->createView();
	imguiView->setViewport({0, 0, options.Width, options.Height});
	imguiContext = ImGui::CreateContext();
	ImGui::SetCurrentContext(imguiContext);
	imGuiHelper = new filagui::ImGuiHelper(engine, imguiView, "", imguiContext);
	ImGuiIO& imguiIO = ImGui::GetIO();
	imguiIO.ImeWindowHandle = windowHandle;
	imguiIO.ClipboardUserData = nullptr;
	imGuiHelper->setDisplaySize(options.Width, options.Height);

	// Create the default view
	defaultView = engine->createView();
	defaultView->setViewport({0, 0, options.Width, options.Height});

	// Create the default scene
	defaultScene = engine->createScene();
	defaultView->setScene(defaultScene);

	// Create the default camera
	constexpr double cameraNear = 0.1;
	constexpr double cameraFar = 1000.0;
	::utils::EntityManager::get().create(1, &defaultCameraEntity);
	defaultCamera = engine->createCamera(defaultCameraEntity);
	defaultCamera->setExposure(16.0f, 1 / 125.0f, 100.0f);
	defaultCamera->setProjection(80.0f, double(options.Width) / double(options.Height), cameraNear, cameraFar, filament::Camera::Fov::VERTICAL);
	defaultCamera->lookAt({3, 3, 3}, {0, 0, 0}, {0, 1, 0});
	defaultView->setCamera(defaultCamera);
}

engine::graphics::Manager::~Manager() {
	engine->destroy(defaultCameraEntity);
	::utils::EntityManager::get().destroy(defaultCameraEntity);
	engine->destroy(defaultScene);
	engine->destroy(defaultView);
	delete (imGuiHelper);
	engine->destroy(imguiView);
	engine->destroy(renderer);
	engine->destroy(swapChain);
	filament::Engine::destroy(&engine);
}

void engine::graphics::Manager::HandleResize(std::uint32_t width, std::uint32_t height) {
	this->width = width;
	this->height = height;
	//TODO: update every view and every camera
	defaultView->setViewport({0, 0, width, height});
	imguiView->setViewport({0, 0, width, height});
	imGuiHelper->setDisplaySize(int(width), int(height));

	double fov = defaultCamera->getFieldOfViewInDegrees(filament::Camera::Fov::VERTICAL);
	double cameraNear = defaultCamera->getNear();
	double cameraFar = defaultCamera->getCullingFar();
	defaultCamera->setProjection(fov, double(width) / double(height), cameraNear, cameraFar, filament::Camera::Fov::VERTICAL);
}

void engine::graphics::Manager::NewFrame(double deltaTime) {
	if (!UTILS_HAS_THREADING) {
		engine->execute();
	}
	ImGui::SetCurrentContext(imguiContext);
	ImGui::GetIO().DeltaTime = float(deltaTime);
	ImGui::NewFrame();
	renderFrame = renderer->beginFrame(swapChain);
}

void engine::graphics::Manager::EndFrame() {
	ImGui::Render();
	imGuiHelper->processImGuiCommands(ImGui::GetDrawData(), ImGui::GetIO());
	if (renderFrame) {
		for (auto registeredView: registeredViews) {
			if (registeredView->IsEnabled()) {
				renderer->render(registeredView->Filament());
			}
		}
		renderer->render(imguiView);
		renderer->endFrame();
	}
}

void engine::graphics::Manager::RegisterView(engine::graphics::View* view) {
	registeredViews.push_back(view);
}

void engine::graphics::Manager::UnregisterView(engine::graphics::View* view) {
	auto position = std::find(registeredViews.begin(), registeredViews.end(), view);
	if (position != registeredViews.end()) {
		registeredViews.erase(position);
	}
}

void engine::graphics::Initialize(const engine::ApplicationOptions& options, void* windowHandle) {
	GlobalManager = new Manager(options, windowHandle);
}

void engine::graphics::Terminate() {
	delete (GlobalManager);
}
