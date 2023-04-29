// Copyright © 2022-2023 Daylon Wilkins & James Cor
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifdef PLATFORM_WIN32

#include <engine/backend/windows.hpp>
#include <engine/log/log.hpp>
#include <engine/strings/strings.hpp>
#pragma comment(lib, "Winmm.lib") // timeGetDevCaps, timeBeginPeriod

engine::Application::PlatformImplementation::PlatformImplementation(engine::Application* app) {
	application = app;
	application->platImpl.reset(this);
	this->Data = std::make_unique<engine::Application::PlatformImplementation::PlatformData>();
	this->Callbacks = std::make_unique<engine::Application::PlatformImplementation::PlatformCallbacks>(application->commonImpl.get());

	// Get the information needed for WaitFor
	this->Data->WaitTimer = CreateWaitableTimerExW(nullptr, nullptr, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
	TIMECAPS caps;
	timeGetDevCaps(&caps, sizeof caps);
	timeBeginPeriod(caps.wPeriodMin);
	this->Data->WaitSchedulerPeriodMs = (int)caps.wPeriodMin;
	LARGE_INTEGER qpf;
	QueryPerformanceFrequency(&qpf);
	this->Data->WaitQpcPerSecond = qpf.QuadPart;
}

engine::Application::PlatformImplementation::~PlatformImplementation() = default;

engine::Application::CommonImplementation* engine::Application::PlatformImplementation::GetCommonImplementation() {
	return application->commonImpl.get();
}

int WINAPI WinMain(_In_ HINSTANCE hInstance,
				   _In_opt_ HINSTANCE hPrevInstance,
				   _In_ LPSTR lpCmdLine,
				   _In_ int nShowCmd) {
#if !defined(NDEBUG) || defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif //!defined(NDEBUG) || defined(_DEBUG)

	auto application = std::unique_ptr<engine::Application>(engine::NewApplication());
	auto platImpl = new engine::Application::PlatformImplementation(application.get());
	platImpl->Data->InstanceHandle = hInstance;
	platImpl->Data->ShowCmd = nShowCmd;

	auto commonImpl = platImpl->GetCommonImplementation();
	if (!commonImpl->Initialize()) {
		return 1;
	}

	while (commonImpl->UpdateLoop()) {}
	application->Shutdown();

	return 0;
}

#endif //PLATFORM_WIN32
