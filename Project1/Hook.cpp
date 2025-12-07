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
		KBDLLHOOKSTRUCT* pKbDllHookStruct = (KBDLLHOOKSTRUCT*)lParam;
		
		if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)
		{
			if (ShouldBlockKey(pKbDllHookStruct->vkCode))
			{
				scprintf(hStdout::hStdout, TEXT("hook blocked key: 0x%x\n"), pKbDllHookStruct->vkCode);
				return 1;
			}
		}
	}

	return CallNextHookEx(NULL, nCode, wParam, lParam);
}