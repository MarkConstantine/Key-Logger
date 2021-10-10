#pragma once
#include <windows.h>
#include <winhttp.h>
#include <iostream>

struct PUSH_PARAMS
{
    LPCWSTR lpszIpAddress;
    WORD wPort;
    DWORD dwPushPeriodMs;
};

DWORD WINAPI PushThread(LPVOID lpParam);
DWORD ReadLog(CHAR** buffer);
BOOL DeleteLog();
DWORD Push(LPCWSTR ip, WORD port, CHAR* buffer, DWORD dwBytesRead);