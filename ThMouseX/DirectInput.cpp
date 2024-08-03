#include "framework.h"
#include <dinput.h>
#include <string>
#include <vector>
#include <wrl/client.h>
#include <mutex>

#include "../Common/macro.h"
#include "../Common/DataTypes.h"
#include "../Common/Variables.h"
#include "../Common/MinHook.h"
#include "../Common/Log.h"
#include "../Common/Helper.h"
#include "InputDetermine.h"
#include "DirectInput.h"

namespace minhook = common::minhook;
namespace note = common::log;
namespace helper = common::helper;

constexpr auto GetDeviceStateIdx = 9;

using namespace std;
using namespace core::inputdetermine;
using namespace Microsoft::WRL;

#define TAG "[DirectInput] "

namespace core::directinput {
    HRESULT WINAPI GetDeviceStateDInput8(IDirectInputDevice8A* pDevice, DWORD cbData, LPVOID lpvData);
    decltype(&GetDeviceStateDInput8) OriGetDeviceStateDInput8;
    BYTE bombDikCode = DIK_X;
    BYTE specialDikCode = DIK_C;

    void Initialize() {
        using enum InputMethod;
        static bool initialized = false;
        static mutex mtx;
        HMODULE dinput8{};
        {
            const lock_guard lock(mtx);
            if (initialized)
                return;
            if ((g_currentConfig.InputMethods & DirectInput) == None)
                return;

            dinput8 = GetModuleHandleW((g_systemDirPath + wstring(L"\\DInput8.dll")).c_str());
            if (!dinput8)
                return;

            do {
                auto utilityPath = g_currentModuleDirPath + wstring(L"\\Utility.exe");
                auto [exitCode, message] = helper::CallProcess(utilityPath, L"GetDirectInputMappingTableId");
                if (exitCode == -1) {
                    if (message.size() > 0)
                        note::LastErrorToFile(message.c_str());
                    break;
                }
                auto resHandle = FindResourceW(dinput8, MAKEINTRESOURCEW(exitCode), (LPWSTR)RT_RCDATA);
                if (resHandle == nil) {
                    note::LastErrorToFile(TAG "Failed to get the mapping table from DInput8.dll");
                    break;
                }
                auto mappingTable = (BYTE*)LoadResource(dinput8, resHandle);
                if (mappingTable == nil) {
                    note::LastErrorToFile(TAG "Failed to get the mapping table from DInput8.dll");
                    break;
                }
                bombDikCode = helper::MapVk2Dik(gs_bombButton, mappingTable);
                specialDikCode = helper::MapVk2Dik(gs_extraButton, mappingTable);
            } while (false);

            initialized = true;
        }

        auto _DirectInput8Create = (decltype(&DirectInput8Create))GetProcAddress(dinput8, "DirectInput8Create");
        if (!_DirectInput8Create) {
            note::LastErrorToFile(TAG "Failed to import DInput8.dll|DirectInput8Create");
            return;
        }

        ComPtr<IDirectInput8A> pDInput8;
        auto rs = _DirectInput8Create(GetModuleHandleA(nil), DIRECTINPUT_VERSION, IID_IDirectInput8A, &pDInput8, nil);
        if (FAILED(rs)) {
            note::DxErrToFile(TAG "Failed to create an IDirectInput8 instance", rs);
            return;
        }

        ComPtr<IDirectInputDevice8A> pDevice8;
        rs = pDInput8->CreateDevice(GUID_SysKeyboard, &pDevice8, nil);
        if (FAILED(rs)) {
            note::DxErrToFile(TAG "Failed to create an IDirectInputDevice8 instance", rs);
            return;
        }

        auto vtable = *(DWORD**)pDevice8.Get();

        minhook::CreateHook(vector<minhook::HookConfig>{
            { PVOID(vtable[GetDeviceStateIdx]), &GetDeviceStateDInput8, &OriGetDeviceStateDInput8 },
        });
    }

    HRESULT WINAPI GetDeviceStateDInput8(IDirectInputDevice8A* pDevice, DWORD cbData, LPVOID lpvData) {
        using enum GameInput;
        auto hr = OriGetDeviceStateDInput8(pDevice, cbData, lpvData);
        if (SUCCEEDED(hr) && cbData == sizeof(BYTE) * 256) {
            auto keys = PBYTE(lpvData);
            auto gameInput = DetermineGameInput();
            if ((gameInput & USE_BOMB) == USE_BOMB)
                keys[bombDikCode] |= 0x80;
            if ((gameInput & USE_SPECIAL) == USE_SPECIAL)
                keys[specialDikCode] |= 0x80;
            if ((gameInput & MOVE_LEFT) == MOVE_LEFT)
                keys[DIK_LEFT] |= 0x80;
            if ((gameInput & MOVE_RIGHT) == MOVE_RIGHT)
                keys[DIK_RIGHT] |= 0x80;
            if ((gameInput & MOVE_UP) == MOVE_UP)
                keys[DIK_UP] |= 0x80;
            if ((gameInput & MOVE_DOWN) == MOVE_DOWN)
                keys[DIK_DOWN] |= 0x80;
        }
        return hr;
    }
}
