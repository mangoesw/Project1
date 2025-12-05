// https://learn.microsoft.com/en-us/windows/win32/procthread/creating-threads
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>

#define MAX_THREADS 2
#define BUF_SIZE 255
#define FSMOD pHotkey[i]->fsModifiers
#define VK pHotkey[i]->vk

DWORD WINAPI hotkeyThread(LPVOID lpParam);
void ErrorHandler(LPCTSTR lpszFunction);
void scprintf(HANDLE hStdout, LPCTSTR format, ...);


typedef struct Hotkey {
    int id;
    UINT fsModifiers;
    UINT vk;
} HOTKEY, * PHOTKEY;


int _tmain()
{
    PHOTKEY pHotkey[MAX_THREADS];
    DWORD   dwThreadIdArray[MAX_THREADS];
    HANDLE  hThreadArray[MAX_THREADS];

    // Create MAX_THREADS worker threads.

    for (int i = 0; i < MAX_THREADS; i++)
    {
        // Allocate memory for thread data.

        pHotkey[i] = (PHOTKEY)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
            sizeof(HOTKEY));

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
				VK = 0x42; // 'b' key
                break;
            case 1:
				FSMOD = MOD_ALT;
                VK = 0x43; // 'c' key
				break;
        }


        hThreadArray[i] = CreateThread(
            NULL,                   // default security attributes
            0,                      // use default stack size  
            hotkeyThread,       // thread function name
            pHotkey[i],          // argument to thread function 
            0,                      // use default creation flags 
            &dwThreadIdArray[i]);   // returns the thread identifier 


        if (hThreadArray[i] == NULL)
        {
            ErrorHandler(TEXT("CreateThread"));
            ExitProcess(3);
        }
    }

    // Wait until all threads have terminated.

    WaitForMultipleObjects(MAX_THREADS, hThreadArray, TRUE, INFINITE);

    // Close all thread handles and free memory allocations.

    for (int i = 0; i < MAX_THREADS; i++)
    {
        CloseHandle(hThreadArray[i]);
        if (pHotkey[i] != NULL)
        {
            HeapFree(GetProcessHeap(), 0, pHotkey[i]);
            pHotkey[i] = NULL;    // Ensure address is not reused.
        }
    }

    return 0;
}


DWORD WINAPI hotkeyThread(LPVOID lpParam)
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
        // _tprintf(_T("Hotkey 'ALT+b' registered, using MOD_NOREPEAT flag\n"));
    }

    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0) != 0)
    {
        if (msg.message == WM_HOTKEY)
        {
            scprintf(hStdout, TEXT("%d\n"), pHotkey->id);
            // _tprintf(_T("WM_HOTKEY received\n"));
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