// the base: https://learn.microsoft.com/en-us/windows/win32/procthread/creating-threads
#include "Hook.h"
#include "HotkeyConfig.h"
#include <new>
#include <strsafe.h>
#include <system_error>
#include <tchar.h>
#include <thread>
#include <vector>
#include <windows.h>

constexpr auto HOTKEY_THREADS = 10;
constexpr auto THREADS = HOTKEY_THREADS + 1;
constexpr auto BUF_SIZE = 255;

void HotkeyThread(PHOTKEY pHotkey);
void HookThread();
void ErrorHandler(LPCTSTR lpszFunction);
void scprintf(HANDLE hStdout, LPCTSTR format, ...);


int _tmain()
{
    PHOTKEY pHotkey[HOTKEY_THREADS] = {};
    std::vector<std::thread> threads;
    threads.reserve(THREADS);

    try
    {
        for (int i = 0; i < THREADS; ++i)
        {
            if (i == THREADS - 1)
            {
                threads.emplace_back(HookThread);
            }
            else
            {
                pHotkey[i] = new HOTKEY();

                ConfigureHotkey(pHotkey[i], i);

                threads.emplace_back(HotkeyThread, pHotkey[i]);
            }
        }
    }
    catch (const std::bad_alloc&)
    {
        ExitProcess(2);
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

    for (int i = 0; i < HOTKEY_THREADS; ++i)
    {
        if (pHotkey[i] != NULL)
        {
            delete pHotkey[i];
            pHotkey[i] = NULL;    // Ensure address is not reused.
        }
    }

    return 0;
}


void HotkeyThread(PHOTKEY pHotkey)
{
    HANDLE hStdout;

    hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hStdout == INVALID_HANDLE_VALUE)
        return;

    if (RegisterHotKey(
        NULL,
        pHotkey->id,
        pHotkey->fsModifiers,
        pHotkey->vk))
    {
        scprintf(hStdout, TEXT("id = %d\nfsModifiers = %d\nvk = %d\n\n"),
            pHotkey->id, pHotkey->fsModifiers, pHotkey->vk);
    }
    else
    {
        scprintf(hStdout, TEXT("RegisterHotKey failed for id %d: 0x%x\n"),
            pHotkey->id, HRESULT_FROM_WIN32(GetLastError()));
		ErrorHandler(TEXT("RegisterHotKey"));
        return;
    }
    
	// unpress modifier keys before sending inputs, and send key down after
    std::vector<WORD> modVks;
    if (pHotkey->fsModifiers & MOD_ALT) modVks.push_back(VK_MENU);
    if (pHotkey->fsModifiers & MOD_CONTROL) modVks.push_back(VK_CONTROL);
    if (pHotkey->fsModifiers & MOD_SHIFT) modVks.push_back(VK_SHIFT);
    if (pHotkey->fsModifiers & MOD_WIN) modVks.push_back(VK_LWIN);

    for (auto it = modVks.rbegin(); it != modVks.rend(); ++it)
    {
        INPUT input = {};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = *it;
        input.ki.dwFlags = KEYEVENTF_KEYUP;
        pHotkey->inputs.insert(pHotkey->inputs.begin(), input);
    }

    for (WORD vk : modVks)
    {
        AddKeyInput(pHotkey->inputs, vk, false);
    }

    INPUT* inputs = pHotkey->inputs.data();
    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0) != 0)
    {
        if (msg.message == WM_HOTKEY)
        {
            scprintf(hStdout, TEXT("%d\n"), pHotkey->id);

			UINT cInputs = (UINT)pHotkey->inputs.size();
            UINT uSent = SendInput(cInputs, inputs, sizeof(INPUT));
            if (uSent != cInputs)
            {
                scprintf(hStdout, TEXT("SendInput failed: 0x%x\n"), HRESULT_FROM_WIN32(GetLastError()));
				ErrorHandler(TEXT("SendInput"));
            }
        }
    }
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


void ErrorHandler(LPCTSTR lpszFunction)
{
    // Retrieve the system error message for the last-error code.

    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError();

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf,
        0, NULL);

    // Display the error message.

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
        (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
    StringCchPrintf((LPTSTR)lpDisplayBuf,
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s"),
        lpszFunction, dw, lpMsgBuf);
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

    // Free error-handling buffer allocations.

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
}

void scprintf(HANDLE hStdout, LPCTSTR format, ...)
{
    TCHAR msgBuf[BUF_SIZE];
    size_t cchStringSize;
    DWORD dwChars;
    va_list args;

    va_start(args, format);
    StringCchVPrintf(msgBuf, BUF_SIZE, format, args);
    va_end(args);

    StringCchLength(msgBuf, BUF_SIZE, &cchStringSize);
    WriteConsole(hStdout, msgBuf, (DWORD)cchStringSize, &dwChars, NULL);
}
