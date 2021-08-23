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
DWORD Push(LPCWSTR ip, WORD port);