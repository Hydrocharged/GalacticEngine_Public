// Copyright © 2022-2023 Daylon Wilkins & James Cor
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <engine/application.hpp>
#include <engine/gui/renderer.hpp>
#include <stb_image.h>

void engine::gui::Renderer::Initialize(engine::Application* app) {
	application = app;
	filament::Engine& engine = *engine::graphics::GetEngine();
	initFilamentResources(engine);
}

Rml::CompiledGeometryHandle engine::gui::Renderer::CompileGeometry(Rml::Vertex* vertexes, int vertexCount, int* indexes, int indexCount, Rml::TextureHandle rmlTextureHandle) {
	auto vertexSection = vertexFreeList.Allocate(vertexCount);
	auto indexSection = indexFreeList.Allocate(indexCount);
	memcpy(vertexFreeList.SectionData(vertexSection), vertexes, sizeof(Rml::Vertex) * vertexCount);
	{
		auto indexSectionData = indexFreeList.SectionData(indexSection);
		auto vertexStartingIndex = (int)vertexSection.Index;
		for (int i = 0; i < indexCount; i++) {
			indexSectionData[i] = indexes[i] + vertexStartingIndex;
		}
	}

	filament::MaterialInstance* materialInstance;
	switch (currentState) {
		case RenderState::ClearStencil: {
			materialInstance = clearStencilMatInstance;
			break;
		}
		case RenderState::SetStencil: {
			if (setStencilMatInstanceIndex >= setStencilMatInstances.size()) {
				makeStencilMaterials(int(setStencilMatInstances.size()) * 2);
			}
			materialInstance = setStencilMatInstances[setStencilMatInstanceIndex];
			++setStencilMatInstanceIndex;
			break;
		}
		default: {
			materialInstance = material->createInstance();
			break;
		}
	}

	filament::Texture* texture = nullptr;
	filament::TextureSampler sampler(filament::TextureSampler::MinFilter::LINEAR_MIPMAP_LINEAR, filament::TextureSampler::MagFilter::LINEAR);
	if (rmlTextureHandle != 0) {
		texture = reinterpret_cast<filament::Texture*>(rmlTextureHandle);
		materialInstance->setParameter("albedo", texture, sampler);
	} else {
		materialInstance->setParameter("albedo", emptyTexture, sampler);
	}
	auto compiledGeometry = new RmlCompiledGeometry{
		.Transform = currentTransform,
		.Translation = {0, 0},
		.Texture = texture,
		.State = currentState,
		.MaterialInstance = materialInstance,
		.Vertexes = vertexSection,
		.Indexes = indexSection,
	};
	return reinterpret_cast<Rml::CompiledGeometryHandle>(compiledGeometry);
}

void engine::gui::Renderer::RenderCompiledGeometry(Rml::CompiledGeometryHandle geometryHandle, const Rml::Vector2f& translation) {
	auto compiledGeometry = reinterpret_cast<RmlCompiledGeometry*>(geometryHandle);
	auto compFunc = (currentState == RenderState::CompareStencil) ? filament::MaterialInstance::StencilCompareFunc::E : filament::MaterialInstance::StencilCompareFunc::A;
	compiledGeometry->MaterialInstance->setStencilCompareFunction(compFunc, filament::MaterialInstance::StencilFace::FRONT_AND_BACK);
	frameGeometry.push_back(engine::gui::RmlCompiledGeometry{
		.Transform = currentTransform,
		.Translation = {translation.x, translation.y},
		.Texture = compiledGeometry->Texture,
		.State = currentState,
		.MaterialInstance = compiledGeometry->MaterialInstance,
		.Vertexes = compiledGeometry->Vertexes,
		.Indexes = compiledGeometry->Indexes,
	});
}

void engine::gui::Renderer::ReleaseCompiledGeometry(Rml::CompiledGeometryHandle geometryHandle) {
	geometryToDelete.push_back(reinterpret_cast<RmlCompiledGeometry*>(geometryHandle));
}

void engine::gui::Renderer::RenderGeometry(Rml::Vertex* vertexes, int vertexCount, int* indexes, int indexCount, Rml::TextureHandle textureHandle, const Rml::Vector2f& translation) {
	auto geometryHandle = CompileGeometry(vertexes, vertexCount, indexes, indexCount, textureHandle);
	RenderCompiledGeometry(geometryHandle, translation);
	ReleaseCompiledGeometry(geometryHandle);
}

void engine::gui::Renderer::EnableScissorRegion(bool enable) {
	if (!enable) {
		currentState = RenderState::NoStencil;
	}
}

void engine::gui::Renderer::SetScissorRegion(int x, int y, int width, int height) {
	Rml::Vertex clearStencilVertices[3];
	clearStencilVertices[0].position = {0.0f, 0.0f};
	clearStencilVertices[0].colour = {255, 0, 0, 255};
	clearStencilVertices[1].position = {0.0f, 100000.0f};
	clearStencilVertices[1].colour = {255, 0, 0, 255};
	clearStencilVertices[2].position = {100000.0f, 0.0f};
	clearStencilVertices[2].colour = {255, 0, 0, 255};

	int clearStencilIndices[3] = {0, 2, 1};

	currentState = RenderState::ClearStencil;

	auto originalTransform = currentTransform;
	currentTransform = filament::math::mat4f();
	RenderGeometry(clearStencilVertices, 3, clearStencilIndices, 3, 0, Rml::Vector2f(0, 0));
	currentTransform = originalTransform;

	const auto left = float(x);
	const auto right = float(x + width);
	const auto top = float(y);
	const auto bottom = float(y + height);

	Rml::Vertex setStencilVertices[4];
	setStencilVertices[0].position = {left, top};
	setStencilVertices[0].colour = {0, 255, 0, 255};
	setStencilVertices[1].position = {right, top};
	setStencilVertices[1].colour = {0, 255, 0, 255};
	setStencilVertices[2].position = {right, bottom};
	setStencilVertices[2].colour = {0, 255, 0, 255};
	setStencilVertices[3].position = {left, bottom};
	setStencilVertices[3].colour = {0, 255, 0, 255};

	int setStencilIndices[6] = {0, 2, 1, 0, 3, 2};

	currentState = RenderState::SetStencil;

	RenderGeometry(setStencilVertices, 4, setStencilIndices, 6, 0, Rml::Vector2f(0, 0));

	currentState = RenderState::CompareStencil;
}

bool engine::gui::Renderer::LoadTexture(Rml::TextureHandle& textureHandle, Rml::Vector2i& textureDimensions, const Rml::String& source) {
	int width;
	int height;
	int channels; // Not used, the last parameter forces the image to have 4 channels
	unsigned char* data = stbi_load(source.c_str(), &width, &height, &channels, 4);
	if (!data) {
		engine::log::Error("RmlUi could not load the texture: %s", source.c_str());
		return false;
	}
	textureDimensions.x = (int)width;
	textureDimensions.y = (int)height;

	filament::Engine& engine = *engine::graphics::GetEngine();
	auto texture = filament::Texture::Builder()
		.width(width)
		.height(height)
		.levels(0xFF)
		.sampler(filament::Texture::Sampler::SAMPLER_2D)
		.format(filament::Texture::InternalFormat::RGBA8)
		.build(engine);
	filament::Texture::PixelBufferDescriptor buffer{};
	buffer.buffer = data;
	buffer.size = width * height * 4;
	buffer.format = filament::Texture::Format::RGBA;
	buffer.type = filament::Texture::Type::UBYTE;
	buffer.alignment = 1;
	buffer.setCallback((filament::Texture::PixelBufferDescriptor::Callback)&stbi_image_free);
	texture->setImage(engine, 0, std::move(buffer));
	texture->generateMipmaps(engine);

	textureHandle = reinterpret_cast<Rml::TextureHandle>(texture);
	return true;
}

bool engine::gui::Renderer::GenerateTexture(Rml::TextureHandle& textureHandle, const Rml::byte* data, const Rml::Vector2i& sourceDimensions) {
	size_t size = sourceDimensions.x * sourceDimensions.y * 4;
	Rml::byte* copiedData = new Rml::byte[sourceDimensions.x * sourceDimensions.y * 4];
	memcpy(copiedData, data, size);
	filament::Engine& engine = *engine::graphics::GetEngine();
	auto texture = filament::Texture::Builder()
		.width(sourceDimensions.x)
		.height(sourceDimensions.y)
		.levels(0xFF)
		.sampler(filament::Texture::Sampler::SAMPLER_2D)
		.format(filament::Texture::InternalFormat::RGBA8)
		.build(engine);
	filament::Texture::PixelBufferDescriptor buffer{};
	buffer.buffer = copiedData;
	buffer.size = size;
	buffer.format = filament::Texture::Format::RGBA;
	buffer.type = filament::Texture::Type::UBYTE;
	buffer.alignment = 1;
	buffer.setCallback(releaseRmlData);
	texture->setImage(engine, 0, std::move(buffer));
	texture->generateMipmaps(engine);

	textureHandle = reinterpret_cast<Rml::TextureHandle>(texture);
	return true;
}

void engine::gui::Renderer::ReleaseTexture(Rml::TextureHandle textureHandle) {
	texturesToDelete.push_back(reinterpret_cast<filament::Texture*>(textureHandle));
}

void engine::gui::Renderer::SetTransform(const Rml::Matrix4f* transform) {
	if (transform != nullptr) {
		currentTransform = *reinterpret_cast<filament::math::mat4f*>(const_cast<Rml::Matrix4f*>(transform));
	} else {
		currentTransform = filament::math::mat4f();
	}
}
