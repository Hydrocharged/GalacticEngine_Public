// Copyright © 2022-2023 Daylon Wilkins & James Cor
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <engine/physics/manager.hpp>

static_assert(sizeof(JPH::BodyID) == sizeof(std::uint32_t), "Expected BodyID to be the same size as an uint32");

engine::physics::Body::Body(std::uint32_t id, bool destructible) : id(id), destructible(destructible) {}

engine::physics::Body::~Body() {
	if (destructible) {
		auto bodyID = static_cast<JPH::BodyID>(id);
		auto& bodyInterface = engine::physics::GlobalManager->physicsSystem->GetBodyInterface();
		bodyInterface.RemoveBody(bodyID);
		bodyInterface.DestroyBody(bodyID);
	}
}

void engine::physics::Body::Activate() const {
	auto bodyID = static_cast<JPH::BodyID>(id);
	engine::physics::GlobalManager->physicsSystem->GetBodyInterface().ActivateBody(bodyID);
}

void engine::physics::Body::Deactivate() const {
	auto bodyID = static_cast<JPH::BodyID>(id);
	engine::physics::GlobalManager->physicsSystem->GetBodyInterface().DeactivateBody(bodyID);
}

std::uint32_t engine::physics::Body::GetID() const {
	return id;
}

glm::vec3 engine::physics::Body::GetPosition() const {
	auto bodyID = static_cast<JPH::BodyID>(id);
	return toGLM(engine::physics::GlobalManager->physicsSystem->GetBodyInterface().GetPosition(bodyID));
}

glm::vec3 engine::physics::Body::GetCenterOfMassPosition() const {
	auto bodyID = static_cast<JPH::BodyID>(id);
	return toGLM(engine::physics::GlobalManager->physicsSystem->GetBodyInterface().GetCenterOfMassPosition(bodyID));
}

glm::vec3 engine::physics::Body::GetLinearVelocity() const {
	auto bodyID = static_cast<JPH::BodyID>(id);
	return toGLM(engine::physics::GlobalManager->physicsSystem->GetBodyInterface().GetLinearVelocity(bodyID));
}

float engine::physics::Body::GetMaxLinearVelocity() const {
	auto bodyID = static_cast<JPH::BodyID>(id);
	auto body = engine::physics::GlobalManager->physicsSystem->GetBodyLockInterface().TryGetBody(bodyID);
#if !defined(NDEBUG) || defined(_DEBUG)
	if (!body) {
		engine::log::Debug("Could not get the physics body when querying GetMaxLinearVelocity");
		return 500.0f; // Default taken from BodyCreationSettings.h
	}
#endif //!defined(NDEBUG) || defined(_DEBUG)
	return body->GetMotionProperties()->GetMaxLinearVelocity();
}

glm::vec3 engine::physics::Body::GetAngularVelocity() const {
	auto bodyID = static_cast<JPH::BodyID>(id);
	return toGLM(engine::physics::GlobalManager->physicsSystem->GetBodyInterface().GetAngularVelocity(bodyID));
}

float engine::physics::Body::GetMaxAngularVelocity() const {
	auto bodyID = static_cast<JPH::BodyID>(id);
	auto body = engine::physics::GlobalManager->physicsSystem->GetBodyLockInterface().TryGetBody(bodyID);
#if !defined(NDEBUG) || defined(_DEBUG)
	if (!body) {
		engine::log::Debug("Could not get the physics body when querying GetMaxAngularVelocity");
		return 0.25f * JPH::JPH_PI * 60.0f; // Default taken from BodyCreationSettings.h
	}
#endif //!defined(NDEBUG) || defined(_DEBUG)
	return body->GetMotionProperties()->GetMaxAngularVelocity();
}

glm::quat engine::physics::Body::GetRotation() const {
	auto bodyID = static_cast<JPH::BodyID>(id);
	return toGLM(engine::physics::GlobalManager->physicsSystem->GetBodyInterface().GetRotation(bodyID));
}

glm::mat4 engine::physics::Body::GetRotationMatrix() const {
	return glm::toMat4(GetRotation());
}

glm::vec3 engine::physics::Body::GetScale() const {
	auto bodyID = static_cast<JPH::BodyID>(id);
	return toGLM(engine::physics::GlobalManager->physicsSystem->GetBodyInterface().GetShape(bodyID)->GetLocalBounds().GetSize());
}

glm::mat4 engine::physics::Body::GetTransform() const {
	auto bodyID = static_cast<JPH::BodyID>(id);
	return toGLM(engine::physics::GlobalManager->physicsSystem->GetBodyInterface().GetWorldTransform(bodyID));
}

glm::mat4 engine::physics::Body::GetScaledTransform() const {
	auto bodyID = static_cast<JPH::BodyID>(id);
	auto shape = engine::physics::GlobalManager->physicsSystem->GetBodyInterface().GetTransformedShape(bodyID);
	auto scale = shape.mShape->GetLocalBounds().GetSize();
	JPH::RMat44 transform = JPH::RMat44::sRotation(shape.mShapeRotation).PreScaled(scale);
	transform.SetTranslation(shape.mShapePositionCOM - transform.Multiply3x3(shape.mShape->GetCenterOfMass()));
	return toGLM(transform);
}

glm::vec3 engine::physics::Body::GetBoundingBox(glm::vec3& min, glm::vec3& max) const {
	auto bodyID = static_cast<JPH::BodyID>(id);
	auto boundingBox = engine::physics::GlobalManager->physicsSystem->GetBodyInterface().GetTransformedShape(bodyID).mShape->GetLocalBounds();
	min = toGLM(boundingBox.mMin);
	max = toGLM(boundingBox.mMax);
	return toGLM(boundingBox.GetSize());
}

float engine::physics::Body::GetFriction() const {
	auto bodyID = static_cast<JPH::BodyID>(id);
	return engine::physics::GlobalManager->physicsSystem->GetBodyInterface().GetFriction(bodyID);
}

float engine::physics::Body::GetGravityFactor() const {
	auto bodyID = static_cast<JPH::BodyID>(id);
	return engine::physics::GlobalManager->physicsSystem->GetBodyInterface().GetGravityFactor(bodyID);
}

float engine::physics::Body::GetRestitution() const {
	auto bodyID = static_cast<JPH::BodyID>(id);
	return engine::physics::GlobalManager->physicsSystem->GetBodyInterface().GetRestitution(bodyID);
}

engine::physics::MotionType engine::physics::Body::GetMotionType() const {
	auto bodyID = static_cast<JPH::BodyID>(id);
	return toGLM(engine::physics::GlobalManager->physicsSystem->GetBodyInterface().GetMotionType(bodyID));
}

engine::physics::MotionQuality engine::physics::Body::GetMotionQuality() const {
	auto bodyID = static_cast<JPH::BodyID>(id);
	return toGLM(engine::physics::GlobalManager->physicsSystem->GetBodyInterface().GetMotionQuality(bodyID));
}

engine::physics::Shape engine::physics::Body::GetShape() const {
	auto bodyID = static_cast<JPH::BodyID>(id);
	auto body = engine::physics::GlobalManager->physicsSystem->GetBodyLockInterface().TryGetBody(bodyID);
#if !defined(NDEBUG) || defined(_DEBUG)
	if (!body) {
		engine::log::Debug("Could not get the physics body when querying GetShape");
		return Shape::Box;
	}
#endif //!defined(NDEBUG) || defined(_DEBUG)
	return toGLM(body->GetShape()->GetSubType());
}

engine::physics::Body* engine::physics::Body::GetCopy() const {
	return new Body(id, false);
}

bool engine::physics::Body::IsActive() const {
	auto bodyID = static_cast<JPH::BodyID>(id);
	return engine::physics::GlobalManager->physicsSystem->GetBodyInterface().IsActive(bodyID);
}

bool engine::physics::Body::IsDynamic() const {
	auto bodyID = static_cast<JPH::BodyID>(id);
	return engine::physics::GlobalManager->physicsSystem->GetBodyInterface().GetMotionType(bodyID) == JPH::EMotionType::Dynamic;
}

bool engine::physics::Body::IsKinematic() const {
	auto bodyID = static_cast<JPH::BodyID>(id);
	return engine::physics::GlobalManager->physicsSystem->GetBodyInterface().GetMotionType(bodyID) == JPH::EMotionType::Kinematic;
}

bool engine::physics::Body::IsStatic() const {
	auto bodyID = static_cast<JPH::BodyID>(id);
	return engine::physics::GlobalManager->physicsSystem->GetBodyInterface().GetMotionType(bodyID) == JPH::EMotionType::Static;
}

bool engine::physics::Body::IsSensor() const {
	auto bodyID = static_cast<JPH::BodyID>(id);
	auto body = engine::physics::GlobalManager->physicsSystem->GetBodyLockInterface().TryGetBody(bodyID);
#if !defined(NDEBUG) || defined(_DEBUG)
	if (!body) {
		engine::log::Debug("Could not get the physics body when querying IsSensor");
		return false;
	}
#endif //!defined(NDEBUG) || defined(_DEBUG)
	return body->IsSensor();
}

bool engine::physics::Body::TestRay(glm::vec3 origin, glm::vec3 direction, float magnitude) const {
	glm::vec3 contactPoint;
	return TestRay(origin, direction, magnitude, contactPoint);
}

bool engine::physics::Body::TestRay(glm::vec3 origin, glm::vec3 direction, float magnitude, glm::vec3& contactPoint) const {
	return TestRay(origin, glm::normalize(direction) * magnitude, contactPoint);
}

bool engine::physics::Body::TestRay(glm::vec3 origin, glm::vec3 directionWithMagnitude) const {
	glm::vec3 contactPoint;
	return TestRay(origin, directionWithMagnitude, contactPoint);
}

bool engine::physics::Body::TestRay(glm::vec3 origin, glm::vec3 directionWithMagnitude, glm::vec3& contactPoint) const {
	JPH::RayCast ray{toJPH(origin), toJPH(directionWithMagnitude)};
	JPH::ClosestHitCollisionCollector<JPH::RayCastBodyCollector> collector;
	engine::physics::GlobalManager->physicsSystem->GetBroadPhaseQuery().CastRay(ray, collector);
	if (collector.HadHit() && collector.mHit.mBodyID.GetIndexAndSequenceNumber() == id) {
		contactPoint = origin + (collector.mHit.mFraction * (directionWithMagnitude));
		return true;
	}
	return false;
}

void engine::physics::Body::SetPosition(glm::vec3 position, bool forceActivate) const {
	auto bodyID = static_cast<JPH::BodyID>(id);
	auto activation = forceActivate ? JPH::EActivation::Activate : JPH::EActivation::DontActivate;
	engine::physics::GlobalManager->physicsSystem->GetBodyInterface().SetPosition(bodyID, toJPH(position), activation);
}

void engine::physics::Body::SetLinearVelocity(glm::vec3 velocity) const {
	auto bodyID = static_cast<JPH::BodyID>(id);
	engine::physics::GlobalManager->physicsSystem->GetBodyInterface().SetLinearVelocity(bodyID, toJPH(velocity));
}

void engine::physics::Body::SetLinearVelocityClamped(glm::vec3 velocity) const {
	auto bodyID = static_cast<JPH::BodyID>(id);
	auto body = engine::physics::GlobalManager->physicsSystem->GetBodyLockInterface().TryGetBody(bodyID);
#if !defined(NDEBUG) || defined(_DEBUG)
	if (!body) {
		engine::log::Debug("Could not get the physics body when setting SetLinearVelocityClamped");
		return;
	}
#endif //!defined(NDEBUG) || defined(_DEBUG)
	body->SetLinearVelocityClamped(toJPH(velocity));
}

void engine::physics::Body::SetMaxLinearVelocity(float velocity) const {
	auto bodyID = static_cast<JPH::BodyID>(id);
	auto body = engine::physics::GlobalManager->physicsSystem->GetBodyLockInterface().TryGetBody(bodyID);
#if !defined(NDEBUG) || defined(_DEBUG)
	if (!body) {
		engine::log::Debug("Could not get the physics body when setting SetMaxLinearVelocity");
		return;
	}
#endif //!defined(NDEBUG) || defined(_DEBUG)
	body->GetMotionProperties()->SetMaxLinearVelocity(velocity);
}

void engine::physics::Body::SetAngularVelocity(glm::vec3 velocity) const {
	auto bodyID = static_cast<JPH::BodyID>(id);
	engine::physics::GlobalManager->physicsSystem->GetBodyInterface().SetAngularVelocity(bodyID, toJPH(velocity));
}

void engine::physics::Body::SetAngularVelocityClamped(glm::vec3 velocity) const {
	auto bodyID = static_cast<JPH::BodyID>(id);
	auto body = engine::physics::GlobalManager->physicsSystem->GetBodyLockInterface().TryGetBody(bodyID);
#if !defined(NDEBUG) || defined(_DEBUG)
	if (!body) {
		engine::log::Debug("Could not get the physics body when setting SetAngularVelocityClamped");
		return;
	}
#endif //!defined(NDEBUG) || defined(_DEBUG)
	body->SetAngularVelocityClamped(toJPH(velocity));
}

void engine::physics::Body::SetMaxAngularVelocity(float velocity) const {
	auto bodyID = static_cast<JPH::BodyID>(id);
	auto body = engine::physics::GlobalManager->physicsSystem->GetBodyLockInterface().TryGetBody(bodyID);
#if !defined(NDEBUG) || defined(_DEBUG)
	if (!body) {
		engine::log::Debug("Could not get the physics body when setting SetMaxAngularVelocity");
		return;
	}
#endif //!defined(NDEBUG) || defined(_DEBUG)
	body->GetMotionProperties()->SetMaxAngularVelocity(velocity);
}

void engine::physics::Body::SetRotation(glm::quat rotation, bool forceActivate) const {
	auto bodyID = static_cast<JPH::BodyID>(id);
	auto activation = forceActivate ? JPH::EActivation::Activate : JPH::EActivation::DontActivate;
	engine::physics::GlobalManager->physicsSystem->GetBodyInterface().SetRotation(bodyID, toJPH(rotation), activation);
}

void engine::physics::Body::SetFriction(float friction) const {
	auto bodyID = static_cast<JPH::BodyID>(id);
	engine::physics::GlobalManager->physicsSystem->GetBodyInterface().SetFriction(bodyID, friction);
}

void engine::physics::Body::SetGravityFactor(float gravityFactor) const {
	auto bodyID = static_cast<JPH::BodyID>(id);
	engine::physics::GlobalManager->physicsSystem->GetBodyInterface().SetGravityFactor(bodyID, gravityFactor);
}

void engine::physics::Body::SetRestitution(float restitution) const {
	auto bodyID = static_cast<JPH::BodyID>(id);
	engine::physics::GlobalManager->physicsSystem->GetBodyInterface().SetRestitution(bodyID, restitution);
}

void engine::physics::Body::SetMotionQuality(MotionQuality quality) const {
	auto bodyID = static_cast<JPH::BodyID>(id);
	engine::physics::GlobalManager->physicsSystem->GetBodyInterface().SetMotionQuality(bodyID, toJPH(quality));
}

void engine::physics::Body::SetIsSensor(bool isSensor) const {
	auto bodyID = static_cast<JPH::BodyID>(id);
	auto body = engine::physics::GlobalManager->physicsSystem->GetBodyLockInterface().TryGetBody(bodyID);
#if !defined(NDEBUG) || defined(_DEBUG)
	if (!body) {
		engine::log::Debug("Could not get the physics body when setting SetIsSensor");
		return;
	}
#endif //!defined(NDEBUG) || defined(_DEBUG)
	body->SetIsSensor(isSensor);
}

void engine::physics::Body::AddForce(glm::vec3 force) const {
	auto bodyID = static_cast<JPH::BodyID>(id);
	engine::physics::GlobalManager->physicsSystem->GetBodyInterface().AddForce(bodyID, toJPH(force));
}

void engine::physics::Body::AddForce(glm::vec3 force, glm::vec3 point) const {
	auto bodyID = static_cast<JPH::BodyID>(id);
	engine::physics::GlobalManager->physicsSystem->GetBodyInterface().AddForce(bodyID, toJPH(force), toJPH(point));
}

void engine::physics::Body::AddImpulse(glm::vec3 impulse) const {
	auto bodyID = static_cast<JPH::BodyID>(id);
	engine::physics::GlobalManager->physicsSystem->GetBodyInterface().AddImpulse(bodyID, toJPH(impulse));
}

void engine::physics::Body::AddImpulse(glm::vec3 impulse, glm::vec3 point) const {
	auto bodyID = static_cast<JPH::BodyID>(id);
	engine::physics::GlobalManager->physicsSystem->GetBodyInterface().AddImpulse(bodyID, toJPH(impulse), toJPH(point));
}

void engine::physics::Body::AddAngularImpulse(glm::vec3 impulse) const {
	auto bodyID = static_cast<JPH::BodyID>(id);
	engine::physics::GlobalManager->physicsSystem->GetBodyInterface().AddAngularImpulse(bodyID, toJPH(impulse));
}

void engine::physics::Body::AddTorque(glm::vec3 torque) const {
	auto bodyID = static_cast<JPH::BodyID>(id);
	engine::physics::GlobalManager->physicsSystem->GetBodyInterface().AddTorque(bodyID, toJPH(torque));
}

void engine::physics::Body::MoveKinematic(glm::vec3 position, glm::quat rotation, float seconds) const {
	auto bodyID = static_cast<JPH::BodyID>(id);
	engine::physics::GlobalManager->physicsSystem->GetBodyInterface().MoveKinematic(bodyID, toJPH(position), toJPH(rotation), seconds);
}
