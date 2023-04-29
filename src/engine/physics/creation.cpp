// Copyright © 2022-2023 Daylon Wilkins & James Cor
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <engine/physics/manager.hpp>
#include <engine/log/log.hpp>

#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/TaperedCapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>

std::unique_ptr<engine::physics::Body> engine::physics::CreateSphere(float radius, const BodyCreationProperties properties) {
	return std::move(GlobalManager->CreateSphere(radius, properties));
}

std::unique_ptr<engine::physics::Body> engine::physics::CreateBox(glm::vec3 shape, const BodyCreationProperties properties) {
	return std::move(GlobalManager->CreateBox(shape, properties));
}

std::unique_ptr<engine::physics::Body> engine::physics::CreateCapsule(float height, float radius, const BodyCreationProperties properties) {
	return std::move(GlobalManager->CreateCapsule(height, radius, properties));
}

std::unique_ptr<engine::physics::Body> engine::physics::CreateTaperedCapsule(float height, float topRadius, float bottomRadius, const BodyCreationProperties properties) {
	return std::move(GlobalManager->CreateTaperedCapsule(height, topRadius, bottomRadius, properties));
}

std::unique_ptr<engine::physics::Body> engine::physics::CreateCylinder(float height, float radius, const BodyCreationProperties properties) {
	return std::move(GlobalManager->CreateCylinder(height, radius, properties));
}

std::unique_ptr<engine::physics::Character> engine::physics::CreateCharacter(CharacterCreationProperties properties) {
	return std::move(GlobalManager->CreateCharacter(properties));
}

std::unique_ptr<engine::physics::Body> engine::physics::Manager::CreateSphere(float radius, const BodyCreationProperties properties) {
	auto shapeSettings = JPH::SphereShapeSettings(radius);
	if (properties.Mass.Density >= std::numeric_limits<float>::epsilon()) {
		shapeSettings.SetDensity(properties.Mass.Density);
	} else if (properties.Mass.Weight >= std::numeric_limits<float>::epsilon()) {
		// Convert the mass to a uniform density, adapted from shape->GetMassProperties()
		shapeSettings.SetDensity(properties.Mass.Weight / ((4.0f / 3.0f * JPH::JPH_PI) * radius * radius * radius));
	}
	JPH::ShapeSettings::ShapeResult result{};
	auto shape = new JPH::SphereShape(shapeSettings, result); // Pointer memory is handled by Jolt
	if (result.HasError()) {
		engine::log::Error("Error creating SphereShape: %s", result.GetError().c_str());
		delete (shape);
		return nullptr;
	}
	return std::move(CreateBody(shape, properties));
}

std::unique_ptr<engine::physics::Body> engine::physics::Manager::CreateBox(glm::vec3 boxShape, const BodyCreationProperties properties) {
	// The convex radius must be smaller than the smallest side
	auto halfExtents = toJPH(boxShape * 0.5f);
	auto minSide = halfExtents.ReduceMin();
	auto shapeSettings = JPH::BoxShapeSettings(halfExtents,
											   (minSide <= JPH::cDefaultConvexRadius) ? std::nextafter(minSide, -1.0f) : JPH::cDefaultConvexRadius);
	if (properties.Mass.Density >= std::numeric_limits<float>::epsilon()) {
		shapeSettings.SetDensity(properties.Mass.Density);
	} else if (properties.Mass.Weight >= std::numeric_limits<float>::epsilon()) {
		// Convert the mass to a uniform density, adapted from shape->GetMassProperties()
		shapeSettings.SetDensity(properties.Mass.Weight / (boxShape.x * boxShape.y * boxShape.z));
	}
	JPH::ShapeSettings::ShapeResult result{};
	auto shape = new JPH::BoxShape(shapeSettings, result); // Pointer memory is handled by Jolt
	if (result.HasError()) {
		engine::log::Error("Error creating BoxShape: %s", result.GetError().c_str());
		delete (shape);
		return nullptr;
	}
	return std::move(CreateBody(shape, properties));
}

std::unique_ptr<engine::physics::Body> engine::physics::Manager::CreateCapsule(float height, float radius, const BodyCreationProperties properties) {
	auto shapeSettings = JPH::CapsuleShapeSettings(height / 2.0f, radius);
	if (properties.Mass.Density >= std::numeric_limits<float>::epsilon()) {
		shapeSettings.SetDensity(properties.Mass.Density);
	} else if (properties.Mass.Weight >= std::numeric_limits<float>::epsilon()) {
		// Convert the mass to a uniform density, adapted from shape->GetMassProperties()
		float radiusSq = radius * radius;
		float cylinder_mass = JPH::JPH_PI * height * radiusSq;
		float hemisphere_mass = (2.0f * JPH::JPH_PI / 3.0f) * 2.0f * radiusSq * radius;
		shapeSettings.SetDensity(properties.Mass.Weight / (cylinder_mass + hemisphere_mass));
	}
	JPH::ShapeSettings::ShapeResult result{};
	auto shape = new JPH::CapsuleShape(shapeSettings, result); // Pointer memory is handled by Jolt
	if (result.HasError()) {
		engine::log::Error("Error creating CapsuleShape: %s", result.GetError().c_str());
		delete (shape);
		return nullptr;
	}
	return std::move(CreateBody(shape, properties));
}

std::unique_ptr<engine::physics::Body> engine::physics::Manager::CreateTaperedCapsule(float height, float topRadius, float bottomRadius, const BodyCreationProperties properties) {
	float halfHeight = height / 2.0f;
	auto shapeSettings = JPH::TaperedCapsuleShapeSettings(halfHeight, topRadius, bottomRadius);
	if (properties.Mass.Density >= std::numeric_limits<float>::epsilon()) {
		shapeSettings.SetDensity(properties.Mass.Density);
	} else if (properties.Mass.Weight >= std::numeric_limits<float>::epsilon()) {
		// Convert the mass to a uniform density, adapted from shape->GetMassProperties()
		float mTopCenter = halfHeight + 0.5f * (shapeSettings.mBottomRadius - shapeSettings.mTopRadius);
		float mBottomCenter = -halfHeight + 0.5f * (shapeSettings.mBottomRadius - shapeSettings.mTopRadius);
		float avgRadius = 0.5f * (shapeSettings.mTopRadius + shapeSettings.mBottomRadius);
		auto box = JPH::AABox(JPH::Vec3(-avgRadius, mBottomCenter - shapeSettings.mBottomRadius, -avgRadius),
							  JPH::Vec3(avgRadius, mTopCenter + shapeSettings.mTopRadius, avgRadius));
		auto boxSize = box.GetSize();
		shapeSettings.SetDensity(properties.Mass.Weight / (boxSize.GetX() * boxSize.GetY() * boxSize.GetZ()));
	}
	JPH::ShapeSettings::ShapeResult result{};
	auto shape = new JPH::TaperedCapsuleShape(shapeSettings, result); // Pointer memory is handled by Jolt
	if (result.HasError()) {
		engine::log::Error("Error creating TaperedCapsuleShape: %s", result.GetError().c_str());
		delete (shape);
		return nullptr;
	}
	return std::move(CreateBody(shape, properties));
}

std::unique_ptr<engine::physics::Body> engine::physics::Manager::CreateCylinder(float height, float radius, const BodyCreationProperties properties) {
	auto shapeSettings = JPH::CylinderShapeSettings(height / 2.0f, radius);
	if (properties.Mass.Density >= std::numeric_limits<float>::epsilon()) {
		shapeSettings.SetDensity(properties.Mass.Density);
	} else if (properties.Mass.Weight >= std::numeric_limits<float>::epsilon()) {
		// Convert the mass to a uniform density, adapted from shape->GetMassProperties()
		shapeSettings.SetDensity(properties.Mass.Weight / (JPH::JPH_PI * height * radius * radius));
	}
	JPH::ShapeSettings::ShapeResult result{};
	auto shape = new JPH::CylinderShape(shapeSettings, result); // Pointer memory is handled by Jolt
	if (result.HasError()) {
		engine::log::Error("Error creating CylinderShape: %s", result.GetError().c_str());
		delete (shape);
		return nullptr;
	}
	return std::move(CreateBody(shape, properties));
}


//TODO: create joints
//TODO: create height field shape
