#include "ComClient.h"
#include <Windows.h>
#include <Windows.h>

#include "../Common/Log.h"
#include "../Common/CallbackStore.h"

namespace note = common::log;
namespace callbackstore = common::callbackstore;

#define TAG "[ComClient] "

static bool allowCoUninitialize;

static void CleanUp(bool isProcessTerminating) {
    if (isProcessTerminating || !allowCoUninitialize)
        return;
    CoUninitialize();
}

namespace core::comclient {
    bool Initialize() {
        auto hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        if (hr == S_OK) {
            allowCoUninitialize = true;
        }
        else if (hr != S_FALSE && hr != RPC_E_CHANGED_MODE) {
            note::HResultToFile(TAG "CoInitializeEx failed", hr);
            return false;
        }
        callbackstore::RegisterUninitializeCallback(CleanUp);
        return true;
    }

    bool GetGameConfig(PWCHAR processName, GameConfig& gameConfig) {
        try {
            return IComServerPtr(__uuidof(ComServer))->GetGameConfig(processName, &gameConfig) == VARIANT_TRUE;
        }
        catch (_com_error& ex) {
            note::ComErrToFile(TAG "IServer->GetGameConfig failed", ex);
            return false;
        }
    }
}