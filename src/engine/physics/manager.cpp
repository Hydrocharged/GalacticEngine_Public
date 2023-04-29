// Copyright © 2022-2023 Daylon Wilkins & James Cor
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <engine/physics/manager.hpp>
#include <engine/log/log.hpp>
#include <cstdarg>
#include <thread>

#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>

// Disables common warnings triggered by Jolt
JPH_SUPPRESS_WARNINGS

namespace engine::physics {
	Manager* GlobalManager = nullptr;
}

// Size of the temp allocator used for temporary allocations during the physics update.
const int allocatorSize = 10 * 1024 * 1024;
// This is the max amount of rigid bodies that can be added to the physics system. Adding more will produce an error.
const unsigned int maxPhysicsBodies = 65536;
// The number of mutexes to allocate to protect rigid bodies from concurrent access. 0 represents the default settings.
const unsigned int numberOfBodyMutexes = 0;
// The max amount of body pairs that can be queued at any time for the broad phase.
const unsigned int maxBodyPairs = 65536;
// The maximum size of the contact constraint buffer. If more contacts are detected than this, then the additional ones
// will be ignored (causing clipping, falling through the world, etc.)
const unsigned int maxContactConstraints = 10240;
// Maximum amount of physics jobs to allow.
const int maxPhysicsJobs = JPH::cMaxPhysicsJobs;
// Maximum amount of physics barriers to allow.
const int maxPhysicsBarriers = JPH::cMaxPhysicsBarriers;

static void DebugTraceCallback(const char* inFMT, ...) {
	// Format the message
	va_list list;
		va_start(list, inFMT);
	engine::log::Debug(inFMT, list);
		va_end(list);
}

#ifdef JPH_ENABLE_ASSERTS
static bool AssertionFailed(const char* expr, const char* message, const char* file, JPH::uint line) {
	engine::log::Debug("%s:%u: (%s) %s", file, line, expr, message != nullptr ? message : "");
	return true;
};
#endif // JPH_ENABLE_ASSERTS

// Determines if two object layers can collide.
bool engine::physics::ObjectLayerPairFilterImpl::ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const {
	switch (inObject1) {
		case engine::physics::Layers::NON_MOVING:
			return inObject2 == engine::physics::Layers::MOVING; // Non-moving only collides with moving
		case engine::physics::Layers::MOVING:
			return true; // Moving collides with everything
		default:
			assert(false);
			return false;
	}
};

engine::physics::BroadPhaseLayerImpl::BroadPhaseLayerImpl() = default;

JPH::uint engine::physics::BroadPhaseLayerImpl::GetNumBroadPhaseLayers() const {
	return BroadPhaseLayers::NUM_LAYERS;
}

JPH::BroadPhaseLayer engine::physics::BroadPhaseLayerImpl::GetBroadPhaseLayer(JPH::ObjectLayer layer) const {
	assert(layer < Layers::NUM_LAYERS);
	return objToBP[layer];
}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
const char* engine::physics::BroadPhaseLayerImpl::GetBroadPhaseLayerName(JPH::BroadPhaseLayer layer) const {
	switch ((JPH::BroadPhaseLayer::Type)layer) {
		case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::NONMOVING:
			return "NONMOVING";
		case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:
			return "MOVING";
		default:
			assert(false);
			return "INVALID";
	}
}
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

// Determines if two broadphase layers can collide.
bool engine::physics::ObjectVsBroadPhaseLayerFilterImpl::ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const {
	switch (inLayer1) {
		case engine::physics::Layers::NON_MOVING:
			return inLayer2 == engine::physics::BroadPhaseLayers::MOVING;
		case engine::physics::Layers::MOVING:
			return true;
		default:
			assert(false);
			return false;
	}
}

JPH::ValidateResult engine::physics::InternalContactListener::OnContactValidate(const JPH::Body& inBody1, const JPH::Body& inBody2, JPH::RVec3Arg inBaseOffset, const JPH::CollideShapeResult& inCollisionResult) {
	return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
}

void engine::physics::InternalContactListener::OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) {
}

void engine::physics::InternalContactListener::OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) {
}

void engine::physics::InternalContactListener::OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair) {
}

// Modeled from Jolt/Physics/Collision/CollisionCollectorImpl.h::ClosestHitCollisionCollector
template<class CollectorType>
class FurthestHitCollisionCollector : public CollectorType {
public:
	void Reset() override {
		CollectorType::Reset();
		mHadHit = false;
	}
	void AddHit(const typename CollectorType::ResultType& inResult) override {
		float early_out = inResult.GetEarlyOutFraction();
		if (!mHadHit || early_out > mHit.GetEarlyOutFraction()) {
			mHit = inResult;
			mHadHit = true;
		}
	}
	inline bool HadHit() const {
		return mHadHit;
	}
	typename CollectorType::ResultType mHit;

private:
	bool mHadHit = false;
};

engine::physics::Manager::Manager(engine::Application* application) : application(application) {
	JPH::RegisterDefaultAllocator();
	JPH::Trace = DebugTraceCallback;
	JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = AssertionFailed);
	JPH::Factory::sInstance = new JPH::Factory();
	JPH::RegisterTypes();

	tempAllocator = std::make_unique<JPH::TempAllocatorImpl>(allocatorSize);
	int numOfThreads = (int)std::thread::hardware_concurrency() - 1;
	if (numOfThreads < 0) {
		numOfThreads = 0;
	}
	jobSystem = std::make_unique<JPH::JobSystemThreadPool>(maxPhysicsJobs, maxPhysicsBarriers, numOfThreads);
	broadPhaseLayerImpl = std::make_unique<BroadPhaseLayerImpl>();
	objectVsBroadPhaseLayerFilterImpl = std::make_unique<ObjectVsBroadPhaseLayerFilterImpl>();
	objectLayerPairFilterImpl = std::make_unique<ObjectLayerPairFilterImpl>();
	physicsSystem = std::make_unique<JPH::PhysicsSystem>();
	physicsSystem->Init(maxPhysicsBodies,
						numberOfBodyMutexes,
						maxBodyPairs,
						maxContactConstraints,
						*broadPhaseLayerImpl,
						*objectVsBroadPhaseLayerFilterImpl,
						*objectLayerPairFilterImpl);
	JPH::BodyInterface& bodyInterface = physicsSystem->GetBodyInterface();
	physicsSystem->OptimizeBroadPhase();
}

void engine::physics::Manager::Update(double deltaTime) {
	if (!enabled) {
		return;
	}
	double simulatedDeltaTime = 0.0;
	for (; (simulatedDeltaTime + maxDeltaTimeStep) < deltaTime; simulatedDeltaTime += maxDeltaTimeStep) {
		application->FixedUpdate(maxDeltaTimeStep);
		physicsSystem->Update(maxDeltaTimeStepf, 1, 1, tempAllocator.get(), jobSystem.get());
	}
	double remainingTime = deltaTime - simulatedDeltaTime;
	application->FixedUpdate(remainingTime);
	physicsSystem->Update(float(remainingTime), 1, 1, tempAllocator.get(), jobSystem.get());
}

void engine::physics::Manager::SetUpdateRate(double rate) {
	maxDeltaTimeStep = 1.0 / rate;
	maxDeltaTimeStepf = (float)maxDeltaTimeStep;
}

void engine::physics::Manager::OptimizeBroadPhase() {
	physicsSystem->OptimizeBroadPhase();
}

glm::vec3 engine::physics::Manager::GetGravity() {
	return toGLM(physicsSystem->GetGravity());
}

void engine::physics::Manager::SetGravity(glm::vec3 gravity) {
	physicsSystem->SetGravity(toJPH(gravity));
}

std::vector<engine::physics::RayResult> engine::physics::Manager::CastRay(glm::vec3 origin, glm::vec3 directionWithMagnitude, RayFilter filter) {
	JPH::RayCast ray{toJPH(origin), toJPH(directionWithMagnitude)};
	std::vector<RayResult> hitBodies;
	switch (filter) {
		case RayFilter::AllHit: {
			JPH::AllHitCollisionCollector<JPH::RayCastBodyCollector> collector;
			physicsSystem->GetBroadPhaseQuery().CastRay(ray, collector);
			collector.Sort();
			auto numOfHits = size_t(collector.mHits.size());
			hitBodies.reserve(numOfHits);
			JPH::BroadPhaseCastResult* results = collector.mHits.data();
			for (size_t i = 0; i < numOfHits; i++) {
				auto& result = results[i];
				hitBodies.emplace_back(RayResult{
					.Body = Body(result.mBodyID.GetIndexAndSequenceNumber(), false),
					.ContactPoint = origin + (result.mFraction * (directionWithMagnitude)),
				});
			}
			break;
		}
		case RayFilter::AnyHit: {
			JPH::AnyHitCollisionCollector<JPH::RayCastBodyCollector> collector;
			physicsSystem->GetBroadPhaseQuery().CastRay(ray, collector);
			if (collector.HadHit()) {
				hitBodies.reserve(1);
				hitBodies.push_back(RayResult{
					.Body = Body(collector.mHit.mBodyID.GetIndexAndSequenceNumber(), false),
					.ContactPoint = origin + (collector.mHit.mFraction * (directionWithMagnitude)),
				});
			}
			break;
		}
		case RayFilter::ClosestHit: {
			JPH::ClosestHitCollisionCollector<JPH::RayCastBodyCollector> collector;
			physicsSystem->GetBroadPhaseQuery().CastRay(ray, collector);
			if (collector.HadHit()) {
				hitBodies.reserve(1);
				hitBodies.push_back(RayResult{
					.Body = Body(collector.mHit.mBodyID.GetIndexAndSequenceNumber(), false),
					.ContactPoint = origin + (collector.mHit.mFraction * (directionWithMagnitude)),
				});
			}
			break;
		}
		case RayFilter::FurthestHit: {
			FurthestHitCollisionCollector<JPH::RayCastBodyCollector> collector;
			physicsSystem->GetBroadPhaseQuery().CastRay(ray, collector);
			if (collector.HadHit()) {
				hitBodies.reserve(1);
				hitBodies.push_back(RayResult{
					.Body = Body(collector.mHit.mBodyID.GetIndexAndSequenceNumber(), false),
					.ContactPoint = origin + (collector.mHit.mFraction * (directionWithMagnitude)),
				});
			}
			break;
		}
	}
	return hitBodies;
}

std::unique_ptr<engine::physics::Body> engine::physics::Manager::CreateBody(JPH::Shape* shape, const BodyCreationProperties properties) {
	JPH::BodyInterface& bodyInterface = physicsSystem->GetBodyInterface();
	auto layer = properties.MotionType == MotionType::Static ? Layers::NON_MOVING : Layers::MOVING;
	auto activate = properties.MotionType == MotionType::Static ? JPH::EActivation::DontActivate : JPH::EActivation::Activate;
	JPH::BodyCreationSettings settings(shape, toJPH(properties.Position), toJPH(properties.Rotation), toJPH(properties.MotionType), layer);
	settings.mMotionQuality = toJPH(properties.MotionQuality);
	JPH::BodyID bodyID = bodyInterface.CreateAndAddBody(settings, activate);
	if (bodyID.IsInvalid()) {
		delete (shape);
		engine::log::Debug("Physics bodies limit has been hit, cannot create more bodies");
		return nullptr;
	}
	return std::unique_ptr<Body>(new Body(bodyID.GetIndexAndSequenceNumber()));
}

std::unique_ptr<engine::physics::Character> engine::physics::Manager::CreateCharacter(const CharacterCreationProperties properties) {
	float halfHeight = 0.5f * properties.Height;
	float radius = 0.5f * properties.Width;

	JPH::CharacterSettings characterSettings;
	characterSettings.mLayer = Layers::MOVING;
	characterSettings.mMass = properties.Weight;
	characterSettings.mMaxSlopeAngle = properties.MaxSlopeAngle;
	characterSettings.mGravityFactor = properties.GravityFactor;
	// The supporting volume should be shifted, so that the bottom of the capsule is at 0.0
	characterSettings.mSupportingVolume = JPH::Plane(JPH::Vec3::sAxisY(), -radius);
	characterSettings.mFriction = 0.5f;

	auto shape = JPH::RotatedTranslatedShapeSettings(
		JPH::Vec3(0, halfHeight + radius, 0),
		JPH::Quat::sIdentity(),
		new JPH::CapsuleShape(halfHeight, radius)).Create().Get();
	characterSettings.mShape = shape;

	JPH::uint64 inUserData = 0;
	auto character = new JPH::Character(&characterSettings, toJPH(properties.Position), toJPH(properties.Rotation), inUserData, physicsSystem.get());
	return std::unique_ptr<Character>(new Character(character));
}

engine::physics::Manager::~Manager() {
	delete JPH::Factory::sInstance;
	JPH::Factory::sInstance = nullptr;
}

void engine::physics::Manager::Enable() {
	enabled = true;
}

void engine::physics::Manager::Disable() {
	enabled = false;
}

void engine::physics::Enable() {
	GlobalManager->Enable();
}

void engine::physics::Disable() {
	GlobalManager->Disable();
}

void engine::physics::Initialize(engine::Application* application) {
	GlobalManager = new Manager(application);
}

void engine::physics::Update(double deltaTime) {
	GlobalManager->Update(deltaTime);
}

void engine::physics::Terminate() {
	delete (GlobalManager);
}

void engine::physics::SetUpdateRate(double rate) {
	GlobalManager->SetUpdateRate(rate);
}

glm::vec3 engine::physics::GetGravity() {
	return GlobalManager->GetGravity();
}

void engine::physics::SetGravity(glm::vec3 gravity) {
	GlobalManager->SetGravity(gravity);
}

std::uint32_t engine::physics::GetMaxNumberOfBodies() {
	return maxPhysicsBodies;
}

std::vector<engine::physics::RayResult> engine::physics::CastRay(glm::vec3 origin, glm::vec3 direction, float magnitude, RayFilter filter) {
	return GlobalManager->CastRay(origin, glm::normalize(direction) * magnitude, filter);
}

std::vector<engine::physics::RayResult> engine::physics::CastRay(glm::vec3 origin, glm::vec3 directionWithMagnitude, RayFilter filter) {
	return GlobalManager->CastRay(origin, directionWithMagnitude, filter);
}
