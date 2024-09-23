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

    bool GetGameConfig(PWCHAR processName, GameConfig& gameConfig, CommonConfig& commonConfig) {
        packaged_task<bool()> task([&] {
            auto hr = CoInitialize(nullptr);
            if (hr != S_OK && hr != S_FALSE) {
                note::HResultToFile(TAG "CoInitialize failed", hr);
                return false;
            }
            defer({ CoUninitialize(); });
            auto cookie = helper::ActivateActCtx(actCtxHandle);
            if (!cookie) {
                note::LastErrorToFile(TAG "ActivateActCtx failed");
                return false;
            }
            try {
                auto rs = false;
                helper::ComMethodTimeout([&] {
                    rs = IComServerPtr(__uuidof(ComServer))->GetGameConfig(processName, &gameConfig, &commonConfig) == VARIANT_TRUE;
                }, 1000);
                return rs;
            }
            catch (_com_error& ex) {
                note::ComErrToFile(TAG "IServer->GetGameConfig failed", ex);
                return false;
            }
        });
        auto result = task.get_future();
        thread task_td(move(task));
        task_td.join();
        return result.get();
    }
}