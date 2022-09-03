// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

#include "apihijack.h"
#include "MyDirect3DCreate9.h"
#include "MyDirect3DCreate8.h"
#include "MyDirectInput8Create.h"
#include "MyMmsystem.h"
#include "MyPeekMessageA.h"
#include "global.h"

#include "MySetCursor.h"
#include "Helper.h"
#include "../DX8Hook/subGlobal.h"

#include "MyKeyboardState.h"

HINSTANCE hinstance = NULL;
static char buffer[256];

template <typename T>
inline MH_STATUS MH_CreateHookApiEx(LPCWSTR pszModule, LPCSTR pszProcName, LPVOID pDetour, T** ppOriginal) {
    return MH_CreateHookApi(pszModule, pszProcName, pDetour, reinterpret_cast<LPVOID*>(ppOriginal));
}


BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
) {
    char dbBuff[128];

    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
        {
            hinstance = hModule;
            // We don't need thread notifications for what we're doing.  Thus, get
            // rid of them, thereby eliminating some of the overhead of this DLL
            DisableThreadLibraryCalls(hModule);

            // Only hook the APIs if this is the process of the thxx.exe.
            GetModuleFileName(GetModuleHandle(NULL), buffer, sizeof(buffer));

            PathStripPath(buffer);
            // Nếu là cái GUI Loader thì không hook gì hết
            if (strcmp(buffer, "THMouseGUI.exe") == 0) return TRUE;

            //sprintf_s(dbBuff, sizeof(dbBuff), "attach to %s", buffer);
            //WriteToLog(dbBuff);
            for (int i = 0; i < gs_gameConfigArray.Length; i++) {
                // so sánh current process name với names trong data structure
                if (strcmp(buffer, gs_gameConfigArray.Configs[i].ProcessName) == 0) {
                    // TODO: - kiểm tra xem hook có thành công hay không
                    //        (không có API để hook thì tính là thất bại)
                    //       - sử dụng OutputDebugString để kiểm tra trạng thái (output của nó là Immediate Window)
                    //       - sử dụng thuật toán Bresenham để cải tiến di chuyển (tuyệt đối không dùng phép chia)

                    // lấy gameConfig tương ứng
                    g_currentGameConfig = gs_gameConfigArray.Configs[i];
                    pixelRate = &g_currentGameConfig.PixelRate;
                    pixelOffset = (FloatPoint2*)&g_currentGameConfig.PixelOffset;
                    basePixelOffset = (FloatPoint2*)&g_currentGameConfig.BasePixelOffset;
                    baseResolutionX = g_currentGameConfig.BaseResolutionX;
                    offsetIsRelative = g_currentGameConfig.OffsetIsRelative;

                    if (offsetIsRelative == true) {
                        baseOfCode = ResolveBaseName(g_currentGameConfig.BaseName, _ref firstOffsetDirection);
                        if (baseOfCode == 0) break;
                    }

                    // chuột trái có tác dụng bắn bomb
                    // biến này quyết định chuột trái ánh xạ đến nút nào (của DirectInput)
                    // biến này do loader cung cấp
                    // sử dụng trong WinmmHook
                    g_boomButton = gs_boomButton;
                    g_extraButton = gs_extraButton;

                    //sprintf_s(dbBuff, sizeof(dbBuff), "found %s. Let's hook it.\n", buffer);
                    //WriteToLog(dbBuff);

                    gs_textureFilePath2 = gs_textureFilePath;

                    if (MH_Initialize() != MH_OK) {
                        break;
                    }

                    // có thể lấy được windows handle từ directx object
                    // hook DirectX chủ yếu để hiển thị cursor đồ họa hỗ trợ di chuyển

                    // hook DirectX 9 
                    // HookAPICalls(&D3DHook);
                    MH_CreateHookApiEx(L"d3d9.dll", "Direct3DCreate9",
                        &MyDirect3DCreate9, &D3DHook.Functions[0].OrigFn);

                    // hook DirectX 8 
                    // HookAPICalls(&D3D8Hook);
                    MH_CreateHookApiEx(L"d3d8.dll", "Direct3DCreate8", &MyDirect3DCreate8, &D3D8Hook.Functions[0].OrigFn);

                    // hook DirectInput8 (ánh xạ input chuột lên bàn phím)
                    // HookAPICalls(&DInput8Hook);
                    MH_CreateHookApiEx(L"DINPUT8.dll", "DirectInput8Create", &MyDirectInput8Create, &DInput8Hook.Functions[0].OrigFn);
                    MH_CreateHookApiEx(L"DINPUT.dll", "DirectInputCreateW", &MyDirectInputCreateW, &DInputHook.Functions[0].OrigFn);

                    // hook JoyStick (ánh xạ input chuột lên DirectInput)
                    // HookAPICalls(&WinmmHook);
                    MH_CreateHookApiEx(L"WINMM.dll", "joyGetDevCapsA", &MyJoyGetDevCapsA, (void**)0);
                    MH_CreateHookApiEx(L"WINMM.dll", "joyGetPosEx", &MyJoyGetPosEx, (void**)0);
                    MH_CreateHookApiEx(L"WINMM.dll", "joyGetPos", &MyJoyGetPos, &WinmmHook.Functions[2].OrigFn);

                    // hook Message Loop (đọc trạng thái của chuột)
                    //HookAPICalls(&PeekMessageAHook);
                    MH_CreateHookApiEx(L"USER32.dll", "PeekMessageA", &MyPeekMessageA, &PeekMessageAHook.Functions[0].OrigFn);
                    MH_CreateHookApiEx(L"USER32.dll", "PeekMessageW", &MyPeekMessageW, &PeekMessageAHook.Functions[1].OrigFn);

                    HookAPICalls(&SetCursorHook);

                    MH_CreateHookApiEx(L"USER32.dll", "GetKeyboardState", &MyGetKeyboardState, &getKeyboardState);

                    MH_EnableHook(MH_ALL_HOOKS);
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
            MH_Uninitialize();
            break;
    }
    return TRUE;
}

