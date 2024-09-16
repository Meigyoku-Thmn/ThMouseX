#include <Windows.h>
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
namespace inputdetermine = core::inputdetermine;

constexpr auto GetDeviceStateIdx = 9;

using namespace std;
using namespace Microsoft::WRL;

#define TAG "[DirectInput] "

struct InputRuleItemForDInput {
    PBYTE       vkCodeSrcPtr;
    BYTE        dikCodeDest;
    GameInput   input;
};

namespace core::directinput {
    using enum GameInput;

    static HRESULT WINAPI GetDeviceStateDInput8(IDirectInputDevice8A* pDevice, DWORD cbData, LPVOID lpvData);
    static decltype(&GetDeviceStateDInput8) OriGetDeviceStateDInput8;

    static InputRuleItemForDInput inputRule[]{
        { &gs_vkCodeForLeftClick,       DIK_X,      CLICK_LEFT      },
        { &gs_vkCodeForMiddleClick,     DIK_C,      CLICK_MIDDLE    },
        { &gs_vkCodeForRightClick,      0,          CLICK_RIGHT     },
        { &gs_vkCodeForXButton1Click,   0,          CLICK_XBUTTON1  },
        { &gs_vkCodeForXButton2Click,   0,          CLICK_XBUTTON2  },
        { &gs_vkCodeForScrollUp,        0,          SCROLL_UP       },
        { &gs_vkCodeForScrollDown,      0,          SCROLL_DOWN     },
        { &gs_vkCodeForScrollLeft,      0,          SCROLL_LEFT     },
        { &gs_vkCodeForScrollRight,     0,          SCROLL_RIGHT    },
        { nil,                          DIK_LEFT,   MOVE_LEFT       },
        { nil,                          DIK_RIGHT,  MOVE_RIGHT      },
        { nil,                          DIK_UP,     MOVE_UP         },
        { nil,                          DIK_DOWN,   MOVE_DOWN       },
    };

    void Initialize() {
        static bool initialized = false;
        static mutex mtx;
        HMODULE dinput8{};
        {
            const scoped_lock lock(mtx);
            if (initialized)
                return;
            if ((g_currentConfig.InputMethods & InputMethod_DirectInput) == InputMethod_None)
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
                for (auto& ruleItem : inputRule) {
                    auto vkCodeSrc = ruleItem.vkCodeSrcPtr == nil ? (BYTE)0 : *ruleItem.vkCodeSrcPtr;
                    ruleItem.dikCodeDest = helper::MapVk2Dik(vkCodeSrc, mappingTable, ruleItem.dikCodeDest);
                }
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
            { PVOID(vtable[GetDeviceStateIdx]), & GetDeviceStateDInput8, & OriGetDeviceStateDInput8 },
        });
    }

    static HRESULT WINAPI GetDeviceStateDInput8(IDirectInputDevice8A* pDevice, DWORD cbData, LPVOID lpvData) {
        using enum GameInput;
        auto hr = OriGetDeviceStateDInput8(pDevice, cbData, lpvData);
        if (SUCCEEDED(hr) && cbData == sizeof(BYTE) * 256) {
            auto keys = PBYTE(lpvData);
            auto gameInput = inputdetermine::DetermineGameInput();
            for (const auto& ruleItem : inputRule) {
                if ((gameInput & ruleItem.input) == ruleItem.input)
                    keys[ruleItem.dikCodeDest] |= 0x80;
            }
        }
        return hr;
    }
}
