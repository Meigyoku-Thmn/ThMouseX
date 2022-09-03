// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the THDXHOOK_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// THDXHOOK_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef THDXHOOK_EXPORTS
#define THDXHOOK_API __declspec(dllexport)
#else
#define THDXHOOK_API __declspec(dllimport)
#endif

#include "GameConfigArray.h"

THDXHOOK_API int installThDxHook(
    const GameConfigArray* config,
    int leftButton,
    int midButton,
    const char* pTextureFilePath
);

THDXHOOK_API int removeThDxHook(void);