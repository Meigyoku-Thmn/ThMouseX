module;

#include "framework.h"
#include "macro.h"
#include <vector>
#include <Psapi.h>
#include <tlhelp32.h>

export module common.helper;

export DLLEXPORT void ReportLastError(const char *title) {
    auto dwErr = GetLastError();
    // lookup error code and display it
    LPVOID lpMsgBuf{};
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dwErr,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPTSTR)&lpMsgBuf,
        0,
        NULL
    );
    MessageBox(NULL, (LPCTSTR)lpMsgBuf, title, MB_OK | MB_ICONERROR);
    // Free the buffer.
    LocalFree(lpMsgBuf);
}
