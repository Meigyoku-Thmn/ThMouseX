module;

#include "framework.h"
#include "macro.h"
#include <iostream>

export module common.log;

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

export DLLEXPORT int ConsoleLog(const char* _Format, ...) {
#ifdef _DEBUG
    if (consoleLoaded == false) {
        OpenConsole();
        consoleLoaded = true;
    }
    va_list args;
    va_start(args, _Format);
    auto rs = vprintf(_Format, args);
    va_end(args);
    return rs;
#else
    return 0;
#endif
}