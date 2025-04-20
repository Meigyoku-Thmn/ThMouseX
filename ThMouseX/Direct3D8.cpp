#ifndef _WIN64
#include <Windows.h>
#include <DirectX8/Include/d3d8.h>
#include <DirectX8/Include/d3dx8core.h>
#include <vector>
#include <string>
#include <memory>
#include <wrl/client.h>
#include <comdef.h>
#include <mutex>
#include <cstdint>
#include <imgui.h>
#include "imgui_impl_win32.h"
#include "imgui_impl_dx8.h"
#include "ImGuiOverlay.h"

#include "../Common/macro.h"
#include "../Common/DataTypes.h"
#include "../Common/MinHook.h"
#include "../Common/CallbackStore.h"
#include "../Common/Variables.h"
#include "../Common/Helper.h"
#include "../Common/Helper.Encoding.h"
#include "../Common/Log.h"
#include "Direct3D8.h"

static CommonConfig& g_c = g_commonConfig;

namespace minhook = common::minhook;
namespace callbackstore = common::callbackstore;
namespace helper = common::helper;
namespace encoding = common::helper::encoding;
namespace note = common::log;
namespace imguioverlay = core::imguioverlay;

#define TAG "[DirectX8] "

static constexpr D3DCOLOR ToneColor(UCHAR i) {
    return D3DCOLOR_RGBA(i, i, i, 255);
}

static void SetTextureColorStage(IDirect3DDevice8* dev, DWORD stage, DWORD op, DWORD arg1, DWORD arg2) {
    dev->SetTextureStageState(stage, D3DTSS_COLOROP, op);
    dev->SetTextureStageState(stage, D3DTSS_COLORARG1, arg1);
    dev->SetTextureStageState(stage, D3DTSS_COLORARG2, arg2);
}

constexpr auto CreateDeviceIdx = 15;

constexpr auto ResetIdx = 14;
constexpr auto PresentIdx = 15;

using namespace std;
using namespace Microsoft::WRL;

namespace core::directx8 {
    HRESULT WINAPI D3DCreateDevice(IDirect3D8* pD3D, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice8** ppReturnedDeviceInterface);
    decltype(&D3DCreateDevice) OriCreateDevice;

    HRESULT WINAPI D3DReset(IDirect3DDevice8* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters);
    decltype(&D3DReset) OriReset;

    HRESULT WINAPI D3DPresent(IDirect3DDevice8* pDevice, RECT* pSourceRect, RECT* pDestRect, HWND hDestWindowOverride, RGNDATA* pDirtyRegion);
    decltype(&D3DPresent) OriPresent;

    // job flags
    bool firstStepPrepared;
    bool measurementPrepared;
    bool cursorStatePrepared;
    bool imGuiConfigured;

    bool imGuiPrepared;
    bool imGuiCorePrepared;

    static void ClearMeasurementFlags() {
        measurementPrepared = false;
        cursorStatePrepared = false;
    }

    // cursor and screen state
    LPDIRECT3DTEXTURE8  cursorTexture;
    LPD3DXSPRITE        cursorSprite;
    D3DXVECTOR2         cursorPivot;
    D3DXVECTOR2         cursorScale;
    float               d3dScale = 1.f;
    float               imGuiMousePosScaleX = 1.f;
    float               imGuiMousePosScaleY = 1.f;

    HMODULE d3d8;

    static void CleanUp(bool forReal = false) {
        if (imGuiPrepared) {
            ImGui_ImplDX8_Shutdown();
            ImGui_ImplWin32_Shutdown();
        }
        helper::SafeRelease(cursorSprite);
        helper::SafeRelease(cursorTexture);
        firstStepPrepared = false;
        measurementPrepared = false;
        cursorStatePrepared = false;
        imGuiConfigured = false;
        imGuiPrepared = false;
        if (forReal) {
            if (imGuiCorePrepared)
                ImGui::DestroyContext();
            helper::SafeFreeLib(d3d8);
        }
    }

    HRESULT WINAPI D3DCreateDevice(IDirect3D8* pD3D, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice8** ppReturnedDeviceInterface) {
        CleanUp();
        return OriCreateDevice(
            pD3D, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);
    }

    static void TearDownCallback(bool isProcessTerminating) {
        if (isProcessTerminating)
            return;
        CleanUp(true);
    }

    static bool initialized = false;
    static mutex mtx;
    void Initialize() {
        {
            const scoped_lock lock(mtx);
            if (initialized)
                return;
            GetModuleHandleExW(0, (g_systemDirPath + wstring(L"\\d3d8.dll")).c_str(), &d3d8);
            if (!d3d8)
                return;
            initialized = true;
        }

        auto _Direct3DCreate8 = bcast<decltype(&Direct3DCreate8)>(GetProcAddress(d3d8, "Direct3DCreate8"));
        if (!_Direct3DCreate8) {
            note::LastErrorToFile(TAG "Failed to import d3d8.dll|Direct3DCreate8");
            return;
        }

        WindowHandle tmpWnd(CreateWindowA("BUTTON", "Temp Window",
            WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 300, 300, nil, nil, nil, nil));
        if (!tmpWnd) {
            note::LastErrorToFile(TAG "Failed to create a temporary window");
            return;
        }

        ComPtr<IDirect3D8> pD3D;
        pD3D.Attach(_Direct3DCreate8(D3D_SDK_VERSION));
        if (!pD3D) {
            note::ToFile(TAG "Failed to create an IDirect3D8 instance.");
            return;
        }

        D3DPRESENT_PARAMETERS d3dpp{
            .BackBufferWidth = 4,
            .BackBufferHeight = 4,
            .BackBufferFormat = D3DFMT_X8R8G8B8,
            .BackBufferCount = 1,
            .SwapEffect = D3DSWAPEFFECT_DISCARD,
            .hDeviceWindow = tmpWnd.get(),
            .Windowed = TRUE,
        };
        ComPtr<IDirect3DDevice8> pDevice;
        auto rs = pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dpp.hDeviceWindow, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &pDevice);
        if (FAILED(rs)) {
            note::DxErrToFile(TAG "Failed to create an IDirect3DDevice8 instance", rs);
            return;
        }

        auto vtable = *bcast<uintptr_t**>(pD3D.Get());
        auto vtable2 = *bcast<uintptr_t**>(pDevice.Get());

        callbackstore::RegisterUninitializeCallback(TearDownCallback);
        callbackstore::RegisterClearMeasurementFlagsCallback(ClearMeasurementFlags);

        minhook::CreateHook(vector<minhook::HookConfig>{
            { bcast<PVOID>(vtable[CreateDeviceIdx]), &D3DCreateDevice, &OriCreateDevice, APP_NAME "_D3DCreateDevice" },
            { bcast<PVOID>(vtable2[ResetIdx]), &D3DReset, &OriReset, APP_NAME "_D3DReset" },
            { bcast<PVOID>(vtable2[PresentIdx]), &D3DPresent, &OriPresent, APP_NAME "_D3DPresent" },
        });
    }

    static void PrepareFirstStep(IDirect3DDevice8* device) {
        if (firstStepPrepared)
            return;
        firstStepPrepared = true;

        D3DDEVICE_CREATION_PARAMETERS params;
        auto rs = device->GetCreationParameters(&params);
        if (FAILED(rs)) {
            note::DxErrToFile(TAG "PrepareFirstStep: device->GetCreationParameters failed", rs);
            return;
        }
        g_hFocusWindow = params.hFocusWindow;
        g_isMinimized = IsIconic(g_hFocusWindow);

        if (g_c.TextureFilePath[0] && SUCCEEDED(D3DXCreateTextureFromFileW(device, g_c.TextureFilePath, &cursorTexture))) {
            D3DXCreateSprite(device, &cursorSprite);
            D3DSURFACE_DESC cursorSize;
            cursorTexture->GetLevelDesc(0, &cursorSize);
            cursorPivot = { scast<float>(cursorSize.Height - 1) / 2.f, scast<float>(cursorSize.Width - 1) / 2.f };
        }
    }

    HRESULT WINAPI D3DReset(IDirect3DDevice8* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters) {
        CleanUp();
        return OriReset(pDevice, pPresentationParameters);
    }

    /*
    this routine:
    - remove window's border if game is fullscreened (exclusive mode)
    - determine g_pixelRate
    - determine g_pixelOffset
    */
    static void PrepareMeasurement(IDirect3DDevice8* pDevice) {
        if (measurementPrepared)
            return;
        measurementPrepared = true;

        if (!g_hFocusWindow)
            return;

        RECTSIZE clientSize{};
        if (GetClientRect(g_hFocusWindow, &clientSize) == FALSE) {
            note::LastErrorToFile(TAG "PrepareMeasurement: GetClientRect failed (1)");
            return;
        }

        ComPtr<IDirect3DSurface8> pSurface;
        auto rs = pDevice->GetRenderTarget(&pSurface);
        if (FAILED(rs)) {
            note::DxErrToFile(TAG "PrepareMeasurement: pDevice->GetRenderTarget failed", rs);
            return;
        }

        D3DSURFACE_DESC d3dSize;
        rs = pSurface->GetDesc(&d3dSize);
        if (FAILED(rs)) {
            note::DxErrToFile(TAG "PrepareMeasurement: pSurface->GetDesc failed", rs);
            return;
        }

        // There is no way to get back D3DPRESENT_PARAMETERS in DirectX8
        // So, use a heuristic method to detect fullscreen mode
        helper::FixWindowCoordinate(helper::TestFullscreenHeuristically(),
            d3dSize.Width, d3dSize.Height, scast<UINT>(clientSize.width()), scast<UINT>(clientSize.height()));

        if (GetClientRect(g_hFocusWindow, &clientSize) == FALSE) {
            note::LastErrorToFile(TAG "PrepareMeasurement: GetClientRect failed (2)");
            return;
        }
        if (g_gameConfig.BaseHeight != -1)
            g_pixelRate = scast<float>(g_gameConfig.BaseHeight) / scast<float>(clientSize.height());
        g_pixelOffset.X = g_gameConfig.BasePixelOffset.X / g_pixelRate;
        g_pixelOffset.Y = g_gameConfig.BasePixelOffset.Y / g_pixelRate;
        imGuiMousePosScaleX = scast<float>(clientSize.width()) / scast<float>(d3dSize.Width);
        imGuiMousePosScaleY = scast<float>(clientSize.height()) / scast<float>(d3dSize.Height);
    }

    /*
    Determine scaling
    */
    static void PrepareCursorState(IDirect3DDevice8* pDevice) {
        if (cursorStatePrepared)
            return;
        cursorStatePrepared = true;

        if (!g_hFocusWindow)
            return;

        ComPtr<IDirect3DSurface8> pSurface;
        auto rs = pDevice->GetRenderTarget(&pSurface);
        if (FAILED(rs)) {
            d3dScale = 0.f;
            note::DxErrToFile(TAG "PrepareCursorState: pDevice->GetRenderTarget failed", rs);
            return;
        }
        D3DSURFACE_DESC d3dSize;
        rs = pSurface->GetDesc(&d3dSize);
        if (FAILED(rs)) {
            d3dScale = 0.f;
            note::DxErrToFile(TAG "PrepareCursorState: pSurface->GetDesc failed", rs);
            return;
        }
        auto scale = scast<float>(d3dSize.Height) / scast<float>(g_c.TextureBaseHeight);
        cursorScale = D3DXVECTOR2(scale, scale);

        RECTSIZE clientSize{};
        if (GetClientRect(g_hFocusWindow, &clientSize) == FALSE) {
            note::LastErrorToFile(TAG "PrepareCursorState: GetClientRect failed");
            return;
        }
        d3dScale = scast<float>(clientSize.width()) / scast<float>(d3dSize.Width);
    }

    static UCHAR tone = 0;
    static auto toneStage = ModulateStage::WhiteInc;
    static void RenderCursor(IDirect3DDevice8* pDevice) {
        if (!cursorTexture)
            return;

        bool needRestoreViewport = false;
        ComPtr<IDirect3DSurface8> pSurface;
        D3DSURFACE_DESC d3dSize;
        D3DVIEWPORT8 currentViewport;
        if (SUCCEEDED(pDevice->GetRenderTarget(&pSurface)) && SUCCEEDED(pSurface->GetDesc(&d3dSize))) {
            needRestoreViewport = true;
            pDevice->GetViewport(&currentViewport);
            D3DVIEWPORT8 myViewport{
                .X = 0,
                .Y = 0,
                .Width = d3dSize.Width,
                .Height = d3dSize.Height,
            };
            pDevice->SetViewport(&myViewport);
        }

        pDevice->BeginScene();

        // scale mouse cursor's position from screen coordinate to D3D coordinate
        auto pointerPosition = helper::GetPointerPosition();
        D3DXVECTOR2 cursorPositionD3D(scast<float>(pointerPosition.x), scast<float>(pointerPosition.y));
        if (d3dScale != 0.f && d3dScale != 1.f)
            cursorPositionD3D /= d3dScale;

        // DirectX8 doesn't have a conventional method for sprite pivot
        cursorPositionD3D.x -= cursorPivot.x;
        cursorPositionD3D.y -= cursorPivot.y;

        cursorSprite->Begin();
        // draw the cursor and scale cursor sprite to match the current render resolution
        if (g_inputEnabled) {
            using enum ModulateStage;
            helper::CalculateNextTone(tone, toneStage);
            if (toneStage == WhiteInc || toneStage == WhiteDec) {
                // default behaviour: texture color * diffuse color
                // this:              texture color + diffuse color (except alpha)
                SetTextureColorStage(pDevice, 0, D3DTOP_ADD, D3DTA_TEXTURE, D3DTA_DIFFUSE);
                cursorSprite->Draw(cursorTexture, nil, &cursorScale, nil, 0, &cursorPositionD3D, ToneColor(tone));
            }
            else {
                cursorSprite->Draw(cursorTexture, nil, &cursorScale, nil, 0, &cursorPositionD3D, ToneColor(tone));
            }
        }
        else {
            cursorSprite->Draw(cursorTexture, nil, &cursorScale, nil, 0, &cursorPositionD3D, D3DCOLOR_RGBA(255, 200, 200, 128));
        }
        cursorSprite->End();

        if (needRestoreViewport)
            pDevice->SetViewport(&currentViewport);

        pDevice->EndScene();
    }

    static void PrepareImGui(IDirect3DDevice8* pDevice) {
        if (imGuiPrepared)
            return;
        imGuiPrepared = true;

        if (!g_hFocusWindow)
            return;

        if (!imGuiCorePrepared) {
            imguioverlay::Prepare();
            imGuiCorePrepared = true;
        }
        ImGui_ImplWin32_Init(g_hFocusWindow);
        ImGui_ImplDX8_Init(pDevice);
    }

    static void ConfigureImGui(IDirect3DDevice8* pDevice) {
        if (imGuiConfigured)
            return;
        imGuiConfigured = true;

        auto& io = ImGui::GetIO();
        io.Fonts->Clear();

        ComPtr<IDirect3DSurface8> pSurface;
        auto rs = pDevice->GetRenderTarget(&pSurface);
        if (FAILED(rs)) {
            note::DxErrToFile(TAG "ConfigureImGui: pDevice->GetRenderTarget failed", rs);
            return;
        }
        D3DSURFACE_DESC d3dSize;
        rs = pSurface->GetDesc(&d3dSize);
        if (FAILED(rs)) {
            note::DxErrToFile(TAG "ConfigureImGui: pSurface->GetDesc failed", rs);
            return;
        }

        imguioverlay::Configure(scast<float>(d3dSize.Height) / scast<float>(g_c.ImGuiBaseVerticalResolution));
    }

    static void RenderImGui(IDirect3DDevice8* pDevice) {
        if (!g_showImGui)
            return;
        ImGui_ImplDX8_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ComPtr<IDirect3DSurface8> pSurface;
        auto rs = pDevice->GetRenderTarget(&pSurface);
        if (FAILED(rs)) {
            note::DxErrToFile(TAG "RenderImGui: pDevice->GetRenderTarget failed", rs);
            return;
        }
        D3DSURFACE_DESC d3dSize;
        rs = pSurface->GetDesc(&d3dSize);
        if (FAILED(rs)) {
            note::DxErrToFile(TAG "RenderImGui: pSurface->GetDesc failed", rs);
            return;
        }
        auto drawData = imguioverlay::Render(d3dSize.Width, d3dSize.Height, imGuiMousePosScaleX, imGuiMousePosScaleY);
        pDevice->BeginScene();
        ImGui_ImplDX8_RenderDrawData(drawData);
        pDevice->EndScene();
    }

    HRESULT WINAPI D3DPresent(IDirect3DDevice8* pDevice, RECT* pSourceRect, RECT* pDestRect, HWND hDestWindowOverride, RGNDATA* pDirtyRegion) {
        PrepareFirstStep(pDevice);
        PrepareMeasurement(pDevice);
        PrepareCursorState(pDevice);
        PrepareImGui(pDevice);
        ConfigureImGui(pDevice);
        RenderCursor(pDevice);
        RenderImGui(pDevice);
        callbackstore::TriggerPostRenderCallbacks();
        return OriPresent(pDevice, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
    }
}
#endif