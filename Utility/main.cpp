#include <iostream>
#include <vector>
#include <Windows.h>
#include <Shlwapi.h>
#include <dinput.h>
#include <wrl/client.h>

#include "../Common/Variables.h"
#include "../Common/macro.h"
#include "../Common/Log.h"
#include "../Common/MinHook.h"

namespace note = common::log;
namespace minhook = common::minhook;

using namespace std;
using namespace Microsoft::WRL;

void GetDirectInputMappingTableId();

static int resultValue = -1;
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd) {
    auto cmdLine = wstring(lpCmdLine);
    if (cmdLine.starts_with(L"GetDirectInputMappingTableId"))
        GetDirectInputMappingTableId();
    return resultValue;
}

static void Prepare() {
    GetSystemDirectoryW(g_systemDirPath, ARRAYSIZE(g_systemDirPath));
    GetModuleFileNameW(nil, g_currentProcessName, ARRAYSIZE(g_currentProcessName));
    memcpy(g_currentModuleDirPath, g_currentProcessName, sizeof(g_currentProcessName));
    PathRemoveFileSpecW(g_currentModuleDirPath);
    memcpy(g_currentProcessDirPath, g_currentProcessName, sizeof(g_currentProcessName));
    PathRemoveFileSpecW(g_currentProcessDirPath);
    PathStripPathW(g_currentProcessName);
    PathRemoveExtensionW(g_currentProcessName);
}

static decltype(&FindResourceW) FindResourceWOri;
static HRSRC WINAPI FindResourceWHook(HMODULE hModule, LPCWSTR lpName, LPCWSTR lpType) {
    auto rs = FindResourceWOri(hModule, lpName, lpType);
    if (lpType == RT_RCDATA)
        resultValue = bcast<int>(lpName);
    return rs;
}

static void GetDirectInputMappingTableId() {
    Prepare();
    ComPtr<IDirectInput8A> pDInput8;
    auto rs = DirectInput8Create(HINST_THISCOMPONENT, DIRECTINPUT_VERSION, IID_IDirectInput8A, &pDInput8, nil);
    if (FAILED(rs)) {
        note::DxErrToFile("Failed to create an IDirectInput8 instance", rs);
        return;
    }
    if (!minhook::Initialize())
        return;
    if (!minhook::CreateApiHook(vector<minhook::HookApiConfig> {
        { L"Kernel32.dll", "FindResourceW", &FindResourceWHook, &FindResourceWOri },
    })) return;
    if (!minhook::EnableAll())
        return;
    ComPtr<IDirectInputDevice8A> pDevice8;
    rs = pDInput8->CreateDevice(GUID_SysKeyboard, &pDevice8, nil);
    if (FAILED(rs)) {
        note::DxErrToFile("Failed to create an IDirectInputDevice8 instance", rs);
        return;
    }
}