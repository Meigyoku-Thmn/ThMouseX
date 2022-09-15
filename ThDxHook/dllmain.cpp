#include "framework.h"
#include "macro.h"
#include <shlwapi.h>

import core.apihijack;
import common.minhook;
import common.var;
import common.datatype;
import common.helper;
import core.lowlevelinputhook;
import core.peekmesssagehook;
import core.setcursorhook;
import core.directinputhook;
import core.windowshook;
import dx8.hook;
import core.directx9hook;

static char buffer[256];

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH: {
            hinstance = hModule;
            // We don't need thread notifications for what we're doing.
            // Thus, get rid of them, thereby eliminating some of the overhead of this DLL.
            DisableThreadLibraryCalls(hModule);

            // Only hook the APIs if we have a configuation of the process.
            // If the process is not what we have anything to do with, just return TRUE, no need to eagerly unload.
            // The DLL will be forcefully unloaded from all processes when ThMouseGUI closes.
            GetModuleFileName(GetModuleHandle(NULL), buffer, sizeof(buffer));
            PathStripPath(buffer);

            // Of course we ignore the mother.
            if (strcmp(buffer, "THMouseGUI.exe") == 0)
                return TRUE;

            for (int i = 0; i < gs_gameConfigArray.Length; i++) {
                if (_stricmp(buffer, gs_gameConfigArray.Configs[i].ProcessName) == 0) {
                    g_currentConfig = gs_gameConfigArray.Configs[i];

                    MHook_Initialize();

                    // hook DirectX 9 for crosshair
                    MHook_CreateHook(D3D9HookConfig());
                    // hook DirectX 8 for crosshair
                    MHook_CreateHook(D3D8HookConfig());

                    // hook DirectInput8 for input manipulation
                    MHook_CreateHook(D3InputHookConfig());
                    // hook Joypad for input manipulation
                    HookAPICalls(&WinmmHook);
                    // hook GetKeyboardState for input manipulation
                    HookAPICalls(&User32Hook);

                    // hook Message Loop for additional runtime configuration
                    HookAPICalls(&PeekMessageAHook);
                    // hook some cursor API for cursor visibility toggling
                    HookAPICalls(&SetCursorHook);

                    MHook_EnableAll();

                    break;
                }
            }
            break;
        }
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            // TODO: implement a clean-up step here
            break;
    }
    return TRUE;
}

