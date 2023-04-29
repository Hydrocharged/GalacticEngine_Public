// Copyright © 2022-2023 Daylon Wilkins & James Cor
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifdef PLATFORM_WIN32

#include <engine/backend/windows.hpp>

// Adapted from https://github.com/blat-blatnik/Snippets/blob/98e4f79e41d976d00e1e7691f58b4c5621f9591b/precise_sleep.c
void engine::Application::PlatformImplementation::WaitFor(double seconds) {
	LARGE_INTEGER qpc;
	QueryPerformanceCounter(&qpc);
	INT64 targetQpc = (INT64)(qpc.QuadPart + seconds * Data->WaitQpcPerSecond);

	if (Data->WaitTimer) {
		const double TOLERANCE = 0.001'02;
		INT64 maxTicks = (INT64)Data->WaitSchedulerPeriodMs * 9'500;
		for (;;) {
			double remainingSeconds = (targetQpc - qpc.QuadPart) / (double)Data->WaitQpcPerSecond;
			INT64 sleepTicks = (INT64)((remainingSeconds - TOLERANCE) * 10'000'000);
			if (sleepTicks <= 0) {
				break;
			}

			LARGE_INTEGER due;
			due.QuadPart = -(sleepTicks > maxTicks ? maxTicks : sleepTicks);
			SetWaitableTimerEx(Data->WaitTimer, &due, 0, NULL, NULL, NULL, 0);
			WaitForSingleObject(Data->WaitTimer, INFINITE);
			QueryPerformanceCounter(&qpc);
		}
	} else {
		const double TOLERANCE = 0.000'02;
		double sleepMs = (seconds - TOLERANCE) * 1000 - Data->WaitSchedulerPeriodMs;
		int sleepSlices = (int)(sleepMs / Data->WaitSchedulerPeriodMs);
		if (sleepSlices > 0) {
			Sleep((DWORD)sleepSlices * Data->WaitSchedulerPeriodMs);
		}
		QueryPerformanceCounter(&qpc);
	}

	while (qpc.QuadPart < targetQpc) {
		YieldProcessor();
		QueryPerformanceCounter(&qpc);
	}
}

#endif //PLATFORM_WIN32
