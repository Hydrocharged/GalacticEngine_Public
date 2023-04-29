// Copyright © 2022-2023 Daylon Wilkins & James Cor
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef ENGINE_GRAPHICS_MANAGER_HPP
#define ENGINE_GRAPHICS_MANAGER_HPP

#include <engine/graphics/graphics.hpp>
#include <glm/glm.hpp>

namespace engine {
	struct ApplicationOptions;
}

namespace engine::graphics {
	// Initialize the rendering engine. Called internally by the engine.
	void Initialize(const engine::ApplicationOptions& options, void* windowHandle);
	// Terminates the rendering engine. Called internally by the engine.
	void Terminate();

	static constexpr std::uint32_t MinimumWindowWidth = 320;
	static constexpr std::uint32_t MinimumWindowHeight = 240;

	class Manager {
	public:
		Manager(const engine::ApplicationOptions& options, void* windowHandle);
		~Manager();

		filament::Engine* GetEngine() { return engine; }
		filament::Camera* GetDefaultCamera() { return defaultCamera; }
		filament::Scene* GetDefaultScene() { return defaultScene; }
		filament::View* GetDefaultView() { return defaultView; }

		[[nodiscard]] std::uint32_t GetWindowWidth() const { return width; }
		[[nodiscard]] std::uint32_t GetWindowHeight() const { return height; }
		[[nodiscard]] bool IsVSyncEnabled() const { return vsync; }
		[[nodiscard]] int GetFramerateLimit() const { return int(glm::round(1.0 / minimumFrameTime)); }
		[[nodiscard]] double GetMinimumFrameTime() const { return minimumFrameTime; }

		void SetVSync(bool enabled) { vsync = enabled; }
		void SetFramerateLimit(int limit) { minimumFrameTime = 1.0 / double(std::min(5000, std::max(15, limit))); }

		void HandleResize(std::uint32_t width, std::uint32_t height);
		void NewFrame(double deltaTime);
		void EndFrame();
		void RegisterView(engine::graphics::View* view);
		void UnregisterView(engine::graphics::View* view);

	private:
		bool vsync = true; //TODO: need to hook this up to whatever controls VSync in filament
		double minimumFrameTime = 1.0 / 2000.0;
		filament::Engine* engine = nullptr;
		filament::SwapChain* swapChain = nullptr;
		filament::Renderer* renderer = nullptr;
		::utils::Entity defaultCameraEntity;
		filament::Camera* defaultCamera = nullptr;
		filament::Scene* defaultScene = nullptr;
		filament::View* defaultView = nullptr;
		filament::View* imguiView = nullptr;
		filagui::ImGuiHelper* imGuiHelper = nullptr;
		ImGuiContext* imguiContext = nullptr;
		std::uint32_t width = 0;
		std::uint32_t height = 0;
		std::vector<engine::graphics::View*> registeredViews{};
		bool renderFrame = true;
	};

	extern Manager* GlobalManager;
}

#endif //ENGINE_GRAPHICS_MANAGER_HPP
