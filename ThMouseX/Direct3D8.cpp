#include "framework.h"
#include <DirectX8/Include/d3d8.h>
#include <DirectX8/Include/d3dx8core.h>
#include <vector>
#include <string>
#include <memory>
#include <wrl/client.h>
#include <comdef.h>
#include <mutex>
#include <imgui.h>
#include "imgui_impl_win32.h"
#include "imgui_impl_dx8.h"
#include "ImGuiOverlay.h"

#include "../Common/macro.h"
#include "../Common/macro.h"
#include "../Common/DataTypes.h"
#include "../Common/MinHook.h"
#include "../Common/CallbackStore.h"
#include "../Common/Variables.h"
#include "../Common/Helper.h"
#include "../Common/Helper.Encoding.h"
#include "../Common/Log.h"
#include "Direct3D8.h"

namespace minhook = common::minhook;
namespace callbackstore = common::callbackstore;
namespace helper = common::helper;
namespace encoding = common::helper::encoding;
namespace note = common::log;
namespace imguioverlay = core::imguioverlay;

#define TAG "[DirectX8] "

#define ToneColor(i) D3DCOLOR_RGBA(i, i, i, 255)

#define SetTextureColorStage(dev, i, op, arg1, arg2)      \
    dev->SetTextureStageState(i, D3DTSS_COLOROP, op);     \
    dev->SetTextureStageState(i, D3DTSS_COLORARG1, arg1); \
    dev->SetTextureStageState(i, D3DTSS_COLORARG2, arg2)

constexpr auto CreateDeviceIdx = 15;

constexpr auto ResetIdx = 14;
constexpr auto PresentIdx = 15;

using namespace std;
using namespace Microsoft::WRL;

using CallbackType = void (*)(void);

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

    void ClearMeasurementFlags() {
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

    void CleanUp(bool forReal = false) {
        ImGui_ImplDX8_InvalidateDeviceObjects();
        SAFE_RELEASE(cursorSprite);
        SAFE_RELEASE(cursorTexture);
        firstStepPrepared = false;
        measurementPrepared = false;
        cursorStatePrepared = false;
        imGuiConfigured = false;
        if (forReal && imGuiPrepared) {
            ImGui_ImplDX8_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();
        }
    }

    HRESULT WINAPI D3DCreateDevice(IDirect3D8* pD3D, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice8** ppReturnedDeviceInterface) {
        CleanUp();
        return OriCreateDevice(
            pD3D, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);
    }

    void TearDownCallback(bool isProcessTerminating) {
        if (isProcessTerminating)
            return;
        CleanUp(true);
    }

    void Initialize() {
        static bool initialized = false;
        static mutex mtx;
        HMODULE d3d8{};
        {
            const lock_guard lock(mtx);
            if (initialized)
                return;
            d3d8 = GetModuleHandleW((g_systemDirPath + wstring(L"\\d3d8.dll")).c_str());
            if (!d3d8)
                return;
            initialized = true;
        }

        auto _Direct3DCreate8 = (decltype(&Direct3DCreate8))GetProcAddress(d3d8, "Direct3DCreate8");
        if (!_Direct3DCreate8) {
            note::LastErrorToFile(TAG "Failed to import d3d8.dll|Direct3DCreate8.");
            return;
        }

        WindowHandle tmpWnd(CreateWindowA("BUTTON", "Temp Window",
            WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 300, 300, NULL, NULL, NULL, NULL));
        if (!tmpWnd) {
            note::LastErrorToFile(TAG "Failed to create a temporary window.");
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

        auto baseAddress = (DWORD)d3d8;
        auto vtable = *(DWORD**)pD3D.Get();
        auto vtable2 = *(DWORD**)pDevice.Get();

        callbackstore::RegisterUninitializeCallback(TearDownCallback);
        callbackstore::RegisterClearMeasurementFlagsCallback(ClearMeasurementFlags);

        minhook::CreateHook(vector<minhook::HookConfig>{
            { PVOID(vtable[CreateDeviceIdx]), & D3DCreateDevice, (PVOID*)&OriCreateDevice },
            {PVOID(vtable2[ResetIdx]), &D3DReset, (PVOID*)&OriReset},
            {PVOID(vtable2[PresentIdx]), &D3DPresent, (PVOID*)&OriPresent},
        });
    }

    void PrepareFirstStep(IDirect3DDevice8* device) {
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

        if (gs_textureFilePath[0] && SUCCEEDED(D3DXCreateTextureFromFileW(device, gs_textureFilePath, &cursorTexture))) {
            D3DXCreateSprite(device, &cursorSprite);
            D3DSURFACE_DESC cursorSize;
            cursorTexture->GetLevelDesc(0, &cursorSize);
            cursorPivot = {(cursorSize.Height - 1) / 2.f, (cursorSize.Width - 1) / 2.f};
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
    void PrepareMeasurement(IDirect3DDevice8* pDevice) {
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
            d3dSize.Width, d3dSize.Height, UINT(clientSize.width()), UINT(clientSize.height()));

        if (GetClientRect(g_hFocusWindow, &clientSize) == FALSE) {
            note::LastErrorToFile(TAG "PrepareMeasurement: GetClientRect failed (2)");
            return;
        }
        g_pixelRate = float(g_currentConfig.BaseHeight) / clientSize.height();
        g_pixelOffset.X = g_currentConfig.BasePixelOffset.X / g_pixelRate;
        g_pixelOffset.Y = g_currentConfig.BasePixelOffset.Y / g_pixelRate;
        imGuiMousePosScaleX = float(clientSize.width()) / d3dSize.Width;
        imGuiMousePosScaleY = float(clientSize.height()) / d3dSize.Height;
    }

    /*
    Determine scaling
    */
    void PrepareCursorState(IDirect3DDevice8* pDevice) {
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
        auto scale = float(d3dSize.Height) / gs_textureBaseHeight;
        cursorScale = D3DXVECTOR2(scale, scale);

        RECTSIZE clientSize{};
        if (GetClientRect(g_hFocusWindow, &clientSize) == FALSE) {
            note::LastErrorToFile(TAG "PrepareCursorState: GetClientRect failed");
            return;
        }
        d3dScale = float(clientSize.width()) / d3dSize.Width;
    }

    void RenderCursor(IDirect3DDevice8* pDevice) {
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
        D3DXVECTOR2 cursorPositionD3D(float(pointerPosition.x), float(pointerPosition.y));
        if (d3dScale != 0.f && d3dScale != 1.f)
            cursorPositionD3D /= d3dScale;

        // DirectX8 doesn't have a conventional method for sprite pivot
        cursorPositionD3D.x -= cursorPivot.x;
        cursorPositionD3D.y -= cursorPivot.y;

        cursorSprite->Begin();
        // draw the cursor and scale cursor sprite to match the current render resolution
        if (g_inputEnabled) {
            static UCHAR tone = 0;
            static auto toneStage = WhiteInc;
            helper::CalculateNextTone(_ref tone, _ref toneStage);
            if (toneStage == WhiteInc || toneStage == WhiteDec) {
                // default behaviour: texture color * diffuse color
                // this:              texture color + diffuse color (except alpha)
                SetTextureColorStage(pDevice, 0, D3DTOP_ADD, D3DTA_TEXTURE, D3DTA_DIFFUSE);
                cursorSprite->Draw(cursorTexture, NULL, &cursorScale, NULL, 0, &cursorPositionD3D, ToneColor(tone));
            }
            else {
                cursorSprite->Draw(cursorTexture, NULL, &cursorScale, NULL, 0, &cursorPositionD3D, ToneColor(tone));
            }
        }
        else {
            cursorSprite->Draw(cursorTexture, NULL, &cursorScale, NULL, 0, &cursorPositionD3D, D3DCOLOR_RGBA(255, 200, 200, 128));
        }
        cursorSprite->End();

        if (needRestoreViewport)
            pDevice->SetViewport(&currentViewport);

        pDevice->EndScene();
    }

    void PrepareImGui(IDirect3DDevice8* pDevice) {
        if (imGuiPrepared)
            return;
        imGuiPrepared = true;

        if (!g_hFocusWindow)
            return;

        imguioverlay::Prepare();
        ImGui_ImplWin32_Init(g_hFocusWindow);
        ImGui_ImplDX8_Init(pDevice);
    }

    void ConfigureImGui(IDirect3DDevice8* pDevice) {
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

        imguioverlay::Configure(float(d3dSize.Height) / gs_imGuiBaseVerticalResolution);
    }

    void RenderImGui(IDirect3DDevice8* pDevice) {
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