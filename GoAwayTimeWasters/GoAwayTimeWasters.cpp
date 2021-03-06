// GoAwayTimeWasters.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <ctime>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <regex>

#include <Windows.h>
#include <TlHelp32.h>
#include <atlbase.h>
#include <UIAutomation.h>

namespace gatw {
	std::vector<std::wstring> badProcesses = {
		L"notepad.exe"
	};

	std::vector<std::wstring> badURLs = {
		L"bing.com"
	};

	std::string badProcessesPath = "BadProcesses.txt";
	std::string badURLsPath = "BadURLs.txt";


	bool fileExists(const std::string &path) {
		std::ifstream f(path);
		bool ex = f.good();
		f.close();
		return ex;
	}


	void createMissingConfig(std::string &path, std::vector<std::wstring> &data) {
		if (!fileExists(path)) {
			std::wofstream outfile(path);
			for (int i = 0; i < data.size(); i++) {
				outfile << data[i] << std::endl;
			}
			outfile.close();
		}
	}


	void createConfigsIfMissing() {
		createMissingConfig(badProcessesPath, badProcesses);
		createMissingConfig(badURLsPath, badURLs);
	}


	void loadConfig(const std::string &path, std::vector<std::wstring> &data) {
		data.clear();
		std::wstring line;
		std::wifstream file(path);
		if (file) {
			while (std::getline(file, line)) {
				if (line != L"")
					data.push_back(line);
			}
			file.close();
		}
	}


	time_t getFileEditTime(const std::string &path) {
		struct _stat result;
		if (_stat(path.data(), &result) == 0) {
			time_t modTime = result.st_mtime;
			return modTime;
		}
		return 0;
	}


	void refreshConfig() {
		static time_t badProcessesPreviousEditTime = 0;
		static time_t badURLsPreviousEditTime = 0;

		time_t badProcessesEditTime = getFileEditTime(badProcessesPath);
		if (badProcessesEditTime != badProcessesPreviousEditTime) {
			loadConfig(badProcessesPath, badProcesses);
			badProcessesPreviousEditTime = badProcessesEditTime;
		}

		time_t badURLsEditTime = getFileEditTime(badURLsPath);
		if (badURLsEditTime != badURLsPreviousEditTime) {
			loadConfig(badURLsPath, badURLs);
			badURLsPreviousEditTime = badURLsEditTime;
		}
	}

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


	void killURLs() {
		std::wstring tabName = getTabName();
		for (int i = 0; i < badURLs.size(); i++) {
			std::wregex regex(badURLs[i]);
			if (std::regex_search(tabName, regex)) {
				killProcessesByName(L"chrome.exe");
				ShowWindow(GetConsoleWindow(), SW_SHOW);
				std::wcout << L"Identified time wasters. Closing browser." << std::endl;
				std::wcout << L"Page: " << tabName << std::endl;
				std::wcout << L"Matched: " << badURLs[i] << std::endl;
				std::wcout << std::endl;
				Sleep(10 * 1000);
				ShowWindow(GetConsoleWindow(), SW_HIDE);
			}
 		}
	}


	[[noreturn]] void start() {
		std::cout << "Go Away Time Wasters!" << std::endl;

		CoInitialize(NULL);
		createConfigsIfMissing();
		HWND hwnd = GetConsoleWindow();
		HMENU hmenu = GetSystemMenu(hwnd, false);
		EnableMenuItem(hmenu, SC_CLOSE, MF_GRAYED);
		Sleep(3 * 1000);
		ShowWindow(hwnd, SW_HIDE);

		while (true) {
			time_t rawTime;
			time(&rawTime);

			tm timeInfo;
			localtime_s(&timeInfo, &rawTime);

			int wday = timeInfo.tm_wday;
			int tmhour = timeInfo.tm_hour;
			int tmsec = timeInfo.tm_sec;

			if (wday >= 1 && wday <= 5 && tmhour >= 1 && tmhour <= 16) {
				refreshConfig();
				killProcesses();
				killURLs();
			}
			else {
				refreshConfig();
				killProcesses();
				killURLs();
			}

			if (tmhour >= 1 && tmhour <= 6 && tmsec == 0) {
				HWND hwnd = FindWindow(L"Shell_TrayWnd", NULL);
				SendMessage(hwnd, WM_COMMAND, (WPARAM)419, 0);
				Sleep(1000);
				ShowWindow(GetConsoleWindow(), SW_SHOW);
				std::cout << "Time for bed!" << std::endl;
				Sleep(5 * 1000);
				ShowWindow(GetConsoleWindow(), SW_HIDE);
			}

			Sleep(250);
		}
	}
}

int main() {
	gatw::start();
    return 0;
}

