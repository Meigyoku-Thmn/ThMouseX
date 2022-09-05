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

HINSTANCE hinstance = NULL;
static char buffer[256];

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
            if (strcmp(buffer, "THMouseGUI.exe") == 0)
                return TRUE;

            // sprintf_s(dbBuff, sizeof(dbBuff), "attach to %s", buffer);
            // WriteToLog(dbBuff);
            for (int i = 0; i < gs_gameConfigArray.Length; i++) {
                // so sánh current process name với names trong data structure
                if (strcmp(buffer, gs_gameConfigArray.Configs[i].ProcessName) == 0) {
                    // TODO: - kiểm tra xem hook có thành công hay không
                    //        (không có API để hook thì tính là thất bại)
                    //       - sử dụng OutputDebugString để kiểm tra trạng thái (output của nó là Immediate Window)

                    // lấy gameConfig tương ứng
                    g_currentGameConfig = gs_gameConfigArray.Configs[i];
                    pixelRate = &g_currentGameConfig.PixelRate;
                    pixelOffset = (FloatPoint2*)&g_currentGameConfig.PixelOffset;
                    basePixelOffset = (FloatPoint2*)&g_currentGameConfig.BasePixelOffset;
                    baseResolutionX = g_currentGameConfig.BaseResolutionX;
                    offsetIsRelative = g_currentGameConfig.OffsetIsRelative;

                    if (offsetIsRelative == true) {
                        baseOfCode = ResolveBaseName(g_currentGameConfig.BaseName, _ref firstOffsetDirection);
                        if (baseOfCode == 0)
                            break;
                    }

                    // chuột trái có tác dụng bắn bomb
                    // biến này quyết định chuột trái ánh xạ đến nút nào (của DirectInput)
                    // biến này do loader cung cấp
                    // sử dụng trong WinmmHook
                    g_boomButton = gs_boomButton;
                    g_extraButton = gs_extraButton;

                    // sprintf_s(dbBuff, sizeof(dbBuff), "found %s. Let's hook it.\n", buffer);
                    // WriteToLog(dbBuff);

                    gs_textureFilePath2 = gs_textureFilePath;

                    // có thể lấy được windows handle từ directx object
                    // hook DirectX chủ yếu để hiển thị cursor đồ họa hỗ trợ di chuyển

                    // hook DirectX 9 
                    HookAPICalls(&D3DHook);

                    // hook DirectX 8 
                    HookAPICalls(&D3D8Hook);

                    // hook DirectInput8 (ánh xạ input chuột lên bàn phím)
                    HookAPICalls(&DInput8Hook);
                    HookAPICalls(&DInputHook);

                    // hook JoyStick (ánh xạ input chuột lên DirectInput)
                    HookAPICalls(&WinmmHook);

                    // hook Message Loop (đọc trạng thái của chuột)
                    HookAPICalls(&PeekMessageAHook);

                    // ẩn hiện chuột
                    HookAPICalls(&SetCursorHook);

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
            break;
    }
    return TRUE;
}

