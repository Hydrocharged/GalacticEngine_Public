// Copyright © 2022-2023 Daylon Wilkins & James Cor
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef ENGINE_PHYSICS_PHYSICS_HPP
#define ENGINE_PHYSICS_PHYSICS_HPP

#include <memory>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>

namespace engine {
	class Application;
}

namespace engine::physics {
	class Manager;

	// Mass determines a body's mass when it is created. If both Density and Mass are zero, then a default density of
	// 1000 kilograms / meter^3 is used. If either Density or Mass is non-zero, then it is used. If both Density and
	// Mass are non-zero, then only Density is used.
	struct Mass {
		float Density = 0.0f; // kg / m^3
		float Weight = 0.0f; // kg
	};

	// Determines how physics will be simulated on a body.
	enum class MotionType : std::uint8_t {
		Static, // Non-movable
		Kinematic, // Movable using velocities only, does not respond to forces
		Dynamic, // Responds to forces as a normal physics object
	};

	// Determines how well a body detects collisions when it has a high velocity.
	enum class MotionQuality : std::uint8_t {
		// Update the body in discrete steps. A body will tunnel through thin objects if its velocity is high enough.
		// This is the cheapest way of simulating a body.
		Discrete,

		// Update the body using linear casting. When stepping the body, its collision shape is cast from
		// start to destination using the starting rotation. The body will not be able to tunnel through thin
		// objects at high velocity, but tunneling is still possible if the body is long and thin and has a high
		// angular velocity. Time is stolen from the object (which means it will move up to the first collision
		// and will not bounce off the surface until the next integration step). This will make the body appear
		// to go slower when it collides with high velocity. In order to not get stuck, the body is always
		// allowed to move by a fraction of its inner radius, which may eventually lead it to pass through geometry.
		// This is a far more expensive way of simulating a body.
		LinearCast,
	};

	// Determines which bodies should be returned when casting a ray.
	enum class RayFilter : std::uint8_t {
		// Returns all bodies hit by the ray. Sorted from the body closest to the origin ascending.
		AllHit,
		// Returns a body hit by the ray. This may vary between calls.
		AnyHit,
		// Returns only the body hit by the ray that is closest to the origin
		ClosestHit,
		// Returns only the body hit by the ray that is furthest from the origin. This performs a bit slower than
		// AnyHit and ClosestHit.
		FurthestHit
	};

	// The character's contact state with the ground.
	enum class GroundState : std::uint8_t {
		// Can freely move on the ground.
		Normal,
		// The ground's slope is too steep.
		Steep,
		// The ground is unstable, and the character may need a downward force applied to it.
		Unstable,
		// Not on the ground.
		Airborne,
	};

	// The shape of the body's collider.
	enum class Shape : std::uint8_t {
		Sphere,
		Box,
		Capsule,
		TaperedCapsule,
		Cylinder,
	};

	// The entity upon which all physics calculations are performed.
	class Body {
	public:
		~Body();

		// Put this body into the awake state
		void Activate() const;
		// Put this body into the sleep state
		void Deactivate() const;

		// Get an ID that uniquely identifies this body. This ID is not guaranteed to be unique across the application's
		// lifetime as IDs are recycled, however it is guaranteed to be unique for the body's lifetime.
		[[nodiscard]] std::uint32_t GetID() const;
		// Get the world space position of the body
		[[nodiscard]] glm::vec3 GetPosition() const;
		// Get the world space position of this body's center of mass
		[[nodiscard]] glm::vec3 GetCenterOfMassPosition() const;
		// Get the world space linear velocity of the center of mass (m/s)
		[[nodiscard]] glm::vec3 GetLinearVelocity() const;
		// Get the maximum linear velocity that the body can achieve (m/s). Used to prevent the system from exploding.
		[[nodiscard]] float GetMaxLinearVelocity() const;
		// Get the world space angular velocity of the center of mass (rad/s)
		[[nodiscard]] glm::vec3 GetAngularVelocity() const;
		// Get the maximum angular velocity that the body can achieve (rad/s). Used to prevent the system from exploding.
		[[nodiscard]] float GetMaxAngularVelocity() const;
		// Get the world space rotation of the body
		[[nodiscard]] glm::quat GetRotation() const;
		// Get the world space rotation of the body
		[[nodiscard]] glm::mat4 GetRotationMatrix() const;
		// Get the world space scale of the body's shape
		[[nodiscard]] glm::vec3 GetScale() const;
		// Calculates this body's transform in world space
		[[nodiscard]] glm::mat4 GetTransform() const;
		// Calculates this body's transform in world space, includes the body shape's scale
		[[nodiscard]] glm::mat4 GetScaledTransform() const;
		// Get the size of the bounding box, while assigning the minimum and maximum to the referenced parameters
		glm::vec3 GetBoundingBox(glm::vec3& min, glm::vec3& max) const;
		// Set the friction (strictly between 0 and 1, 0 = no friction, 1 = friction force equals force that presses the two bodies together)
		[[nodiscard]] float GetFriction() const;
		// Get the gravity factor, which is a multiplier on gravity's influence (0 = no gravity, 1 = normal gravity, >1 = more gravity)
		[[nodiscard]] float GetGravityFactor() const;
		// Get the restitution (strictly between 0 and 1, 0 = completely inelastic collision response, 1 = completely elastic collision response)
		[[nodiscard]] float GetRestitution() const;
		// Get the motion type of the body
		[[nodiscard]] MotionType GetMotionType() const;
		// Get the motion quality of the body
		[[nodiscard]] MotionQuality GetMotionQuality() const;
		// Get the body's shape
		[[nodiscard]] Shape GetShape() const;
		// Get a copy of this body. May safely be deleted without affecting the original body.
		[[nodiscard]] Body* GetCopy() const;
		// Get whether this body is actively simulating (true) or sleeping (false)
		[[nodiscard]] bool IsActive() const;
		// Get whether this body is dynamic, which means that it moves and forces can act on it
		[[nodiscard]] bool IsDynamic() const;
		// Get whether this body is kinematic, which means that it will move according to its current velocity, but forces do not affect it
		[[nodiscard]] bool IsKinematic() const;
		// Get whether this body is static (non-movable)
		[[nodiscard]] bool IsStatic() const;
		// Get whether this body is a sensor
		[[nodiscard]] bool IsSensor() const;
		// Casts a ray in the given direction and returns true if the ray did not encounter other bodies before
		// colliding with this body.
		[[nodiscard]] bool TestRay(glm::vec3 origin, glm::vec3 direction, float magnitude) const;
		// Casts a ray in the given direction and returns true if the ray did not encounter other bodies before
		// colliding with this body. If a collision occurred, then this also sets the contact point in world space.
		[[nodiscard]] bool TestRay(glm::vec3 origin, glm::vec3 direction, float magnitude, glm::vec3& contactPoint) const;
		// Casts a ray in the given direction and returns true if the ray did not encounter other bodies before
		// colliding with this body. The direction should not be normalized, as the direction's magnitude determines the
		// length of the ray.
		[[nodiscard]] bool TestRay(glm::vec3 origin, glm::vec3 directionWithMagnitude) const;
		// Casts a ray in the given direction and returns true if the ray did not encounter other bodies before
		// colliding with this body. If a collision occurred, then this also sets the contact point in world space. The
		// direction should not be normalized, as the direction's magnitude determines the length of the ray.
		[[nodiscard]] bool TestRay(glm::vec3 origin, glm::vec3 directionWithMagnitude, glm::vec3& contactPoint) const;

		// Set the world space position of the body. Can choose to forcefully activate the body if it's sleeping.
		void SetPosition(glm::vec3 position, bool forceActivate = false) const;
		// Set the world space linear velocity of the center of mass (m/s)
		void SetLinearVelocity(glm::vec3 velocity) const;
		// Set the world space linear velocity of the center of mass (m/s), ensuring the velocity is clamped against the maximum linear velocity
		void SetLinearVelocityClamped(glm::vec3 velocity) const;
		// Set the maximum linear velocity that the body can achieve (m/s). Used to prevent the system from exploding.
		void SetMaxLinearVelocity(float velocity) const;
		// Set the world space angular velocity of the center of mass (rad/s)
		void SetAngularVelocity(glm::vec3 velocity) const;
		// Set the world space angular velocity of the center of mass (rad/s), ensuring the velocity is clamped against the maximum angular velocity
		void SetAngularVelocityClamped(glm::vec3 velocity) const;
		// Set the maximum angular velocity that the body can achieve (rad/s). Used to prevent the system from exploding.
		void SetMaxAngularVelocity(float velocity) const;
		// Set the world space rotation of the body. Can choose to forcefully activate the body if it's sleeping.
		void SetRotation(glm::quat rotation, bool forceActivate = false) const;
		// Set the friction (strictly between 0 and 1, 0 = no friction, 1 = friction force equals force that presses the two bodies together)
		void SetFriction(float friction) const;
		// Set the gravity factor, which is a multiplier on gravity's influence (0 = no gravity, 1 = normal gravity, >1 = more gravity)
		void SetGravityFactor(float gravityFactor) const;
		// Set the restitution (strictly between 0 and 1, 0 = completely inelastic collision response, 1 = completely elastic collision response)
		void SetRestitution(float restitution) const;
		// Set the motion quality of the body
		void SetMotionQuality(MotionQuality quality) const;
		// Change the body to a sensor. A sensor will receive collision callbacks, but will not cause any collision
		// responses and can be used as a trigger volume. The cheapest sensor (in terms of CPU usage) is a
		// MotionType::Static (which may still be moved using SetPosition). These sensors will only detect collisions
		// with active MotionType::Dynamic or MotionType::Kinematic bodies. As soon as a body goes to sleep, the contact
		// point with the sensor will be lost. If you make a sensor MotionType::Dynamic or MotionType::Kinematic and
		// activate it, the sensor will be able to detect collisions with sleeping bodies. An active sensor will never
		// go to sleep automatically.
		void SetIsSensor(bool isSensor) const;

		// Add force (N) at center of mass for the next time step
		void AddForce(glm::vec3 force) const;
		// Add force (N) at inPosition for the next time step
		void AddForce(glm::vec3 force, glm::vec3 point) const;
		// Adds an impulse to the center of mass (kg m/s)
		void AddImpulse(glm::vec3 impulse) const;
		// Adds an impulse to the point in world space (kg m/s)
		void AddImpulse(glm::vec3 impulse, glm::vec3 point) const;
		// Adds an angular impulse in world space (N m/s)
		void AddAngularImpulse(glm::vec3 impulse) const;
		// Adds torque (N m) for the next time step
		void AddTorque(glm::vec3 torque) const;
		// Set the velocity of the body such that it will translate/rotate by position/rotation in some seconds
		void MoveKinematic(glm::vec3 position, glm::quat rotation, float seconds) const;

		friend inline bool operator<(const Body& lhs, const Body& rhs) { return lhs.id < rhs.id; }
		friend inline bool operator>(const Body& lhs, const Body& rhs) { return lhs.id > rhs.id; }
		friend inline bool operator<=(const Body& lhs, const Body& rhs) { return lhs.id <= rhs.id; }
		friend inline bool operator>=(const Body& lhs, const Body& rhs) { return lhs.id >= rhs.id; }
		friend inline bool operator==(const Body& lhs, const Body& rhs) { return lhs.id == rhs.id; }
		friend inline bool operator!=(const Body& lhs, const Body& rhs) { return lhs.id != rhs.id; }

	private:
		friend class Manager;

		friend class Character;

		Body(uint32_t id, bool destructible = true);
		uint32_t id = 0;
		bool destructible = true;
	};

	// The set of parameters that govern the creation of all character bodies.
	struct CharacterCreationProperties {
	public:
		glm::vec3 Position{};
		glm::quat Rotation = glm::identity<glm::quat>();
		float Weight = 80.0f; // kg
		float MaxSlopeAngle = glm::radians(50.0f); // radians
		float GravityFactor = 1.0f;
		float Width = 0.41f; // meters
		float Height = 1.75f; // meters
	};

	// A special physics body that is meant to be controlled by a player.
	class Character {
	public:
		~Character();

		// Get the physics body associated with this character.
		Body GetBody();
		// Get the state of the ground in relation to the character.
		GroundState GetGroundState();
		// Get the normal of the ground. Only relevant when the character has a proper ground state.
		glm::vec3 GetGroundNormal();

		// Calculates additional steps that are specific for the contained physics body. This should be called after
		// all interactions with the body. This should only be called from FixedUpdate.
		void PostSimulation();

	private:
		friend class Manager;

		Character(void* character);
		void* character;
	};

	// The result of a ray cast in the world space.
	struct RayResult {
	public:
		// The body that was hit
		Body Body;
		// The point, in world space, that the ray made contact with the body
		glm::vec3 ContactPoint;
	};

	// Receives collision events that occur between physics bodies.
	class ContactListener {
		//TODO: implement
		//void OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override;
		//void OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override;
		//void OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair) override;
	};

	// Validates collision events before they're passed to listeners. This allows for fine-grained validation above what
	// layers offer, however they occur after quite a bit of processing has been done. If possible, prefer to separate
	// bodies by layer rather than in the validator.
	class ContactValidator {
		//TODO: implement
		//JPH::ValidateResult OnContactValidate(const JPH::Body& inBody1, const JPH::Body& inBody2, JPH::RVec3Arg inBaseOffset, const JPH::CollideShapeResult& inCollisionResult) override;
	};

	// Enable the physics engine. The engine is enabled by default, so this should only be called if the engine was
	// manually disabled.
	void Enable();
	// Disable the physics engine. When disabled, the simulation will not advance, however all other aspects will still
	// function, such as creating new bodies, updating gravity, etc.
	void Disable();
	// Set the number of updates that the physics engine will calculate per second.
	void SetUpdateRate(double rate);
	// Get the gravity
	glm::vec3 GetGravity();
	// Set the gravity
	void SetGravity(glm::vec3 gravity);
	// Get the maximum number of physics bodies that may be created at any one time.
	std::uint32_t GetMaxNumberOfBodies();
	// Casts a ray in world space against all bodies and returns those that collide with the ray.
	std::vector<RayResult> CastRay(glm::vec3 origin, glm::vec3 direction, float magnitude, RayFilter filter);
	// Casts a ray in world space against all bodies and returns those that collide with the ray. The direction should
	// not be normalized, as the direction's magnitude determines the length of the ray.
	std::vector<RayResult> CastRay(glm::vec3 origin, glm::vec3 directionWithMagnitude, RayFilter filter);

	// The set of parameters that govern the creation of all bodies.
	struct BodyCreationProperties {
	public:
		glm::vec3 Position{};
		glm::quat Rotation = glm::identity<glm::quat>();
		MotionType MotionType = MotionType::Dynamic;
		MotionQuality MotionQuality = MotionQuality::Discrete;
		Mass Mass{};
	};

	// Returns a new sphere. Will return a nullptr once the max body count has been reached.
	std::unique_ptr<Body> CreateSphere(float radius, BodyCreationProperties properties);
	// Returns a new box. Will return a nullptr once the max body count has been reached.
	std::unique_ptr<Body> CreateBox(glm::vec3 shape, BodyCreationProperties properties);
	// Returns a new capsule. Height represents the distance between the center of the two hemispheres, meaning the
	// total length will be (height + radius * 2). Will return a nullptr once the max body count has been reached.
	std::unique_ptr<Body> CreateCapsule(float height, float radius, BodyCreationProperties properties);
	// Returns a new tapered capsule. Will return a nullptr once the max body count has been reached.
	std::unique_ptr<Body> CreateTaperedCapsule(float height, float topRadius, float bottomRadius, BodyCreationProperties properties);
	// Returns a new cylinder. Will return a nullptr once the max body count has been reached.
	std::unique_ptr<Body> CreateCylinder(float height, float radius, BodyCreationProperties properties);

	// Returns a new character. Will return a nullptr once the max body count has been reached.
	std::unique_ptr<Character> CreateCharacter(CharacterCreationProperties properties);
}

#endif //ENGINE_PHYSICS_PHYSICS_HPP
