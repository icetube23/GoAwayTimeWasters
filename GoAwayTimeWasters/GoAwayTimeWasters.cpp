// GoAwayTimeWasters.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <ctime>
#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>

namespace gatw {
	void killProcessesByName(const wchar_t *processName) {
		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);
		PROCESSENTRY32 entry;
		entry.dwSize = sizeof(entry);

		CloseHandle(snapshot);
	 }


	[[ noreturn ]] void start() {
		while (true) {
			time_t rawTime;
			time(&rawTime);

			tm timeInfo;
			localtime_s(&timeInfo, &rawTime);

			int wday = timeInfo.tm_wday;
			int tmhour = timeInfo.tm_hour;

			if (wday >= 1 && wday <= 5 && tmhour >= 1 && tmhour <= 16) {
				std::cout << "Work!" << std::endl;
			} 
			else {
				std::cout << "Relax!" << std::endl;
			}

			Sleep(250);
		}
	}
}

int main() {
	gatw::start();
    return 0;
}

