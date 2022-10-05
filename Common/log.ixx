module;

#ifdef _DEBUG
#include "framework.h"
#include "macro.h"
#include <iostream>
#include <chrono>
#include <cstdio>
#include <string>
#include <codecvt>
#endif

export module common.log;

import common.var;

using namespace std;

void OpenConsole() {
    if (AllocConsole() == FALSE)
        return;

#pragma warning(push)
#pragma warning(disable: 6031)
    freopen("conin$", "r", stdin);
    freopen("conout$", "w", stdout);
    freopen("conout$", "w", stderr);
#pragma warning(pop)

    printf("Debugging Window:\n\n");
}

FILE* logFile;
string logPath;
export DLLEXPORT void FileLog(const char* _Format, ...) {
    va_list args;
    va_start(args, _Format);
    if (logFile == NULL) {
        if (logPath.size() == 0)
            logPath = wstring_convert<codecvt_utf8_utf16<wchar_t>>().to_bytes(wstring(g_currentModuleDirPath) + L"/log.txt");
        logFile = fopen(logPath.c_str(), "a+");
        if (logFile != NULL)
            setvbuf(logFile, NULL, _IONBF, 0);
    }
    if (logFile != NULL) {
        vfprintf(logFile, _Format, args);
    }
    va_end(args);
}

export DLLEXPORT void ConsoleLog(const char* _Format, ...) {
    OpenConsole();
    va_list args;
    va_start(args, _Format);
    vprintf(_Format, args);
    va_end(args);
}

export DLLEXPORT void PrintFPS() {
    using namespace chrono;
    static time_point<steady_clock> oldTime = high_resolution_clock::now();
    static int fps; fps++;

    if (duration_cast<seconds>(high_resolution_clock::now() - oldTime) >= seconds{1}) {
        oldTime = high_resolution_clock::now();
        ConsoleLog("FPS: %d\n", fps);
        fps = 0;
    }
}