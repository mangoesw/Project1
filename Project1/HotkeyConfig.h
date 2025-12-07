#pragma once
#include <vector>
#include <windows.h>

typedef struct Hotkey {
    int id;
    UINT fsModifiers;
    UINT vk;
    std::vector<INPUT> inputs;
} HOTKEY, * PHOTKEY;

void AddKeyInput(std::vector<INPUT>& inputs, WORD vk, bool keyUp = false);
void AddKeyPress(std::vector<INPUT>& inputs, WORD vk);
void AddUnicodeInput(std::vector<INPUT>& inputs, WCHAR wch, bool keyUp = false);
void AddUnicodeChar(std::vector<INPUT>& inputs, WCHAR wch);
void AddUnicodeString(std::vector<INPUT>& inputs, LPCWSTR str);

void ConfigureHotkey(PHOTKEY pHotkey, int index);
