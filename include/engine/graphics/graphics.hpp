// Copyright © 2022-2023 Daylon Wilkins & James Cor
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef ENGINE_GRAPHICS_GRAPHICS_HPP
#define ENGINE_GRAPHICS_GRAPHICS_HPP

#include <engine/utils/utils.hpp>
#include <vector>
#include <typeindex>

#include <filament/Camera.h>
#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/LightManager.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/Renderer.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/Skybox.h>
#include <filament/TextureSampler.h>
#include <filament/TransformManager.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>
#include <filament/Viewport.h>
#include <filagui/ImGuiHelper.h>
#include <utils/EntityManager.h>
#include <third_party/imgui/imgui.h>

namespace engine::graphics {
	std::uint32_t GetWindowWidth();
	std::uint32_t GetWindowHeight();
	filament::Engine* GetEngine();
	filament::Camera* GetDefaultCamera();
	filament::Scene* GetDefaultScene();
	filament::View* GetDefaultView();
	bool IsVSyncEnabled();
	int GetFramerateLimit();

	void SetVSync(bool enabled);
	void SetFramerateLimit(int limit);

	class View {
	public:
		View();
		~View();
		filament::View* Filament();

		bool IsEnabled();
		void SetEnabled(bool enabled);

	private:
		filament::View* view;
		bool enabled = true;
	};

	class Scene {
	public:
		Scene(engine::graphics::View* targetView);
		Scene(filament::View* targetView);
		~Scene();
		filament::Scene* Filament();

	private:
		filament::Scene* scene;
		filament::View* view; // Does not own the view, simply here for reference
	};

	class Camera {
	public:
		Camera(engine::graphics::View* targetView);
		Camera(filament::View* targetView);
		~Camera();
		filament::Camera* Filament();

	private:
		::utils::Entity entity;
		filament::Camera* camera;
		filament::View* view; // Does not own the view, simply here for reference
	};
}

#endif //ENGINE_GRAPHICS_GRAPHICS_HPP
