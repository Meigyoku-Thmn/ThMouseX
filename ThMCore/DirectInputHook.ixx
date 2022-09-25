module;

#include "framework.h"
#include "macro.h"
#include "Include/dinput.h"
#include <string>
#include <vector>

export module core.directinputhook;

import common.var;
import core.inputdeterminte;
import common.minhook;
import common.datatype;

constexpr auto GetDeviceStateIdx = 9;
constexpr auto ErrorMessageTitle = "DInput Hook Setup Error";

using namespace std;

HRESULT WINAPI GetDeviceStateDInput8(IDirectInputDevice8A* pDevice, DWORD cbData, LPVOID lpvData);
decltype(&GetDeviceStateDInput8) OriGetDeviceStateDInput8;

inline const char* GetDInputErrStr(const int errorCode) {
    if (errorCode == DIERR_NOTINITIALIZED)
        return "DIERR_NOTINITIALIZED";
    if (errorCode == DIERR_NOINTERFACE)
        return "DIERR_NOINTERFACE";
    if (errorCode == DIERR_DEVICENOTREG)
        return "DIERR_DEVICENOTREG";
    if (errorCode == DIERR_BETADIRECTINPUTVERSION)
        return "DIERR_BETADIRECTINPUTVERSION";
    if (errorCode == DIERR_INVALIDPARAM)
        return "DIERR_INVALIDPARAM";
    if (errorCode == DIERR_OLDDIRECTINPUTVERSION)
        return "DIERR_OLDDIRECTINPUTVERSION";
    if (errorCode == DIERR_OUTOFMEMORY)
        return "DIERR_OUTOFMEMORY";
    return "Unknown error.";
}

export DLLEXPORT bool PopulateDInputMethodRVAs() {
    bool                    result = false;
    DWORD*                  vtable{};
    HRESULT                 rs{};
    IDirectInput8A*         pDInput8{};
    IDirectInputDevice8A*   pDevice8{};
    DWORD                   baseAddress{};

    rs = DirectInput8Create(GetModuleHandleA(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8A, (PVOID*)&pDInput8, NULL);
    if (rs != DI_OK) {
        MessageBoxA(NULL, (string("Failed to create an IDirectInput8 instance:") + GetDInputErrStr(rs)).c_str(), ErrorMessageTitle, MB_OK | MB_ICONERROR);
        goto CleanAndReturn;
    }

    rs = pDInput8->CreateDevice(GUID_SysKeyboard, &pDevice8, NULL);
    if (rs != DI_OK) {
        MessageBoxA(NULL, (string("Failed to create an IDirectInputDevice8 instance:") + GetDInputErrStr(rs)).c_str(), ErrorMessageTitle, MB_OK | MB_ICONERROR);
        goto CleanAndReturn;
    }

    vtable = *(DWORD**)pDevice8;
    baseAddress = (DWORD)GetModuleHandleA("DInput8.dll");

    gs_dinput8_GetDeviceState_RVA = vtable[GetDeviceStateIdx] - baseAddress;

    result = true;
CleanAndReturn:
    pDevice8 && pDevice8->Release();
    pDInput8 && pDInput8->Release();
    return result;
}

export vector<MHookConfig> DInputHookConfig() {
    if (g_currentConfig.InputMethod != InputMethod::DirectInput)
        return {};
    auto baseAddress = (DWORD)GetModuleHandleA("DInput8.dll");
    return {
        {PVOID(baseAddress + gs_dinput8_GetDeviceState_RVA), &GetDeviceStateDInput8, (PVOID*)&OriGetDeviceStateDInput8},
    };
}

HRESULT WINAPI GetDeviceStateDInput8(IDirectInputDevice8A* pDevice, DWORD cbData, LPVOID lpvData) {
    auto hr = OriGetDeviceStateDInput8(pDevice, cbData, lpvData);
    if (SUCCEEDED(hr) && cbData == sizeof(BYTE) * 256) {
        auto keys = PBYTE(lpvData);
        auto gameInput = DetermineGameInput();
        if (gameInput & USE_BOMB)
            keys[DIK_X] |= 0x80;
        if (gameInput & USE_SPECIAL)
            keys[DIK_C] |= 0x80;
        if (gameInput & MOVE_LEFT)
            keys[DIK_LEFT] |= 0x80;
        if (gameInput & MOVE_RIGHT)
            keys[DIK_RIGHT] |= 0x80;
        if (gameInput & MOVE_UP)
            keys[DIK_UP] |= 0x80;
        if (gameInput & MOVE_DOWN)
            keys[DIK_DOWN] |= 0x80;
    }
    return hr;
}
