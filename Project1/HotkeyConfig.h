#pragma once
#include <unordered_set>
#include <vector>
#include <windows.h>


class Hotkey {
	std::unordered_set<WORD> ShortcutKeys;
	std::vector<INPUT> KeyOutputs;

public:
	void AddKeyInput(WORD vk, bool keyUp)
	{
		INPUT input = {};
		input.type = INPUT_KEYBOARD;
		input.ki.wVk = vk;
		input.ki.dwFlags = keyUp ? KEYEVENTF_KEYUP : NULL;
		KeyOutputs.push_back(input);
	}

	void AddKeyPress(WORD vk)
	{
		AddKeyInput(vk, false);
		AddKeyInput(vk, true);
	}

	void AddUnicodeInput(WCHAR wch, bool keyUp)
	{
		INPUT input = {};
		input.type = INPUT_KEYBOARD;
		input.ki.wVk = 0;
		input.ki.wScan = wch;
		input.ki.dwFlags = KEYEVENTF_UNICODE | (keyUp ? KEYEVENTF_KEYUP : NULL);
		KeyOutputs.push_back(input);
	}

	void AddUnicodeChar(WCHAR wch)
	{
		AddUnicodeInput(wch, false);
		AddUnicodeInput(wch, true);
	}

	void AddUnicodeString(LPCWSTR str)
	{
		while (*str)
		{
			WCHAR wch = *str++;
			// Handle surrogate pairs for characters outside BMP (> U+FFFF)
			if (wch >= 0xD800 && wch <= 0xDBFF && *str >= 0xDC00 && *str <= 0xDFFF)
			{
				// High surrogate followed by low surrogate
				AddUnicodeInput(wch, false);
				AddUnicodeInput(*str, false);
				AddUnicodeInput(*str, true);
				AddUnicodeInput(wch, true);
				++str;
			}
			else
			{
				AddUnicodeChar(wch);
			}
		}
	}
};