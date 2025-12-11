// the base: https://learn.microsoft.com/en-us/windows/win32/procthread/creating-threads
#include "Hook.h"
#include "HotkeyConfig.h"
#include "Util.h"
#include <strsafe.h>
#include <system_error>
//#include <tchar.h>
#include <thread>
#include <vector>
#include <windows.h>

constexpr auto THREADS = 1;

void HookThread();

int main()
{
	std::vector<std::thread> threads;
	threads.reserve(THREADS);

	try
	{
		for (int i = 0; i < THREADS; ++i)
		{
			threads.emplace_back(HookThread);
		}
	}
	catch (const std::system_error&)
	{
		ErrorHandler(TEXT("std::thread creation"));
		ExitProcess(3);
	}

	for (auto& t : threads)
	{
		if (t.joinable())
		{
			t.join();
		}
	}


	return 0;
}


void HookThread()
{
	HHOOK hHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
	if (hHook == NULL)
	{
		HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
		if (hStdout != INVALID_HANDLE_VALUE)
		{
			scprintf(hStdout, TEXT("SetWindowsHookEx failed: 0x%x\n"), HRESULT_FROM_WIN32(GetLastError()));
			ErrorHandler(TEXT("SetWindowsHookEx"));
		}
		else
		{ 
			ErrorHandler(TEXT("GetStdHandle"));
		}
		return;
	}

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0) != 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	
	UnhookWindowsHookEx(hHook);
}