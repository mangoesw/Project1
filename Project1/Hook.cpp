#include "Hook.h"
#include <windows.h>

LRESULT CALLBACK LowLevelKeyboardProc( // just testing
	int nCode,
	WPARAM wParam,
	LPARAM lParam
)
{
	if (nCode == HC_ACTION) {
		KBDLLHOOKSTRUCT* pKbDllHookStruct = (KBDLLHOOKSTRUCT*)lParam;
		// Example: Block the 'A' key
		if (pKbDllHookStruct->vkCode == 'A')
		{
			return 1;
		}

	}

	return CallNextHookEx(NULL, nCode, wParam, lParam);
}