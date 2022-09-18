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
bool consoleLoaded = false;
#endif

#ifdef _DEBUG
void OpenConsole() {
    if (consoleLoaded == true)
        return;
    consoleLoaded = true;
    AllocConsole();

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
export DLLEXPORT int ConsoleLogAndFile(const char* _Format, ...) {
    if (consoleLoaded == false) {
        OpenConsole();
        consoleLoaded = true;
    }
    va_list args;
    va_start(args, _Format);
    while (true) {
        FILE* logFile;
        logFile = fopen("D:\\thmouse_log.txt", "a+");
        if (logFile == NULL)
            break;
        vfprintf(logFile, _Format, args);
        fclose(logFile);
        break;
    }
    auto rs = vprintf(_Format, args);
    va_end(args);
    return rs;
}
export DLLEXPORT int ConsoleLog(const char* _Format, ...) {
    if (consoleLoaded == false) {
        OpenConsole();
        consoleLoaded = true;
    }
    va_list args;
    va_start(args, _Format);
    auto rs = vprintf(_Format, args);
    va_end(args);
    return rs;
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