// Copyright © 2022-2023 Daylon Wilkins & James Cor
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef ENGINE_GUI_INTERFACES_HPP
#define ENGINE_GUI_INTERFACES_HPP

#include <engine/utils/freelist.hpp>
#include <engine/graphics/graphics.hpp>
#include <RmlUi/Core.h>
#include <math/mat4.h>

namespace engine {
	class Application;
}

namespace engine::gui {
	enum class RenderState {
		NoStencil = 0,
		ClearStencil = 1,
		SetStencil = 2,
		CompareStencil = 3,
	};

	struct RmlCompiledGeometry {
	public:
		filament::math::mat4f Transform;
		filament::math::float2 Translation;
		filament::Texture* Texture = nullptr;
		RenderState State = RenderState::NoStencil;
		filament::MaterialInstance* MaterialInstance = nullptr;
		engine::utils::FreeList<Rml::Vertex, 1024>::Section Vertexes{};
		engine::utils::FreeList<int, 1024>::Section Indexes{};
	};

	class Renderer : public Rml::RenderInterface {
	public:
		Renderer();
		~Renderer() override;

		void Initialize(engine::Application* application);
		void FrameBegin();
		void FrameEnd();

		Rml::CompiledGeometryHandle CompileGeometry(Rml::Vertex* vertexes, int vertexCount, int* indexes, int indexCount, Rml::TextureHandle textureHandle) override;
		void RenderCompiledGeometry(Rml::CompiledGeometryHandle geometryHandle, const Rml::Vector2f& translation) override;
		void ReleaseCompiledGeometry(Rml::CompiledGeometryHandle geometryHandle) override;
		void RenderGeometry(Rml::Vertex* vertexes, int vertexCount, int* indexes, int indexCount, Rml::TextureHandle textureHandle, const Rml::Vector2f& translation) override;
		void EnableScissorRegion(bool enable) override;
		void SetScissorRegion(int x, int y, int width, int height) override;
		bool LoadTexture(Rml::TextureHandle& textureHandle, Rml::Vector2i& textureDimensions, const Rml::String& source) override;
		bool GenerateTexture(Rml::TextureHandle& textureHandle, const Rml::byte* data, const Rml::Vector2i& sourceDimensions) override;
		void ReleaseTexture(Rml::TextureHandle textureHandle) override;
		void SetTransform(const Rml::Matrix4f* transform) override;

	private:
		void initFilamentResources(filament::Engine& engine);
		void renderFrame();
		void makeStencilMaterials(int newCount);
		void deletePending();
		static void releaseRmlData(void*, size_t, void*);

		engine::Application* application = nullptr;
		filament::Texture* emptyTexture;
		engine::utils::FreeList<Rml::Vertex, 1024> vertexFreeList;
		engine::utils::FreeList<int, 1024> indexFreeList;
		std::vector<RmlCompiledGeometry> frameGeometry;
		std::vector<RmlCompiledGeometry*> geometryToDelete;
		std::vector<filament::Texture*> texturesToDelete;
		filament::math::mat4f currentTransform = filament::math::mat4f();
		RenderState currentState{};

		engine::graphics::View* view = nullptr;
		engine::graphics::Scene* scene = nullptr;
		engine::graphics::Camera* camera = nullptr;
		::utils::Entity renderable{};
		filament::Material* material = nullptr;
		filament::MaterialInstance* clearStencilMatInstance = nullptr;
		std::vector<filament::MaterialInstance*> setStencilMatInstances{};
		int setStencilMatInstanceIndex = 0;
		filament::VertexBuffer* vertexBuffer = nullptr;
		filament::IndexBuffer* indexBuffer = nullptr;
		std::uint32_t vertexBufferCount = 0;
		std::uint32_t indexBufferCount = 0;
	};
}

#endif //ENGINE_GUI_INTERFACES_HPP
