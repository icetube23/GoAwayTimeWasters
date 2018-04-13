// GoAwayTimeWasters.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <ctime>
#include <vector>
#include <string>
#include <iostream>

#include <Windows.h>
#include <TlHelp32.h>
#include <atlbase.h>
#include <UIAutomation.h>

namespace gatw {
	std::vector<std::wstring> badProcesses = {
		L"notepad.exe"
	};


	void killProcessesByName(const wchar_t *processName) {
		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);
		PROCESSENTRY32 entry;
		entry.dwSize = sizeof(entry);
		BOOL hasEntry = Process32First(snapshot, &entry);
		
		while (hasEntry) {
			if (wcscmp(entry.szExeFile, processName) == 0) {
				HANDLE process = OpenProcess(PROCESS_TERMINATE, 0, entry.th32ProcessID);
				if (process != NULL) {
					TerminateProcess(process, 1000);
					CloseHandle(process);
				}
			}

			hasEntry = Process32Next(snapshot, &entry);
		}

		CloseHandle(snapshot);
	 }


	void killProcesses() {
		for (int i = 0; i < badProcesses.size(); i++) {
			killProcessesByName(badProcesses[i].data());
		}
	}


	std::wstring getTabName() {
		CComQIPtr<IUIAutomation> uia;
		if (FAILED(uia.CoCreateInstance(CLSID_CUIAutomation)) || !uia) {
			return L"ERRROR";
		}

		CComPtr<IUIAutomationCondition> cond;
		uia->CreatePropertyCondition(UIA_ControlTypePropertyId, CComVariant(0xC354), &cond);

		HWND hwnd = NULL;
		while (true) {
			hwnd = FindWindowEx(NULL, hwnd, L"Chrome_WidgetWin_1", NULL);
			if (!hwnd) {
				return L"ERROR";
			}
			if (!IsWindowVisible(hwnd)) {
				continue;
			}

			CComPtr<IUIAutomationElement> root;
			if (FAILED(uia->ElementFromHandle(hwnd, &root)) || !root) {
				return L"ERROR";
			}

			CComPtr<IUIAutomationElement> urlBox;
			if (FAILED(root->FindFirst(TreeScope_Descendants, cond, &urlBox)) || !urlBox) {
				continue;
			}

			CComVariant url;
			urlBox->GetCurrentPropertyValue(UIA_ValueValuePropertyId, &url);
			return url.bstrVal;
		}
	}


	[[ noreturn ]] void start() {
		CoInitialize(NULL);

		while (true) {
			time_t rawTime;
			time(&rawTime);

			tm timeInfo;
			localtime_s(&timeInfo, &rawTime);

			int wday = timeInfo.tm_wday;
			int tmhour = timeInfo.tm_hour;

			if (wday >= 1 && wday <= 5 && tmhour >= 1 && tmhour <= 16) {
				//killProcesses();
				std::wcout << getTabName() << std::endl;
			}

			Sleep(250);
		}
	}
}

int main() {
	gatw::start();
    return 0;
}

