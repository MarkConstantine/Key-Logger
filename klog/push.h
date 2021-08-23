#pragma once
#include <windows.h>
#include <winhttp.h>
#include <iostream>

DWORD WINAPI PushThread(LPVOID lpParam);
DWORD Push(const std::wstring& ip, int port);