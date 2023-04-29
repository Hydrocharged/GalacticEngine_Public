// Copyright © 2022-2023 Daylon Wilkins & James Cor
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef ENGINE_BACKEND_COMMON_HPP
#define ENGINE_BACKEND_COMMON_HPP

#include <engine/application.hpp>
#include <engine/gui/backend.hpp>
#include <engine/gui/renderer.hpp>
#include <engine/graphics/manager.hpp>

namespace engine {
	class Application::PlatformImplementation {
	public:
		PlatformImplementation(Application* app);
		~PlatformImplementation();

		CommonImplementation* GetCommonImplementation();
		bool InitializeWindow();
		bool InitializeInput();
		bool SetGUIBackend();
		bool ProcessMessages();
		bool UpdateInput();
		void* GetWindowHandle() const;
		bool ImGuiInitialize();
		bool ImGuiNewFrame();
		void ImGuiShutdown();
		void WaitFor(double seconds);

		class PlatformCallbacks;

		struct PlatformData;

		std::unique_ptr<PlatformCallbacks> Callbacks;
		std::unique_ptr<PlatformData> Data;

	private:
		friend class PlatformCallbacks;

		friend class engine::input::Handler;

		friend class Application;

		engine::Application* application;
	};

	class Application::CommonImplementation {
	public:
		CommonImplementation(Application* app);
		~CommonImplementation();

		bool Initialize();
		bool UpdateLoop();
		void HandleResize(std::uint32_t width, std::uint32_t height);

	private:
		friend class PlatformImplementation;

		friend class PlatformImplementation::PlatformCallbacks;

		friend class engine::input::Handler;

		friend class Application;

		engine::Application* application;
		std::unique_ptr<engine::gui::Backend> guiBackend;
		std::unique_ptr<engine::gui::Renderer> guiRenderer;
		double lastRecordedTime = 0.0;
	};
}

#endif //ENGINE_BACKEND_COMMON_HPP
