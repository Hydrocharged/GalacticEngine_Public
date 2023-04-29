// Copyright © 2022-2023 Daylon Wilkins & James Cor
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef ENGINE_GUI_BACKEND_HPP
#define ENGINE_GUI_BACKEND_HPP

#include <RmlUi/Core.h>

namespace engine::gui {
	class Backend : public Rml::SystemInterface {
	public:
		Backend();
		~Backend() override;

		void Initialize(void* data);
		double GetElapsedTime() override;
		void SetMouseCursor(const Rml::String& cursorName) override;
		void SetClipboardText(const Rml::String& textUtf8) override;
		void GetClipboardText(Rml::String& text) override;
		void ActivateKeyboard(Rml::Vector2f caretPosition, float lineHeight) override;

		class PlatformImplementation;

		friend class PlatformImplementation;

	private:
		std::unique_ptr<PlatformImplementation> platImpl;
	};
}

#endif //ENGINE_GUI_BACKEND_HPP
