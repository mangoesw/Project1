#include "Hook.h"
#include "HotkeyConfig.h"
#include "Util.h"
#include <windows.h>

namespace hStdout
{
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
}

LRESULT CALLBACK LowLevelKeyboardProc(
	int nCode,
	WPARAM wParam,
	LPARAM lParam
)
{
	if (nCode == HC_ACTION) {
		
	}

	return CallNextHookEx(NULL, nCode, wParam, lParam);
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