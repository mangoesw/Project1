#include "HotkeyConfig.h"
#include <vector>
#include <windows.h>

void AddKeyInput(std::vector<INPUT>& inputs, WORD vk, bool keyUp)
{
    INPUT input = {};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = vk;
    input.ki.dwFlags = keyUp ? KEYEVENTF_KEYUP : 0;
    inputs.push_back(input);
}

void AddKeyPress(std::vector<INPUT>& inputs, WORD vk)
{
    AddKeyInput(inputs, vk, false);
    AddKeyInput(inputs, vk, true);
}

void AddUnicodeInput(std::vector<INPUT>& inputs, WCHAR wch, bool keyUp)
{
    INPUT input = {};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = 0;
    input.ki.wScan = wch;
    input.ki.dwFlags = KEYEVENTF_UNICODE | (keyUp ? KEYEVENTF_KEYUP : 0);
    inputs.push_back(input);
}

void AddUnicodeChar(std::vector<INPUT>& inputs, WCHAR wch)
{
    AddUnicodeInput(inputs, wch, false);
    AddUnicodeInput(inputs, wch, true);
}

void AddUnicodeString(std::vector<INPUT>& inputs, LPCWSTR str)
{
    while (*str)
    {
        WCHAR wch = *str++;
        // Handle surrogate pairs for characters outside BMP (> U+FFFF)
        if (wch >= 0xD800 && wch <= 0xDBFF && *str >= 0xDC00 && *str <= 0xDFFF)
        {
            // High surrogate followed by low surrogate
            AddUnicodeInput(inputs, wch, false);
            AddUnicodeInput(inputs, *str, false);
            AddUnicodeInput(inputs, *str, true);
            AddUnicodeInput(inputs, wch, true);
            str++;
        }
        else
        {
            AddUnicodeChar(inputs, wch);
        }
    }
}

void ConfigureHotkey(PHOTKEY pHotkey, int index)
{
    pHotkey->id = index;
    switch (index) {
        case 0:
            pHotkey->fsModifiers = MOD_ALT;
            pHotkey->vk = VK_OEM_6;
            AddUnicodeString(pHotkey->inputs, L"=");
            break;
        case 1:
            pHotkey->fsModifiers = MOD_ALT | MOD_SHIFT;
            pHotkey->vk = VK_OEM_6;
            AddUnicodeString(pHotkey->inputs, L"+");
            break;
        case 2:
            pHotkey->fsModifiers = MOD_ALT;
            pHotkey->vk = VK_BACK;
            AddUnicodeString(pHotkey->inputs, L"\\");
            break;
        case 3:
            pHotkey->fsModifiers = MOD_ALT | MOD_SHIFT;
            pHotkey->vk = VK_BACK;
            AddUnicodeString(pHotkey->inputs, L"|");
            break;
        case 4:
            pHotkey->fsModifiers = MOD_ALT;
            pHotkey->vk = VK_HOME;
            AddKeyPress(pHotkey->inputs, VK_DELETE);
            break;
        case 5:
            pHotkey->fsModifiers = MOD_ALT;
            pHotkey->vk = 'H';
            AddKeyPress(pHotkey->inputs, VK_LEFT);
            break;
        case 6:
            pHotkey->fsModifiers = MOD_ALT;
            pHotkey->vk = 'J';
            AddKeyPress(pHotkey->inputs, VK_DOWN);
            break;
        case 7:
            pHotkey->fsModifiers = MOD_ALT;
            pHotkey->vk = 'K';
            AddKeyPress(pHotkey->inputs, VK_UP);
            break;
        case 8:
            pHotkey->fsModifiers = MOD_ALT;
            pHotkey->vk = 'L';
            AddKeyPress(pHotkey->inputs, VK_RIGHT);
            break;
        case 9:
            pHotkey->fsModifiers = MOD_ALT;
            pHotkey->vk = '9';
            AddKeyPress(pHotkey->inputs, VK_VOLUME_UP);
            break;
    }
}
