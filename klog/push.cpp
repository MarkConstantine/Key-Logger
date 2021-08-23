#include "push.h"

DWORD WINAPI PushThread(LPVOID lpParam)
{
    DWORD dwSleepMs = (DWORD)lpParam;
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
        (LPVOID)body.c_str(), // lpOptional
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