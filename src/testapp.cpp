// Copyright © 2022-2023 Daylon Wilkins & James Cor
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <engine/application.hpp>
#include <iostream>

struct ApplicationData {
	Rml::String TestString = "some string of text";
} RmlTestData;

class DemoRender : public engine::Application {
public:
	DemoRender() = default;
	~DemoRender() override = default;

	bool Initialize() override {
		if (!Rml::LoadFontFace("test-assets/rmlui/LatoLatin-Regular.ttf")) {
			engine::log::Error("Unable to load fonts");
			return false;
		}
		engine::graphics::SetVSync(false);

		master = GetAudioManager()->GetMasterSoundGroup();
		child1 = GetAudioManager()->CreateSoundGroup(master);
		child2 = GetAudioManager()->CreateSoundGroup(master);
		grandchild1 = GetAudioManager()->CreateSoundGroup(child1);
		grandchild2 = GetAudioManager()->CreateSoundGroup(child2);
		GetAudioManager()->SetVolume(0, master);

		sin220 = GetAudioManager()->LoadPositionalSoundEffect("test-assets/sounds/220.wav", master);
		if (sin220) {
			sin220->Play();
		}

		sin330 = GetAudioManager()->LoadPositionalSoundEffect("test-assets/sounds/330.wav", child1);
		if (sin330) {
			sin330->Play();
		}

		sin440 = GetAudioManager()->LoadPositionalSoundEffect("test-assets/sounds/440.wav", child2);
		if (sin440) {
			sin440->Play();
		}

		sin550 = GetAudioManager()->LoadPositionalSoundEffect("test-assets/sounds/550.wav", grandchild1);
		if (sin550) {
			sin550->Play();
		}

		sin660 = GetAudioManager()->LoadPositionalSoundEffect("test-assets/sounds/660.wav", grandchild2);
		if (sin660) {
			sin660->Play();
		}

		listener = GetAudioManager()->GetListener();

		if (Rml::ElementDocument* document = GetUIContext()->LoadDocument("test-assets/rmlui/benchmark.rml")) {
			document->SetProperty("z-index", "-1");
			document->Show();
			if (Rml::DataModelConstructor constructor = GetUIContext()->CreateDataModel("dmodel")) {
				constructor.Bind("dvalue", &RmlTestData.TestString);
			}
			if (auto el = document->GetElementById("performance")) {
				el->SetInnerRML(R"(<div data-model="dmodel"><input type="text" data-value="dvalue"/></div>)");
			}
		}

		if (Rml::ElementDocument* document = GetUIContext()->LoadDocument("test-assets/rmlui/transform.rml")) {
			window1 = document;
			document->SetProperty("z-index", "1");
			document->GetElementById("title")->SetInnerRML("Orthographic Transform");
			document->SetProperty(Rml::PropertyId::Left, Rml::Property(80, Rml::Property::DP));
			document->SetProperty(Rml::PropertyId::Top, Rml::Property(20, Rml::Property::DP));
			document->Show();
		}
		if (Rml::ElementDocument* document = GetUIContext()->LoadDocument("test-assets/rmlui/transform.rml")) {
			window2 = document;
			document->SetProperty("z-index", "1");
			document->GetElementById("title")->SetInnerRML("Perspective Transform");
			document->SetProperty(Rml::PropertyId::Left, Rml::Property(700, Rml::Property::DP));
			document->SetProperty(Rml::PropertyId::Top, Rml::Property(20, Rml::Property::DP));
			document->Show();
			std::stringstream s;
			s << "perspective(" << 800 << "dp) ";
			document->SetProperty("transform", s.str());
		}

		auto keyboardID = GetInputHandler()->ListConnectedKeyboards()[0];
		auto keyCallback = [&](engine::input::Keyboard::Key key, engine::input::KeyState state)->void {
			if (state.IsDown) {
				switch (key) {
					case engine::input::Keyboard::Key::Z:
					case engine::input::Keyboard::Key::Escape: {
						GetInputHandler()->SetMouseCaptureState(engine::input::Mouse::CaptureState::None);
						break;
					}
					case engine::input::Keyboard::Key::X: {
						GetInputHandler()->SetMouseCaptureState(engine::input::Mouse::CaptureState::Soft);
						break;
					}
					case engine::input::Keyboard::Key::C: {
						GetInputHandler()->SetMouseCaptureState(engine::input::Mouse::CaptureState::Hard);
						break;
					}
					case engine::input::Keyboard::Key::W: {
						engine::physics::Body body = character->GetBody();
						body.AddForce(glm::vec3{10000.0f, 0.0f, 0.0f});
						break;
					}
					case engine::input::Keyboard::Key::S: {
						engine::physics::Body body = character->GetBody();
						body.AddForce(glm::vec3{-10000.0f, 0.0f, 0.0f});
						break;
					}
					case engine::input::Keyboard::Key::A: {
						engine::physics::Body body = character->GetBody();
						body.AddForce(glm::vec3{0.0f, 0.0f, 10000.0f});
						break;
					}
					case engine::input::Keyboard::Key::D: {
						engine::physics::Body body = character->GetBody();
						body.AddForce(glm::vec3{0.0f, 0.0f, -10000.0f});
						break;
					}
					default:
						break;
				}
			}
		};
		GetInputHandler()->GetDevice(keyboardID)->AddCallback(engine::input::Keyboard::Key::Z, keyCallback);
		GetInputHandler()->GetDevice(keyboardID)->AddCallback(engine::input::Keyboard::Key::Escape, keyCallback);
		GetInputHandler()->GetDevice(keyboardID)->AddCallback(engine::input::Keyboard::Key::X, keyCallback);
		GetInputHandler()->GetDevice(keyboardID)->AddCallback(engine::input::Keyboard::Key::C, keyCallback);

		GetInputHandler()->GetDevice(keyboardID)->AddCallback(engine::input::Keyboard::Key::W, keyCallback);
		GetInputHandler()->GetDevice(keyboardID)->AddCallback(engine::input::Keyboard::Key::S, keyCallback);
		GetInputHandler()->GetDevice(keyboardID)->AddCallback(engine::input::Keyboard::Key::A, keyCallback);
		GetInputHandler()->GetDevice(keyboardID)->AddCallback(engine::input::Keyboard::Key::D, keyCallback);

		return true;
	}

	void Shutdown() override {}

	void TestRayCasting() {
		ImVec2 initialWindowsPos = ImVec2(6.0f * (float)engine::graphics::GetWindowWidth() / 8.0f, 200.0f);
		ImGui::SetNextWindowPos(initialWindowsPos, ImGuiCond_FirstUseEver);
		// Set the details of the ray to cast
		ImGui::Begin("Ray Cast Testing", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text("Set the ray details");
		static glm::vec3 rayPos{};
		ImGui::InputFloat3("Origin", reinterpret_cast<float*>(&rayPos));
		static glm::vec3 rayTarget{1.0f, 0.0f, 1.0f};
		ImGui::InputFloat3("Target", reinterpret_cast<float*>(&rayTarget));
		static float rayMagnitude = 10.0f;
		ImGui::InputFloat("Magnitude", &rayMagnitude);
		ImGui::Separator();
		ImGui::Text("Spawn a cube");
		static glm::vec2 cubeSpawn{1.0, 1.0};
		ImGui::InputFloat2("Location", reinterpret_cast<float*>(&cubeSpawn));
		if (ImGui::Button("Spawn")) {
			allBodies.emplace_back(engine::physics::CreateBox({1, 1, 1}, engine::physics::BodyCreationProperties{
				.Position = {cubeSpawn.x, 3.0f, cubeSpawn.y},
			}));
		}
		ImGui::Separator(); // Cast a ray against all bodies in the world space
		ImGui::Text("Cast a ray in world space");
		static engine::physics::RayFilter rayFilter = engine::physics::RayFilter::AllHit;
		{
			const char* items[] = {"All", "Any", "Closest", "Furthest"};
			static const char* currentItem = items[0];
			if (ImGui::BeginCombo("Filter##combo_world_space", currentItem)) {
				for (int i = 0; i < IM_ARRAYSIZE(items); i++) {
					bool selected = (currentItem == items[i]);
					if (ImGui::Selectable(items[i], selected)) {
						currentItem = items[i];
						switch (i) {
							case 0:
								rayFilter = engine::physics::RayFilter::AllHit;
								break;
							case 1:
								rayFilter = engine::physics::RayFilter::AnyHit;
								break;
							case 2:
								rayFilter = engine::physics::RayFilter::ClosestHit;
								break;
							case 3:
								rayFilter = engine::physics::RayFilter::FurthestHit;
								break;
							default:
								break;
						}
					}
					if (selected) {
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}
		}
		static std::vector<engine::physics::RayResult> hits;
		if (ImGui::Button("Cast Ray##button_world_space")) {
			hits = engine::physics::CastRay(rayPos, glm::normalize(rayTarget - rayPos), rayMagnitude, rayFilter);
		}
		if (!hits.empty()) {
			ImGui::Text("Hits");
			for (auto& hit: hits) {
				glm::vec3 position = hit.ContactPoint;
				ImGui::BulletText("%u: %.2f, %.2f, %.2f", hit.Body.GetID(), position.x, position.y, position.z);
			}
		}
		ImGui::Separator(); // Handle checking a ray against a single body
		ImGui::Text("Cast a ray at a specific target");
		static engine::physics::Body* targetBody = nullptr;
		static engine::physics::Body* lastTarget = nullptr;
		static bool targetHit = false;
		static glm::vec3 targetContactPoint{};
		{
			static std::string currentBody{};
			if (ImGui::BeginCombo("Target##combo_specific_target", currentBody.c_str())) {
				for (auto& body: allBodies) {
					std::string item = std::to_string(body->GetID());
					bool selected = (currentBody == item);
					if (ImGui::Selectable(item.c_str(), selected)) {
						targetBody = body.get();
						currentBody = item;
					}
					if (selected) {
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}
		}
		if (ImGui::Button("Cast Ray##button_specific_target") && targetBody != nullptr) {
			targetHit = targetBody->TestRay(rayPos, rayTarget - rayPos, rayMagnitude, targetContactPoint);
			lastTarget = targetBody;
		}
		if (targetHit) {
			ImGui::Text("%u: Hit", lastTarget->GetID());
			ImGui::BulletText("%.2f, %.2f, %.2f", targetContactPoint.x, targetContactPoint.y, targetContactPoint.z);
		} else if (lastTarget != nullptr) {
			ImGui::Text("%u: Missed or Obstructed", lastTarget->GetID());
		}
		ImGui::Separator();
		if (!allBodies.empty()) {
			ImGui::Text("All Bodies");
			for (auto& body: allBodies) {
				glm::vec3 position = body->GetCenterOfMassPosition();
				ImGui::BulletText("%u: %.2f, %.2f, %.2f", body->GetID(), position.x, position.y, position.z);
			}
		}
		ImGui::End();
	}

	bool Update(double deltaTime) override {
		static float masterf = 0.0f;
		static float child1f = 0.0f;
		static float child2f = 0.0f;
		static float grandchild1f = 0.0f;
		static float grandchild2f = 0.0f;

		ImVec2 initialWindowsPos(10.0f, 10.0f);
		ImGui::SetNextWindowPos(initialWindowsPos, ImGuiCond_FirstUseEver);
		if (ImGui::Begin("Test Window", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::SliderFloat("Master", &masterf, 0.0f, 1.0f);
			ImGui::SliderFloat("Child1", &child1f, 0.0f, 1.0f);
			ImGui::SliderFloat("Child2", &child2f, 0.0f, 1.0f);
			ImGui::SliderFloat("Grandchild1", &grandchild1f, 0.0f, 1.0f);
			ImGui::SliderFloat("Grandchild2", &grandchild2f, 0.0f, 1.0f);
		}
		GetAudioManager()->SetVolume(masterf, master);
		GetAudioManager()->SetVolume(child1f, child1);
		GetAudioManager()->SetVolume(child2f, child2);
		GetAudioManager()->SetVolume(grandchild1f, grandchild1);
		GetAudioManager()->SetVolume(grandchild2f, grandchild2);

		static bool trigger = false;
		static float impulse = 10.0f;
		static float sphereMass = 1.0f;
		static float lastSphereMass = sphereMass;
		static double initialCount = 0;
		initialCount += deltaTime;
		if (initialCount > 0.5) {
			initialWindowsPos = ImVec2((float)engine::graphics::GetWindowWidth() / 2.0f, 10.0f);
			ImGui::SetNextWindowPos(initialWindowsPos, ImGuiCond_FirstUseEver);
			ImGui::Begin("Sphere Velocity Tester", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
			if (!trigger) {
				trigger = true;
				auto properties = engine::physics::BodyCreationProperties{
					.Position = {0.0f, -1.0f, 0.0f},
					.MotionType = engine::physics::MotionType::Static,
				};
				floor = engine::physics::CreateBox({100.0f, 1.0f, 100.0f}, properties).release();
				allBodies.push_back(std::unique_ptr<engine::physics::Body>(floor));
				properties = engine::physics::BodyCreationProperties{
					.Position = {40.0f, 2.0f, 30.0f},
					.MotionType = engine::physics::MotionType::Dynamic,
					.Mass = {.Weight = sphereMass},
				};
				sphere = engine::physics::CreateSphere(0.5f, properties).release();
				allBodies.push_back(std::unique_ptr<engine::physics::Body>(sphere));
				sphere->SetLinearVelocity({0.0f, -1.0f, 0.0f});
				auto keyboardID = GetInputHandler()->ListConnectedKeyboards()[0];
				auto keyCallback = [&](engine::input::Keyboard::Key, engine::input::KeyState state)->void {
					if (sphere && state.IsDown) {
						sphere->Activate();
						sphere->AddImpulse({0, impulse, 0});
					}
				};
				GetInputHandler()->GetDevice(keyboardID)->AddCallback(engine::input::Keyboard::Key::Space, keyCallback);
			} else if (lastSphereMass != sphereMass) {
				lastSphereMass = sphereMass;
				auto properties = engine::physics::BodyCreationProperties{
					.Position = {40.0f, 2.0f, 30.0f},
					.MotionType = engine::physics::MotionType::Dynamic,
					.Mass = {.Weight = sphereMass},
				};
				sphere = engine::physics::CreateSphere(0.5f, properties).release();
				allBodies[1].reset(sphere);
				sphere->SetLinearVelocity({0.0f, -1.0f, 0.0f});
			}
			glm::vec3 position = sphere->GetCenterOfMassPosition();
			glm::vec3 velocity = sphere->GetLinearVelocity();
			ImGui::InputFloat("Sphere Mass (kilograms)", &sphereMass);
			ImGui::InputFloat("Impulse Amount", &impulse);
			ImGui::InputFloat3("Sphere Position", reinterpret_cast<float*>(&position));
			ImGui::InputFloat3("Sphere Velocity", reinterpret_cast<float*>(&velocity));
			ImGui::End();
		}

		static bool charTrigger = false;
		if (initialCount > 0.5) {
			if (!charTrigger) {
				charTrigger = true;
				engine::physics::CharacterCreationProperties charProps;
				charProps.Position = glm::vec3{1, 1, 1};
				character = engine::physics::CreateCharacter(charProps);
			}

			ImGui::SetNextWindowPos(initialWindowsPos, ImGuiCond_FirstUseEver);
			ImGui::Begin("character position", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
			auto body = character->GetBody();
			glm::vec3 pos = body.GetPosition();
			ImGui::LabelText("FPS", "(%2f, %2f, %2f)", pos.x, pos.y, pos.z);
			ImGui::End();
		}

		static bool readFileSystemOnce = false;
		if (!readFileSystemOnce) {
			readFileSystemOnce = true;
			engine::fs::NativeFileSystem fs;
			int count = fs.EnumerateFiles("test-assets/*/", {".wav", ".tga"}, [](std::string_view s) {
				std::cout << s << std::endl;
			}, true);
			std::cout << "File Count: " << count << std::endl;
		}

		ImGui::End();

		static float deg = 0.0f;
		static engine::utils::RollingAverage<double> deltaTimesAvg(10);
		deltaTimesAvg.Update(deltaTime);
		initialWindowsPos = ImVec2(10, ((float)engine::graphics::GetWindowHeight() * 8.0f) / 9.0f);
		ImGui::SetNextWindowPos(initialWindowsPos, ImGuiCond_FirstUseEver);
		ImGui::Begin("Rotation", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::SliderFloat("Degrees", &deg, 0, 360);
		ImGui::SameLine();
		static bool spin = true;
		ImGui::Checkbox("Spin", &spin);
		if (spin) {
			deg += (float)deltaTime * 50.0f;
			if (deg > 360.0f) {
				deg -= 360.0f;
			}
		}
		ImGui::LabelText("FPS", "FPS: %d", (int)(1.0 / deltaTimesAvg.GetCurrentAverage()));
		ImGui::End();
		if (window1) {
			std::stringstream s;
			s << "rotate3d(0.0, 1.0, 0.0, " << deg << "deg)";
			window1->SetProperty("transform", s.str());
		}
		if (window2) {
			std::stringstream s;
			s << "perspective(" << 800 << "dp) ";
			s << "rotate3d(0.0, 1.0, 0.0, " << deg << "deg)";
			window2->SetProperty("transform", s.str());
		}

		TestRayCasting();

		return true;
	}

	bool FixedUpdate(double deltaTime) override {
		return true;
	}

	bool Draw(double deltaTime) override {
		return true;
	}

	engine::ApplicationOptions StartOptions() override {
		engine::ApplicationOptions options{
			.Title = "Galactic Engine",
			.Width = 1280,
			.Height = 720,
		};
		return options;
	}

private:
	std::unique_ptr<engine::audio::PositionalSound> sin220;
	std::unique_ptr<engine::audio::PositionalSound> sin330;
	std::unique_ptr<engine::audio::PositionalSound> sin440;
	std::unique_ptr<engine::audio::PositionalSound> sin550;
	std::unique_ptr<engine::audio::PositionalSound> sin660;
	engine::audio::Listener* listener;
	engine::audio::SoundGroup master;
	engine::audio::SoundGroup child1;
	engine::audio::SoundGroup child2;
	engine::audio::SoundGroup grandchild1;
	engine::audio::SoundGroup grandchild2;
	engine::physics::Body* floor;
	engine::physics::Body* sphere;
	std::vector<std::unique_ptr<engine::physics::Body>> allBodies;
	Rml::ElementDocument* window1;
	Rml::ElementDocument* window2;

	std::unique_ptr<engine::physics::Character> character;
};

engine::Application* engine::NewApplication() {
	return new DemoRender();
}
