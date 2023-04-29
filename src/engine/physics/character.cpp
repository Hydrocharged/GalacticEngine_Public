// Copyright © 2022-2023 Daylon Wilkins & James Cor
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <engine/physics/manager.hpp>

engine::physics::Character::Character(void* character) : character(character) {
	auto jphCharacter = reinterpret_cast<JPH::Character*>(this->character);
	jphCharacter->AddToPhysicsSystem(JPH::EActivation::Activate);
}

engine::physics::Character::~Character() {
	auto jphCharacter = reinterpret_cast<JPH::Character*>(character);
	jphCharacter->RemoveFromPhysicsSystem();
	// Deleting the character pointer also destroys the body associated with the character
	delete jphCharacter;
}

engine::physics::Body engine::physics::Character::GetBody() {
	auto character = reinterpret_cast<JPH::Character*>(this->character);
	auto bodyID = character->GetBodyID().GetIndexAndSequenceNumber();
	return Body(bodyID, false);
}

engine::physics::GroundState engine::physics::Character::GetGroundState() {
	auto character = reinterpret_cast<JPH::Character*>(this->character);
	return toGLM(character->GetGroundState());
}

void engine::physics::Character::PostSimulation() {
	auto character = reinterpret_cast<JPH::Character*>(this->character);
	character->PostSimulation(0.05f);
}

glm::vec3 engine::physics::Character::GetGroundNormal() {
	auto character = reinterpret_cast<JPH::Character*>(this->character);
	return toGLM(character->GetGroundNormal());
}