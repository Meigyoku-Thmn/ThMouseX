#include "../Common/macro.h"
#include "../Common/DataTypes.h"
#include <functional>
#include <Windows.h>

#include "../Common/Log.h"
#include "Intercom.h"

using namespace std;

namespace note = common::log;

#define TAG "[Intercom] "
#define ValidateResult(result) \
if (result == 0) { \
    note::LastErrorToFile(TAG "Failed to get game config from the server window"); \
    return false; \
}0
#define ValidateServerResult(serverResult) \
if (serverResult == 0) { \
    return false; \
}0

static const auto ServerWindowName = L"{BC9D7991-41A0-47F3-BC4D-6CFF2BA205FD}";
static const auto ClientClassName = L"{E4CB816C-3BF1-41F5-BAC5-4D4B847846A7}";

constexpr auto GET_CONFIG_EVENT = 1;
constexpr auto GET_CONFIG_EVENT_RETURN = 2;
constexpr auto GET_MEM_BLOCK_EVENT_RETURN = 3;

constexpr auto GET_MEM_BLOCK_MSG = WM_APP + 1;

constexpr auto Timeout = 1000 * 60 * 10;

static HWND CreateResultWindow();
static void DestroyResultWindow(HWND hwnd);

static function<bool(const COPYDATASTRUCT*)> copyDataCallback;

namespace core::intercom {
    bool QueryGameConfig(LPCWSTR processName, CommonConfig& commonConfig, GameConfig& gameConfig) {
        auto serverHwnd = FindWindowW(nil, ServerWindowName);
        if (serverHwnd == nil) {
            note::ToFile(TAG "Cannot find the server window.");
            return false;
        }

        auto clientHwnd = CreateResultWindow();
        if (clientHwnd == nil)
            return false;
        defer({ DestroyResultWindow(clientHwnd); });

#pragma region Get Common Config and Game Config
        COPYDATASTRUCT data{
            .dwData = GET_CONFIG_EVENT,
            .cbData = scast<DWORD>((wcslen(processName) + 1) * sizeof(processName[0])),
            .lpData = bcast<PVOID>(processName),
        };
        copyDataCallback = [&](const COPYDATASTRUCT* received) {
            if (received->dwData != GET_CONFIG_EVENT_RETURN)
                return false;
            commonConfig = *bcast<CommonConfig*>(received->lpData);
            gameConfig = *bcast<GameConfig*>(bcast<uintptr_t>(received->lpData) + sizeof(CommonConfig));
            return true;
        };
        DWORD_PTR serverResult;
        auto result = SendMessageTimeoutW(serverHwnd, WM_COPYDATA, bcast<WPARAM>(clientHwnd),
            bcast<LPARAM>(&data), SMTO_NOTIMEOUTIFNOTHUNG | SMTO_ERRORONEXIT, Timeout, &serverResult);
        ValidateResult(result);
        ValidateServerResult(serverResult);
#pragma endregion

#pragma region Get TextureFilePath
        copyDataCallback = [&](const COPYDATASTRUCT* received) {
            if (received->dwData != GET_MEM_BLOCK_EVENT_RETURN)
                return false;
            commonConfig.TextureFilePath = new WCHAR[received->cbData / sizeof(commonConfig.TextureFilePath[0])];
            memcpy(commonConfig.TextureFilePath, received->lpData, received->cbData);
            return true;
        };
        result = SendMessageTimeoutW(serverHwnd, GET_MEM_BLOCK_MSG, bcast<WPARAM>(clientHwnd),
            bcast<LPARAM>(commonConfig.TextureFilePath), SMTO_NOTIMEOUTIFNOTHUNG | SMTO_ERRORONEXIT, Timeout, &serverResult);
        ValidateResult(result);
        ValidateServerResult(serverResult);
#pragma endregion

#pragma region Get ImGuiFontPath
        copyDataCallback = [&](const COPYDATASTRUCT* received) {
            if (received->dwData != GET_MEM_BLOCK_EVENT_RETURN)
                return false;
            commonConfig.ImGuiFontPath = new WCHAR[received->cbData / sizeof(commonConfig.ImGuiFontPath[0])];
            memcpy(commonConfig.ImGuiFontPath, received->lpData, received->cbData);
            return true;
        };
        result = SendMessageTimeoutW(serverHwnd, GET_MEM_BLOCK_MSG, bcast<WPARAM>(clientHwnd),
            bcast<LPARAM>(commonConfig.ImGuiFontPath), SMTO_NOTIMEOUTIFNOTHUNG | SMTO_ERRORONEXIT, Timeout, &serverResult);
        ValidateResult(result);
#pragma endregion

#pragma region Get ProcessName
        copyDataCallback = [&](const COPYDATASTRUCT* received) {
            if (received->dwData != GET_MEM_BLOCK_EVENT_RETURN)
                return false;
            gameConfig.ProcessName = new WCHAR[received->cbData / sizeof(gameConfig.ProcessName[0])];
            memcpy(gameConfig.ProcessName, received->lpData, received->cbData);
            return true;
        };
        result = SendMessageTimeoutW(serverHwnd, GET_MEM_BLOCK_MSG, bcast<WPARAM>(clientHwnd),
            bcast<LPARAM>(gameConfig.ProcessName), SMTO_NOTIMEOUTIFNOTHUNG | SMTO_ERRORONEXIT, Timeout, &serverResult);
        ValidateResult(result);
        ValidateServerResult(serverResult);
#pragma endregion

#pragma region Get Offsets
        copyDataCallback = [&](const COPYDATASTRUCT* received) {
            if (received->dwData != GET_MEM_BLOCK_EVENT_RETURN)
                return false;
            gameConfig.Offsets = new DWORD[received->cbData / sizeof(gameConfig.Offsets[0])];
            memcpy(gameConfig.Offsets, received->lpData, received->cbData);
            return true;
        };
        result = SendMessageTimeoutW(serverHwnd, GET_MEM_BLOCK_MSG, bcast<WPARAM>(clientHwnd),
            bcast<LPARAM>(gameConfig.Offsets), SMTO_NOTIMEOUTIFNOTHUNG | SMTO_ERRORONEXIT, Timeout, &serverResult);
        ValidateResult(result);
        ValidateServerResult(serverResult);
#pragma endregion

        return true;
    }
}

static HWND CreateResultWindow() {
    WNDCLASSW windowClass{
        .lpfnWndProc = [](HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
            if (uMsg == WM_COPYDATA && copyDataCallback != nil && copyDataCallback(bcast<const COPYDATASTRUCT*>(lParam)))
                return LRESULT(TRUE);
            return DefWindowProcW(hWnd, uMsg, wParam, lParam);
        },
        .lpszClassName = ClientClassName,
    };
    if (!RegisterClassW(&windowClass)) {
        note::LastErrorToFile(TAG "Cannot register class of the result client window");
        return nil;
    }
    auto messageWindow = CreateWindowW(ClientClassName, ClientClassName, 0, 0, 0, 0, 0, HWND_MESSAGE, nil, nil, nil);
    if (!messageWindow) {
        note::LastErrorToFile(TAG "Cannot create the result client window");
        UnregisterClassW(ClientClassName, nil);
        return nil;
    }
    return messageWindow;
}

static void DestroyResultWindow(HWND hwnd) {
    DestroyWindow(hwnd);
    UnregisterClassW(ClientClassName, nil);
}