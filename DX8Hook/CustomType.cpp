#include "CustomType.h"
#include "stdafx.h"

#ifdef _DEBUG
bool consoleLoaded = false;
void OpenConsole() {
    if (consoleLoaded == true) return;
    consoleLoaded = true;
    AllocConsole();

    freopen("conin$", "r", stdin);
    freopen("conout$", "w", stdout);
    freopen("conout$", "w", stderr);
    printf("Debugging Window:\n\n");

}

int ConsoleLog(const char* _Format, ...) {
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