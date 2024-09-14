#include "ComClient.h"
#include <future>
#include <Windows.h>

#include "../Common/Log.h"
#include "../Common/CallbackStore.h"
#include "../Common/Helper.h"

using namespace std;

namespace note = common::log;
namespace callbackstore = common::callbackstore;
namespace helper = common::helper;

#define TAG "[ComClient] "

static HANDLE actCtxHandle;

static void CleanUp(bool isProcessTerminating) {
    if (isProcessTerminating)
        return;
    if (actCtxHandle != INVALID_HANDLE_VALUE)
        ReleaseActCtx(actCtxHandle);
}

namespace core::comclient {
    bool Initialize() {
        callbackstore::RegisterUninitializeCallback(CleanUp);
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
        return async(launch::async, [&]() {
            ULONG_PTR cookie{};
            auto rs = false;
            auto hr = CoInitialize(nullptr);
            if (FAILED(hr)) {
                note::HResultToFile(TAG "CoInitialize failed", hr);
                goto END;
            }
            if (!ActivateActCtx(actCtxHandle, &cookie)) {
                note::LastErrorToFile(TAG "ActivateActCtx failed");
                goto END;
            }
            try {
                helper::ComMethodTimeout([&]() {
                    rs = IComServerPtr(__uuidof(ComServer))->GetGameConfig(processName, &gameConfig) == VARIANT_TRUE;
                }, 1000);
            }
            catch (_com_error& ex) {
                note::ComErrToFile(TAG "IServer->GetGameConfig failed", ex);
                goto END;
            }
        END:
            DeactivateActCtx(0, cookie);
            CoUninitialize();
            return rs;
        }).get();
    }
}