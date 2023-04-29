// Copyright © 2022-2023 Daylon Wilkins & James Cor
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef ENGINE_PHYSICS_MANAGER_HPP
#define ENGINE_PHYSICS_MANAGER_HPP

#include <engine/application.hpp>

// Must include "Jolt.h" before including any other Jolt header.
#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Character/Character.h>

namespace engine::physics {
	// Initialize the physics engine. Called internally by the engine.
	void Initialize(engine::Application* application);
	// Calculates a physics step. Called internally by the engine.
	void Update(double deltaTime);
	// Terminates the physics engine. Called internally by the engine.
	void Terminate();

	// Layers that objects can be in, which determines the other objects it can collide with.
	namespace Layers {
		static constexpr JPH::uint8 NON_MOVING = 0;
		static constexpr JPH::uint8 MOVING = 1;
		static constexpr JPH::uint8 NUM_LAYERS = 2;
	}

	// Each broad phase layer results in a separate bounding volume tree in the broad phase. Having a layer for
	// non-moving and another for moving objects avoids having to update a tree full of static objects every frame.
	namespace BroadPhaseLayers {
		static constexpr JPH::BroadPhaseLayer NONMOVING(0);
		static constexpr JPH::BroadPhaseLayer MOVING(1);
		static constexpr JPH::uint NUM_LAYERS(2);
	}

	class BroadPhaseLayerImpl final : public JPH::BroadPhaseLayerInterface {
	public:
		BroadPhaseLayerImpl();
		[[nodiscard]] JPH::uint GetNumBroadPhaseLayers() const override;
		[[nodiscard]] JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer layer) const override;

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
		[[nodiscard]] const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer layer) const override;
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

	private:
		JPH::BroadPhaseLayer objToBP[Layers::NUM_LAYERS] = {BroadPhaseLayers::NONMOVING, BroadPhaseLayers::MOVING};
	};

	class ObjectVsBroadPhaseLayerFilterImpl final : public JPH::ObjectVsBroadPhaseLayerFilter {
		[[nodiscard]] bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override;
	};

	class ObjectLayerPairFilterImpl final : public JPH::ObjectLayerPairFilter {
		[[nodiscard]] bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::ObjectLayer inLayer2) const override;
	};

	class InternalContactListener : public JPH::ContactListener {
	public:
		JPH::ValidateResult OnContactValidate(const JPH::Body& inBody1, const JPH::Body& inBody2, JPH::RVec3Arg inBaseOffset, const JPH::CollideShapeResult& inCollisionResult) override;
		void OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override;
		void OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override;
		void OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair) override;
	};

	class Manager {
	public:
		Manager(engine::Application* application);
		~Manager();
		void Enable();
		void Disable();
		void Update(double deltaTime);
		void SetUpdateRate(double rate);
		void OptimizeBroadPhase();

		glm::vec3 GetGravity();
		void SetGravity(glm::vec3 gravity);
		std::vector<RayResult> CastRay(glm::vec3 origin, glm::vec3 direction, RayFilter filter);

		// Returns a new Body defined by the given shape. Will return a nullptr once the max body count has been reached.
		std::unique_ptr<Body> CreateBody(JPH::Shape* shape, BodyCreationProperties properties);
		// Returns a new sphere. Will return a nullptr once the max body count has been reached.
		std::unique_ptr<Body> CreateSphere(float radius, BodyCreationProperties properties);
		// Returns a new box. Will return a nullptr once the max body count has been reached.
		std::unique_ptr<Body> CreateBox(glm::vec3 boxShape, BodyCreationProperties properties);
		// Returns a new capsule. Will return a nullptr once the max body count has been reached.
		std::unique_ptr<Body> CreateCapsule(float height, float radius, BodyCreationProperties properties);
		// Returns a new tapered capsule. Will return a nullptr once the max body count has been reached.
		std::unique_ptr<Body> CreateTaperedCapsule(float height, float topRadius, float bottomRadius, BodyCreationProperties properties);
		// Returns a new cylinder. Will return a nullptr once the max body count has been reached.
		std::unique_ptr<Body> CreateCylinder(float height, float radius, BodyCreationProperties properties);

		// Returns a new character. Will return a nullptr once the max body count has been reached.
		std::unique_ptr<Character> CreateCharacter(CharacterCreationProperties properties);
	private:
		friend class Body;

		std::unique_ptr<JPH::JobSystemThreadPool> jobSystem;
		std::unique_ptr<BroadPhaseLayerImpl> broadPhaseLayerImpl;
		std::unique_ptr<ObjectVsBroadPhaseLayerFilterImpl> objectVsBroadPhaseLayerFilterImpl;
		std::unique_ptr<ObjectLayerPairFilterImpl> objectLayerPairFilterImpl;
		std::unique_ptr<JPH::TempAllocatorImpl> tempAllocator;
		std::unique_ptr<JPH::PhysicsSystem> physicsSystem;
		std::unique_ptr<InternalContactListener> contactListener; //TODO: add a Set function for the new contact listener
		engine::Application* application;

		// 60Hz is the default rate for physics calculations.
		double maxDeltaTimeStep = 1.0 / 60.0;
		float maxDeltaTimeStepf = (float)maxDeltaTimeStep;
		bool enabled = true;
	};

	extern Manager* GlobalManager;
}

// Helpers -------------------------------------------------------------------------------------------------------------
inline glm::vec3 toGLM(JPH::Vec3 vec) {
	return {vec.GetX(), vec.GetY(), vec.GetZ()};
}

inline glm::quat toGLM(JPH::Quat quat) {
	static_assert(sizeof(glm::quat) == sizeof(JPH::Quat));
	return {quat.GetW(), quat.GetX(), quat.GetY(), quat.GetZ()};
}

inline glm::mat4 toGLM(JPH::Mat44 mat) {
	static_assert(sizeof(glm::mat4) == sizeof(JPH::Mat44));
	return *reinterpret_cast<glm::mat4*>(&mat);
}

inline engine::physics::MotionType toGLM(JPH::EMotionType motionType) {
	switch (motionType) {
		case JPH::EMotionType::Static:
			return engine::physics::MotionType::Static;
		case JPH::EMotionType::Kinematic:
			return engine::physics::MotionType::Kinematic;
		case JPH::EMotionType::Dynamic:
			return engine::physics::MotionType::Dynamic;
		default:
			engine::log::Fatal("Additional physics MotionType that has not been accounted for");
			return engine::physics::MotionType::Static;
	}
}

inline engine::physics::Shape toGLM(JPH::EShapeSubType shape) {
	switch (shape) {
		case JPH::EShapeSubType::Sphere:
			return engine::physics::Shape::Sphere;
		case JPH::EShapeSubType::Box:
			return engine::physics::Shape::Box;
		case JPH::EShapeSubType::RotatedTranslated: // RotatedTranslated is only used for character capsules
		case JPH::EShapeSubType::Capsule:
			return engine::physics::Shape::Capsule;
		case JPH::EShapeSubType::TaperedCapsule:
			return engine::physics::Shape::TaperedCapsule;
		case JPH::EShapeSubType::Cylinder:
			return engine::physics::Shape::Cylinder;
		default:
			engine::log::Fatal("Additional physics Shape that has not been accounted for");
			return engine::physics::Shape::Capsule;
	}
}

inline engine::physics::MotionQuality toGLM(JPH::EMotionQuality quality) {
	return (quality == JPH::EMotionQuality::Discrete) ? engine::physics::MotionQuality::Discrete : engine::physics::MotionQuality::LinearCast;
}

inline engine::physics::GroundState toGLM(JPH::Character::EGroundState groundState) {
	switch (groundState) {
		case JPH::Character::EGroundState::OnGround:
			return engine::physics::GroundState::Normal;
		case JPH::Character::EGroundState::OnSteepGround:
			return engine::physics::GroundState::Steep;
		case JPH::Character::EGroundState::NotSupported:
			return engine::physics::GroundState::Unstable;
		case JPH::Character::EGroundState::InAir:
			return engine::physics::GroundState::Airborne;
		default:
			engine::log::Fatal("Additional GroundState that has not been accounted for");
			return engine::physics::GroundState::Normal;
	}
}

inline JPH::Vec3 toJPH(glm::vec3 vec) {
	return {vec.x, vec.y, vec.z};
}

inline JPH::Quat toJPH(glm::quat quat) {
	return {quat.x, quat.y, quat.z, quat.w};
}

inline JPH::Mat44 toJPH(glm::mat4 mat) {
	static_assert(sizeof(JPH::Mat44) == sizeof(glm::mat4));
	return *reinterpret_cast<JPH::Mat44*>(&mat);
}

inline JPH::EMotionType toJPH(engine::physics::MotionType motionType) {
	switch (motionType) {
		case engine::physics::MotionType::Static:
			return JPH::EMotionType::Static;
		case engine::physics::MotionType::Kinematic:
			return JPH::EMotionType::Kinematic;
		case engine::physics::MotionType::Dynamic:
			return JPH::EMotionType::Dynamic;
		default:
			engine::log::Fatal("Additional physics MotionType that has not been accounted for");
			return JPH::EMotionType::Static;
	}
}

inline JPH::EMotionQuality toJPH(engine::physics::MotionQuality quality) {
	return (quality == engine::physics::MotionQuality::Discrete) ? JPH::EMotionQuality::Discrete : JPH::EMotionQuality::LinearCast;
}

inline JPH::Character::EGroundState toJPH(engine::physics::GroundState groundState) {
	switch (groundState) {
		case engine::physics::GroundState::Normal:
			return JPH::Character::EGroundState::OnGround;
		case engine::physics::GroundState::Steep:
			return JPH::Character::EGroundState::OnSteepGround;
		case engine::physics::GroundState::Unstable:
			return JPH::Character::EGroundState::NotSupported;
		case engine::physics::GroundState::Airborne:
			return JPH::Character::EGroundState::InAir;
		default:
			engine::log::Fatal("Additional GroundState that has not been accounted for");
			return JPH::Character::EGroundState::OnGround;
	}
}

#endif //ENGINE_PHYSICS_MANAGER_HPP
