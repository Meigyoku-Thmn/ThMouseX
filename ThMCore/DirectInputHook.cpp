#include "framework.h"
#include <dinput.h>
#include <string>
#include <vector>
#include <wrl/client.h>

#include "../Common/macro.h"
#include "../Common/DataTypes.h"
#include "../Common/Variables.h"
#include "../Common/MinHook.h"
#include "../Common/Log.h"
#include "InputDetermine.h"
#include "DirectInputHook.h"
#include "macro.h"

namespace minhook = common::minhook;
namespace note = common::log;

constexpr auto GetDeviceStateIdx = 9;
constexpr auto ErrorMessageTitle = "DInput Hook Setup Error";

using namespace std;
using namespace core::inputdetermine;
using namespace Microsoft::WRL;

#define TAG "[DirectInput] "

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

namespace core::directinputhook {
    HRESULT WINAPI GetDeviceStateDInput8(IDirectInputDevice8A* pDevice, DWORD cbData, LPVOID lpvData);
    decltype(&GetDeviceStateDInput8) OriGetDeviceStateDInput8;

    bool PopulateMethodRVAs() {
        ModuleHandle dinput8(LoadLibraryW(L"DInput8.dll"));
        if (!dinput8) {
            note::ToFile(TAG " Failed to load DInput8.dll.");
            return false;
        }
        auto _DirectInput8Create = (decltype(&DirectInput8Create))GetProcAddress(dinput8.get(), "DirectInput8Create");
        if (!_DirectInput8Create) {
            note::ToFile(TAG " Failed to import DInput8.dll|DirectInput8Create.");
            return false;
        }

        ComPtr<IDirectInput8A> pDInput8;
        auto rs = _DirectInput8Create(GetModuleHandleA(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8A, (PVOID*)&pDInput8, NULL);
        if (FAILED(rs)) {
            MessageBoxA(NULL, (string("Failed to create an IDirectInput8 instance:") + GetDInputErrStr(rs)).c_str(), ErrorMessageTitle, MB_OK | MB_ICONERROR);
            return false;
        }

        ComPtr<IDirectInputDevice8A> pDevice8;
        rs = pDInput8->CreateDevice(GUID_SysKeyboard, &pDevice8, NULL);
        if (FAILED(rs)) {
            MessageBoxA(NULL, (string("Failed to create an IDirectInputDevice8 instance:") + GetDInputErrStr(rs)).c_str(), ErrorMessageTitle, MB_OK | MB_ICONERROR);
            return false;
        }

        auto vtable = *(DWORD**)pDevice8.Get();
        auto baseAddress = (DWORD)dinput8.get();

        gs_dinput8_GetDeviceState_RVA = vtable[GetDeviceStateIdx] - baseAddress;

        return true;
    }

    vector<minhook::HookConfig> HookConfig() {
        if ((g_currentConfig.InputMethods & InputMethod::DirectInput) == InputMethod::None)
            return {};
        auto baseAddress = (DWORD)GetModuleHandleA("DInput8.dll");
        if (!baseAddress)
            return {};
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
}
