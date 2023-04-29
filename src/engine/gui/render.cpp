// Copyright © 2022-2023 Daylon Wilkins & James Cor
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <engine/application.hpp>
#include <engine/gui/renderer.hpp>
#include <engine/graphics/manager.hpp>
#include <resources/resources.h>

engine::gui::Renderer::Renderer() : vertexFreeList(64), indexFreeList(64) {}

engine::gui::Renderer::~Renderer() {
	deletePending();
	auto engine = engine::graphics::GetEngine();
	engine->destroy(emptyTexture);
	engine->destroy(clearStencilMatInstance);
	for (auto setStencilMatInstance: setStencilMatInstances) {
		engine->destroy(setStencilMatInstance);
	}
	engine->destroy(material);
	engine->destroy(renderable);
	engine->destroy(vertexBuffer);
	engine->destroy(indexBuffer);
	auto& entityManager = ::utils::EntityManager::get();
	entityManager.destroy(renderable);
	delete (camera);
	delete (scene);
	delete (view);
}

void engine::gui::Renderer::initFilamentResources(filament::Engine& engine) {
	// Set up the view, scene, and camera
	view = new engine::graphics::View();
	scene = new engine::graphics::Scene(view);
	camera = new engine::graphics::Camera(view);
	auto fView = view->Filament();
	fView->setPostProcessingEnabled(false);
	fView->setBlendMode(filament::View::BlendMode::TRANSLUCENT);
	fView->setShadowingEnabled(false);
	fView->setStencilBufferEnabled(true);
	::utils::EntityManager::get().create(1, &renderable);
	scene->Filament()->addEntity(renderable);

	// Create the materials
	material = filament::Material::Builder()
		.package(RESOURCES_RMLUI_DATA, RESOURCES_RMLUI_SIZE)
		.build(engine);
	auto defaultInstance = material->getDefaultInstance();
	defaultInstance->setColorWrite(true);
	defaultInstance->setStencilWrite(false);
	defaultInstance->setStencilReadMask(0xFF);
	defaultInstance->setStencilWriteMask(0x00);
	defaultInstance->setStencilOpStencilFail(filament::MaterialInstance::StencilOperation::KEEP, filament::MaterialInstance::StencilFace::FRONT_AND_BACK);
	defaultInstance->setStencilOpDepthFail(filament::MaterialInstance::StencilOperation::KEEP, filament::MaterialInstance::StencilFace::FRONT_AND_BACK);
	defaultInstance->setStencilOpDepthStencilPass(filament::MaterialInstance::StencilOperation::KEEP, filament::MaterialInstance::StencilFace::FRONT_AND_BACK);
	defaultInstance->setStencilCompareFunction(filament::MaterialInstance::StencilCompareFunc::A, filament::MaterialInstance::StencilFace::FRONT_AND_BACK);
	defaultInstance->setStencilReferenceValue(1, filament::MaterialInstance::StencilFace::FRONT_AND_BACK);

	clearStencilMatInstance = material->createInstance();
	clearStencilMatInstance->setColorWrite(false);
	clearStencilMatInstance->setStencilWrite(true);
	clearStencilMatInstance->setStencilReadMask(0x00);
	clearStencilMatInstance->setStencilWriteMask(0xFF);
	clearStencilMatInstance->setStencilOpDepthStencilPass(filament::MaterialInstance::StencilOperation::ZERO, filament::MaterialInstance::StencilFace::FRONT_AND_BACK);

	auto setStencilMatInstance = material->createInstance();
	setStencilMatInstance->setColorWrite(false);
	setStencilMatInstance->setStencilWrite(true);
	setStencilMatInstance->setStencilReadMask(0x00);
	setStencilMatInstance->setStencilWriteMask(0xFF);
	setStencilMatInstance->setStencilOpDepthStencilPass(filament::MaterialInstance::StencilOperation::REPLACE, filament::MaterialInstance::StencilFace::FRONT_AND_BACK);
	constexpr int initialStencilMatCount = 16;
	setStencilMatInstances.reserve(initialStencilMatCount);
	setStencilMatInstances.push_back(setStencilMatInstance);
	makeStencilMaterials(initialStencilMatCount);

	// Create empty texture
	emptyTexture = filament::Texture::Builder()
		.width(1)
		.height(1)
		.sampler(filament::Texture::Sampler::SAMPLER_2D)
		.format(filament::Texture::InternalFormat::RGBA8)
		.build(engine);
	Rml::byte* data = new Rml::byte[]{255, 255, 255, 255};
	filament::Texture::PixelBufferDescriptor buffer{};
	buffer.buffer = data;
	buffer.size = 4;
	buffer.format = filament::Texture::Format::RGBA;
	buffer.type = filament::Texture::Type::UBYTE;
	buffer.alignment = 1;
	buffer.setCallback(releaseRmlData);
	emptyTexture->setImage(engine, 0, std::move(buffer));

	// Pipeline starts with the stencil disabled
	currentState = RenderState::NoStencil;
}

void engine::gui::Renderer::renderFrame() {
	if (frameGeometry.empty()) {
		return;
	}

	auto engine = engine::graphics::GetEngine();
	//TODO: probably don't want to do this every single frame, only if we've detected a width/height change
	double width = engine::graphics::GetWindowWidth();
	double height = engine::graphics::GetWindowHeight();
	camera->Filament()->setProjection(filament::Camera::Projection::ORTHO, 0.0, width, height, 0.0, -100000.0, 100000.0);
	view->Filament()->setViewport({0, 0, std::uint32_t(width), std::uint32_t(height)});

	if (vertexBufferCount < vertexFreeList.NumberOfElements()) {
		vertexBufferCount = vertexFreeList.NumberOfElements();
		engine->destroy(vertexBuffer);
		vertexBuffer = filament::VertexBuffer::Builder()
			.bufferCount(1)
			.vertexCount(vertexFreeList.NumberOfElements())
			.attribute(filament::VertexAttribute::POSITION, 0, filament::VertexBuffer::AttributeType::FLOAT2,
					   0, vertexFreeList.SizeOfElement)
			.attribute(filament::VertexAttribute::COLOR, 0, filament::VertexBuffer::AttributeType::UBYTE4,
					   sizeof(Rml::Vector2f), vertexFreeList.SizeOfElement)
			.attribute(filament::VertexAttribute::UV0, 0, filament::VertexBuffer::AttributeType::FLOAT2,
					   sizeof(Rml::Vector2f) + sizeof(Rml::Colourb), vertexFreeList.SizeOfElement)
			.normalized(filament::VertexAttribute::COLOR)
			.build(*engine);
	}
	if (indexBufferCount < indexFreeList.NumberOfElements()) {
		indexBufferCount = indexFreeList.NumberOfElements();
		engine->destroy(indexBuffer);
		indexBuffer = filament::IndexBuffer::Builder()
			.indexCount(indexFreeList.NumberOfElements())
			.bufferType(filament::IndexBuffer::IndexType::UINT)
			.build(*engine);
		// Filament only takes unsigned buffers, and RmlUi gives us int buffers, so we need to statically check that they're the same bit size
		static_assert(sizeof(int) == sizeof(std::uint32_t));
	}

	auto& renderableManager = engine->getRenderableManager();
	renderableManager.destroy(renderable);
	auto renderableBuilder = filament::RenderableManager::Builder(frameGeometry.size());
	renderableBuilder.boundingBox({{0,      0,      0},
								   {100000, 100000, 100000}}).culling(false);

	//TODO: maybe only set this if it has changed (or only update a small region instead of the whole buffer)
	void* vertexFreeListCopy = malloc(vertexFreeList.SizeOfUnderlyingData());
	void* indexFreeListCopy = malloc(indexFreeList.SizeOfUnderlyingData());
	memcpy(vertexFreeListCopy, vertexFreeList.UnderlyingData(), vertexFreeList.SizeOfUnderlyingData());
	memcpy(indexFreeListCopy, indexFreeList.UnderlyingData(), indexFreeList.SizeOfUnderlyingData());
	auto freeCallback = [](void* buffer, size_t size, void* user) { free(buffer); };
	vertexBuffer->setBufferAt(*engine, 0, filament::VertexBuffer::BufferDescriptor(vertexFreeListCopy, vertexFreeList.SizeOfUnderlyingData(), freeCallback));
	indexBuffer->setBuffer(*engine, filament::IndexBuffer::BufferDescriptor(indexFreeListCopy, indexFreeList.SizeOfUnderlyingData(), freeCallback));

	size_t primitiveIndex = 0;
	for (auto& geometry: frameGeometry) {
		filament::MaterialInstance* materialInstance = geometry.MaterialInstance;
		materialInstance->setParameter("transform", geometry.Transform);
		materialInstance->setParameter("translation", geometry.Translation);

		renderableBuilder.geometry(primitiveIndex, filament::RenderableManager::PrimitiveType::TRIANGLES,
								   vertexBuffer, indexBuffer, geometry.Indexes.Index, geometry.Indexes.Count)
			.blendOrder(primitiveIndex, primitiveIndex)
			.material(primitiveIndex, materialInstance);

		++primitiveIndex;
	}
	renderableBuilder.build(*engine, renderable);
}

void engine::gui::Renderer::FrameBegin() {
	frameGeometry.clear();
	setStencilMatInstanceIndex = 0;
}

void engine::gui::Renderer::FrameEnd() {
	renderFrame();
	deletePending();
}

void engine::gui::Renderer::makeStencilMaterials(int newCount) {
	auto templateInstance = setStencilMatInstances[0];
	for (int i = int(setStencilMatInstances.size()); i < newCount; ++i) {
		setStencilMatInstances.push_back(filament::MaterialInstance::duplicate(templateInstance));
	}
}

void engine::gui::Renderer::deletePending() {
	auto engine = engine::graphics::GetEngine();
	for (auto compiledGeometry: geometryToDelete) {
		vertexFreeList.Deallocate(compiledGeometry->Vertexes);
		indexFreeList.Deallocate(compiledGeometry->Indexes);
		if (compiledGeometry->State != RenderState::ClearStencil && compiledGeometry->State != RenderState::SetStencil) {
			engine->destroy(compiledGeometry->MaterialInstance);
		}
		delete (compiledGeometry);
	}
	geometryToDelete.clear();
	for (auto texture: texturesToDelete) {
		engine->destroy(texture);
	}
	texturesToDelete.clear();
}

void engine::gui::Renderer::releaseRmlData(void* data, size_t size, void* opaquePointer) {
	delete[](reinterpret_cast<Rml::byte*>(data));
}
