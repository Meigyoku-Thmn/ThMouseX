#pragma once
struct UINTSIZE {
    unsigned int width;
    unsigned int height;
};

#ifdef _DEBUG
void OpenConsole();
int ConsoleLog(const char* _Format, ...);
#endif

