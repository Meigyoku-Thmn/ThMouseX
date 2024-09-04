#include "ComClient.h"
#include <Windows.h>
#include <Windows.h>

#include "../Common/Log.h"
#include "../Common/CallbackStore.h"

namespace note = common::log;
namespace callbackstore = common::callbackstore;

#define TAG "[ComClient] "

static bool allowCoUninitialize;
static HANDLE actCtxHandle;

static void CleanUp(bool isProcessTerminating) {
    if (isProcessTerminating)
        return;
    if (allowCoUninitialize)
        CoUninitialize();
    if (actCtxHandle != INVALID_HANDLE_VALUE)
        ReleaseActCtx(actCtxHandle);
}

namespace core::comclient {
    bool Initialize() {
        callbackstore::RegisterUninitializeCallback(CleanUp);
        auto hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        if (hr == S_OK) {
            allowCoUninitialize = true;
        }
        else if (hr != S_FALSE && hr != RPC_E_CHANGED_MODE) {
            note::HResultToFile(TAG "CoInitializeEx failed", hr);
            return false;
        }
        ACTCTXW actCtx{
            .cbSize = sizeof(ACTCTXW),
            .dwFlags = ACTCTX_FLAG_RESOURCE_NAME_VALID | ACTCTX_FLAG_HMODULE_VALID,
            .lpResourceName = MAKEINTRESOURCEW(2),
            .hModule = g_coreModule,
        };
        actCtxHandle = CreateActCtxW(&actCtx);
        if (actCtxHandle == INVALID_HANDLE_VALUE) {
            note::LastErrorToFile(TAG "CreateActCtxW failed");
            return false;
        }
        return true;
    }

    bool GetGameConfig(PWCHAR processName, GameConfig& gameConfig) {
        ULONG_PTR cookie;
        if (!ActivateActCtx(actCtxHandle, &cookie)) {
            note::LastErrorToFile(TAG "ActivateActCtx failed");
            return false;
        }
        bool rs;
        try {
            rs = IComServerPtr(__uuidof(ComServer))->GetGameConfig(processName, &gameConfig) == VARIANT_TRUE;
        }
        catch (_com_error& ex) {
            note::ComErrToFile(TAG "IServer->GetGameConfig failed", ex);
            return false;
        }
        if (!DeactivateActCtx(0, cookie)) {
            note::LastErrorToFile(TAG "DeactivateActCtx failed");
            return false;
        }
        return rs;
    }
}