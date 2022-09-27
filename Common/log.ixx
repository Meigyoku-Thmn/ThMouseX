module;

#ifdef _DEBUG
#include "framework.h"
#include "macro.h"
#include <iostream>
#include <chrono>
#include <cstdio>
#endif

export module common.log;

using namespace std;

#ifdef _DEBUG
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
#endif

#ifdef _DEBUG
export DLLEXPORT void FileLog(const char* _Format, ...) {
    va_list args;
    va_start(args, _Format);
    FILE* logFile;
    logFile = fopen("D:\\thmousex_log.txt", "a+");
    if (logFile != NULL) {
        vfprintf(logFile, _Format, args);
        fclose(logFile);
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
#endif

#ifdef _DEBUG
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
#endif