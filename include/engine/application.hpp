// Copyright © 2022-2023 Daylon Wilkins & James Cor
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef ENGINE_APPLICATION_HPP
#define ENGINE_APPLICATION_HPP

#include <engine/audio/audio.hpp>
#include <engine/input/input.hpp>
#include <engine/physics/physics.hpp>
#include <engine/fs/fs.hpp>
#include <engine/log/log.hpp>
#include <engine/strings/strings.hpp>
#include <engine/graphics/graphics.hpp>
#include <engine/utils/utils.hpp>
#include <RmlUi/Core.h>

namespace engine {
	struct ApplicationOptions {
		std::string Title;
		std::uint32_t Width;
		std::uint32_t Height;
	};

	class Application {
	public:
		class CommonImplementation;

		class PlatformImplementation;

		Application() noexcept;
		virtual ~Application();

		Rml::Context* GetUIContext() { return rmlContext; }
		engine::audio::Manager* GetAudioManager() { return audioManager.get(); }
		engine::input::Handler* GetInputHandler() { return inputHandler.get(); }
		double GetElapsedTime();

		virtual bool Initialize() = 0;
		virtual void Shutdown() = 0;
		virtual bool Update(double deltaTime) = 0;
		virtual bool FixedUpdate(double deltaTime) = 0;
		virtual bool Draw(double deltaTime) = 0;
		virtual ApplicationOptions StartOptions() = 0;

	private:
		friend class CommonImplementation;

		friend class PlatformImplementation;

		friend class engine::input::Handler;

		std::unique_ptr<CommonImplementation> commonImpl;
		std::unique_ptr<PlatformImplementation> platImpl;
		std::unique_ptr<engine::audio::Manager> audioManager;
		std::unique_ptr<engine::input::Handler> inputHandler;
		Rml::Context* rmlContext = nullptr;
	};

	Application* NewApplication();
}

#endif //ENGINE_APPLICATION_HPP
