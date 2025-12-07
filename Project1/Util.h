#pragma once
// #include <strsafe.h>
#include <windows.h>

constexpr size_t BUF_SIZE = 255;

void ErrorHandler(LPCTSTR lpszFunction);
void scprintf(HANDLE hStdout, LPCTSTR format, ...);