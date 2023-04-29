// Copyright © 2022-2023 Daylon Wilkins & James Cor
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef ENGINE_INPUT_INPUT_HPP
#define ENGINE_INPUT_INPUT_HPP

#include <functional>
#include <memory>
#include <vector>
#include <unordered_map>

namespace engine {
	class Application;
}

namespace engine::input {
	class Handler;

	struct KeyState {
	public:
		bool IsDown = false;
		double StartTime = 0.0;
	};

	struct AnalogState {
	public:
		float X = 0.f;
		float Y = 0.f;
		float Deadzone = 0.0001f;
	};

	class Device {
	public:
		enum class Type {
			Mouse,
			Keyboard,
			Gamepad
		};

		struct ID {
		public:
			std::uint32_t Raw;

			[[nodiscard]] Type GetType() const;
		};

		typedef std::function<void(ID)> DisconnectCallback;

	public:
		Device(Handler* handler, Type type, std::uint16_t id);
		virtual ~Device();

		[[nodiscard]] Handler* GetHandler() const;
		[[nodiscard]] ID GetID() const;
		[[nodiscard]] Type GetType() const;
		[[nodiscard]] bool IsConnected() const;
		void AddCallback(DisconnectCallback callback);

	protected:
		friend class Handler;

		virtual void update() = 0;

		Handler* handler;
		ID id{};
		bool isConnected;

	private:
		std::vector<DisconnectCallback> onDisconnect;
	};

	class Mouse : public Device {
	public:
		enum class CaptureState {
			None,
			Soft,
			Hard
		};
		enum class Button : std::uint8_t {
			Left,
			Middle,
			Right,
		};
		typedef std::function<void(Button, KeyState)> ButtonCallback;

		class Setter {
		public:
			~Setter();

			[[nodiscard]] std::shared_ptr<Mouse> GetMouse();
			[[nodiscard]] Device::ID GetID() const;
			void SetButton(Button button, bool isDown, double currentTime);
			void SetPosition(float x, float y);
			void SetDelta(float deltaX, float deltaY);
			void SetScrollWheel(float value);

		private:
			friend class Handler;

			Setter(std::shared_ptr<Mouse> mouse);

			std::shared_ptr<Mouse> mouse;
		};

	public:
		~Mouse() override;

		[[nodiscard]] KeyState GetButton(Button button) const;
		[[nodiscard]] bool IsButtonDown(Button button) const;
		[[nodiscard]] bool IsButtonUp(Button button) const;
		[[nodiscard]] bool IsButtonPressed(Button button) const;
		void GetPosition(float& x, float& y) const;
		void GetDelta(float& dX, float& dY) const;
		[[nodiscard]] float GetScrollWheel() const;
		void SetScrollWheelDeadzone(float value);
		void AddCallback(Button button, ButtonCallback callback);
		void ClearCallbacks(Button button);

	protected:
		void update() override;

	private:
		friend class Handler;

		friend class Setter;

		Mouse(Handler* handler, std::uint16_t id);

		struct buttonState {
			KeyState state;
			std::vector<ButtonCallback> callbacks;
		};

		std::vector<buttonState> buttons;
		std::vector<Button> pressedButtons;
		AnalogState scrollWheel;
		float posX = 0.f;
		float posY = 0.f;
		float dX = 0.f;
		float dY = 0.f;
	};

	class Keyboard : public Device {
	public:
		enum class Key : std::uint8_t {
			F1,
			F2,
			F3,
			F4,
			F5,
			F6,
			F7,
			F8,
			F9,
			F10,
			F11,
			F12,
			PrintScreen,
			ScrollLock,
			PauseBreak,
			NumLock,
			Number1,
			Number2,
			Number3,
			Number4,
			Number5,
			Number6,
			Number7,
			Number8,
			Number9,
			Number0,
			Backtick,
			Tilde,
			Exclamation,
			At,
			Pound, // Hash
			Dollar,
			Percent,
			Caret, // Circumflex
			Ampersand,
			Star,
			LeftParen,
			RightParen,
			Dash,
			Underscore,
			Equals,
			Plus,
			Backspace,
			Tab,
			CapsLock,
			LeftShift,
			RightShift,
			LeftCtrl,
			RightCtrl,
			LeftAlt,
			RightAlt,
			Enter,
			Space,
			Escape,
			LeftBrace,
			RightBrace,
			LeftCurlyBrace,
			RightCurlyBrace,
			Backslash,
			ForwardSlash,
			Pipe, // Separator
			Semicolon,
			Colon,
			Quote,
			DoubleQuote,
			Comma,
			Period,
			QuestionMark,
			LessThan,
			GreaterThan,
			A,
			B,
			C,
			D,
			E,
			F,
			G,
			H,
			I,
			J,
			K,
			L,
			M,
			N,
			O,
			P,
			Q,
			R,
			S,
			T,
			U,
			V,
			W,
			X,
			Y,
			Z,
			Insert,
			Delete,
			Home,
			End,
			PageUp,
			PageDown,
			ArrowUp,
			ArrowDown,
			ArrowLeft,
			ArrowRight,
		};
		typedef std::function<void(Key, KeyState)> KeyCallback;

		class Setter {
		public:
			~Setter();

			[[nodiscard]] std::shared_ptr<Keyboard> GetKeyboard();
			[[nodiscard]] Device::ID GetID() const;
			void SetKey(Key key, bool isDown, double currentTime);

		private:
			friend class Handler;

			Setter(std::shared_ptr<Keyboard> keyboard);

			std::shared_ptr<Keyboard> keyboard;
		};

	public:
		~Keyboard() override;

		[[nodiscard]] KeyState GetKey(Key key) const;
		[[nodiscard]] bool IsKeyDown(Key key) const;
		[[nodiscard]] bool IsKeyUp(Key key) const;
		[[nodiscard]] bool IsKeyPressed(Key key) const;
		void AddCallback(Key key, KeyCallback callback);
		void ClearCallbacks(Key key);

	protected:
		void update() override;

	private:
		friend class Handler;

		friend class Setter;

		Keyboard(Handler* handler, std::uint16_t id);

		struct keyState {
			KeyState state;
			std::vector<KeyCallback> callbacks;
		};

		std::vector<keyState> keys;
		std::vector<Key> pressedKeys;
	};

	class Gamepad : public Device {
	public:
		enum class Button : std::uint8_t {
			A, // Xbox A Button Position
			B, // Xbox B Button Position
			X, // Xbox X Button Position
			Y, // Xbox Y Button Position
			LShoulder,
			LStick,
			RShoulder,
			RStick,
			DPadLeft,
			DPadRight,
			DPadUp,
			DPadDown,
			Start,
			Options // Xbox Share Button
		};
		enum class Trigger : std::uint8_t {
			Left,
			Right,
		};
		enum class Stick : std::uint8_t {
			Left,
			Right,
		};
		typedef std::function<void(Button, KeyState)> ButtonCallback;

		class Setter {
		public:
			~Setter();

			[[nodiscard]] std::shared_ptr<Gamepad> GetGamepad();
			[[nodiscard]] Device::ID GetID() const;
			void SetButton(Button button, bool isDown, double currentTime);
			void SetTrigger(Trigger trigger, float value);
			void SetStick(Stick stick, float x, float y);

		private:
			friend class Handler;

			Setter(std::shared_ptr<Gamepad> gamepad);

			std::shared_ptr<Gamepad> gamepad;
		};

	public:
		~Gamepad() override;

		[[nodiscard]] KeyState GetButton(Button button) const;
		[[nodiscard]] bool IsButtonDown(Button button) const;
		[[nodiscard]] bool IsButtonUp(Button button) const;
		[[nodiscard]] bool IsButtonPressed(Button button) const;
		[[nodiscard]] float GetTrigger(Trigger trigger) const;
		void GetStick(Stick stick, float& x, float& y);
		void SetDeadzone(Trigger trigger, float value);
		void SetDeadzone(Stick stick, float value);
		void AddCallback(Button button, ButtonCallback callback);
		void ClearCallbacks(Button button);

	protected:
		void update() override;

	private:
		friend class Handler;

		friend class Setter;

		Gamepad(Handler* handler, std::uint16_t id);

		struct buttonState {
			KeyState state;
			std::vector<ButtonCallback> callbacks;
		};

		std::vector<buttonState> buttons;
		std::vector<Button> pressedButtons;
		std::vector<AnalogState> triggers;
		std::vector<AnalogState> sticks;
	};

	class Handler {
	public:
		struct MouseID {
			Device::ID ID;
		};
		struct KeyboardID {
			Device::ID ID;
		};
		struct GamepadID {
			Device::ID ID;
		};

	public:
		Handler(engine::Application* application);
		~Handler();

		std::shared_ptr<Mouse> GetDevice(MouseID id);
		std::shared_ptr<Keyboard> GetDevice(KeyboardID id);
		std::shared_ptr<Gamepad> GetDevice(GamepadID id);
		std::unique_ptr<Mouse::Setter> ConnectMouse();
		std::unique_ptr<Keyboard::Setter> ConnectKeyboard();
		std::unique_ptr<Gamepad::Setter> ConnectGamepad();
		std::vector<MouseID> ListConnectedMice();
		std::vector<KeyboardID> ListConnectedKeyboards();
		std::vector<GamepadID> ListConnectedGamepads();

		Mouse::CaptureState GetMouseCaptureState();
		void SetMouseCaptureState(Mouse::CaptureState state);

		void DisconnectDevice(Device::ID id);
		void Update();

	private:
		engine::Application* application;
		std::vector<std::shared_ptr<Device>> devices;
		std::uint16_t nextId = 0;
		Mouse::CaptureState mouseCaptureState = Mouse::CaptureState::None;
	};
}

#endif //ENGINE_INPUT_INPUT_HPP
