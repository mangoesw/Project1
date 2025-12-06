// https://learn.microsoft.com/en-us/windows/win32/procthread/creating-threads
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <vector>

constexpr auto MAX_THREADS = 10;
constexpr auto BUF_SIZE = 255;
#define FSMOD pHotkey[i]->fsModifiers
#define VK pHotkey[i]->vk
#define INPUTS pHotkey[i]->inputs

DWORD WINAPI HotkeyThread(LPVOID lpParam);
void ErrorHandler(LPCTSTR lpszFunction);
void scprintf(HANDLE hStdout, LPCTSTR format, ...);


typedef struct Hotkey {
    int id;
    UINT fsModifiers;
    UINT vk;
	std::vector<INPUT> inputs;
} HOTKEY, * PHOTKEY;


void AddKeyInput(std::vector<INPUT>& inputs, WORD vk, bool keyUp = false)
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

void AddUnicodeInput(std::vector<INPUT>& inputs, WCHAR wch, bool keyUp = false)
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


int _tmain()
{
    PHOTKEY pHotkey[MAX_THREADS];
    DWORD   dwThreadIdArray[MAX_THREADS];
    HANDLE  hThreadArray[MAX_THREADS];

    for (int i = 0; i < MAX_THREADS; i++)
    {
        pHotkey[i] = new HOTKEY();

        if (pHotkey[i] == NULL)
        {
            // If the array allocation fails, the system is out of memory
            // so there is no point in trying to print an error message.
            // Just terminate execution.
            ExitProcess(2);
        }

		pHotkey[i]->id = i;
        switch (i) {
            case 0:
                FSMOD = MOD_ALT;
				VK = VK_OEM_6;
				AddUnicodeString(INPUTS, L"=");
                break;
            case 1:
                FSMOD = MOD_ALT | MOD_SHIFT;
				VK = VK_OEM_4;
                AddUnicodeString(INPUTS, L"+");
				break;
            case 2:
                FSMOD = MOD_ALT;
                VK = VK_BACK;
				AddUnicodeString(INPUTS, L"\\");
                break;
            case 3:
				FSMOD = MOD_ALT | MOD_SHIFT;
				VK = VK_BACK;
                AddUnicodeString(INPUTS, L"|");
				break;
            case 4:
                FSMOD = MOD_ALT;
                VK = VK_SNAPSHOT;
				AddKeyPress(INPUTS, VK_DELETE);
                break;
            case 5:
                FSMOD = MOD_ALT;
                VK = 'H';
				AddKeyPress(INPUTS, VK_LEFT);
                break;
            case 6:
                FSMOD = MOD_ALT;
                VK = 'J';
                AddKeyPress(INPUTS, VK_DOWN);
                break;
            case 7:
                FSMOD = MOD_ALT;
                VK = 'K';
                AddKeyPress(INPUTS, VK_UP);
                break;
            case 8:
                FSMOD = MOD_ALT;
                VK = 'L';
                AddKeyPress(INPUTS, VK_RIGHT);
                break;
            case 9:
                FSMOD = MOD_ALT;
                VK = '9';
                AddKeyPress(INPUTS, VK_VOLUME_UP);
        }


        hThreadArray[i] = CreateThread(
            NULL,                   // default security attributes
            0,                      // use default stack size  
            HotkeyThread,       // thread function name
            pHotkey[i],          // argument to thread function 
            0,                      // use default creation flags 
            &dwThreadIdArray[i]);   // returns the thread identifier 


        if (hThreadArray[i] == NULL)
        {
            ErrorHandler(TEXT("CreateThread"));
            ExitProcess(3);
        }
    }

    WaitForMultipleObjects(MAX_THREADS, hThreadArray, TRUE, INFINITE);

    for (int i = 0; i < MAX_THREADS; i++)
    {
        CloseHandle(hThreadArray[i]);
        if (pHotkey[i] != NULL)
        {
            delete pHotkey[i];
            pHotkey[i] = NULL;    // Ensure address is not reused.
        }
    }

    return 0;
}


DWORD WINAPI HotkeyThread(LPVOID lpParam)
{
    HANDLE hStdout;
	PHOTKEY pHotkey = (PHOTKEY)lpParam;

    hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hStdout == INVALID_HANDLE_VALUE)
        return 1;

    if (RegisterHotKey(
        NULL,
        pHotkey->id,
        pHotkey->fsModifiers,
        pHotkey->vk))
    {
        scprintf(hStdout, TEXT("id = %d\nfsModifiers = %d\nvk = %d\n\n"),
            pHotkey->id, pHotkey->fsModifiers, pHotkey->vk);
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
            }
        }
    }

    return 0;
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
