#include "framework.h"
#include "macro.h"
#include <shlwapi.h>

import common.minhook;
import common.var;
import common.datatype;
import common.helper;
import core.lowlevelinputhook;
import core.messagequeuehook;
import core.directinputhook;
import dx8.hook;
import core.directx9hook;
import core.var;

char buffer[MAX_PATH];
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
            GetModuleFileNameA(GetModuleHandleA(NULL), buffer, sizeof(buffer));
            PathStripPathA(buffer);

            // Of course we ignore the mother.
            if (strcmp(buffer, "THMouseGUI.exe") == 0)
                return TRUE;

            // We use SetWindowsHookEx, so DllMain is always called from a message loop in the target process.
            // Therefore, this is pretty thread-safe beside some edge cases (the PeekMessageHook).
            // The same thing happens for UnhookWindowsHookEx.
            for (int i = 0; i < gs_gameConfigArray.Length; i++) {
                if (_stricmp(buffer, gs_gameConfigArray.Configs[i].ProcessName) == 0) {
                    g_currentConfig = gs_gameConfigArray.Configs[i];

                    MHook_Initialize();

                    // hook DirectX 9 for crosshair cursor and collect window measurement
                    MHook_CreateHook(D3D9HookConfig());

                    // hook DirectX 8 for crosshair cursor and collect window measurement
                    MHook_CreateHook(D3D8HookConfig());

                    // hook DirectInput8 for input manipulation
                    MHook_CreateHook(DInputHookConfig());

                    // hook Joypad and GetKeyboardState for input manipulation
                    MHook_CreateHook(LowLevelInputHookConfig);

                    // hook Message Loop for additional runtime configuration
                    MHook_CreateHook(PeekMessageHookConfig);


                    MHook_EnableAll();
                    hookApplied = true;

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
            if (hookApplied) {
                //CleanUp();
                MHook_Uninitialize();
            }
            break;
    }
    return TRUE;
}

