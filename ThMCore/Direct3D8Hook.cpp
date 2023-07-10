#include "framework.h"
#include <DirectX8/Include/d3d8.h>
#include <DirectX8/Include/d3dx8core.h>
#include <vector>
#include <string>
#include <memory>
#include <wrl/client.h>
#include <comdef.h>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include "imgui_impl_dx8.h"

#include "../Common/macro.h"
#include "../Common/macro.h"
#include "../Common/DataTypes.h"
#include "../Common/MinHook.h"
#include "../Common/CallbackStore.h"
#include "../Common/Variables.h"
#include "../Common/Helper.h"
#include "../Common/Log.h"
#include "Direct3D8Hook.h"

namespace minhook = common::minhook;
namespace callbackstore = common::callbackstore;
namespace helper = common::helper;
namespace note = common::log;

#define TAG "[DirectX8] "

#define ToneColor(i) D3DCOLOR_RGBA(i, i, i, 255)

#define SetTextureColorStage(dev, i, op, arg1, arg2)      \
    dev->SetTextureStageState(i, D3DTSS_COLOROP, op);     \
    dev->SetTextureStageState(i, D3DTSS_COLORARG1, arg1); \
    dev->SetTextureStageState(i, D3DTSS_COLORARG2, arg2)

constexpr auto CreateDeviceIdx = 15;

constexpr auto ResetIdx = 14;
constexpr auto PresentIdx = 15;
constexpr auto ErrorMessageTitle = "D3D8 Hook Setup Error";

using namespace std;
using namespace Microsoft::WRL;

inline const char* GetD3dErrStr(const int errorCode) {
    if (errorCode == D3DERR_INVALIDCALL)
        return "D3DERR_INVALIDCALL";
    if (errorCode == D3DERR_NOTAVAILABLE)
        return "D3DERR_NOTAVAILABLE";
    if (errorCode == D3DERR_OUTOFVIDEOMEMORY)
        return "D3DERR_OUTOFVIDEOMEMORY";
    return "Unknown error.";
}

using CallbackType = void (*)(void);

namespace core::directx8hook {
    HRESULT WINAPI D3DCreateDevice(IDirect3D8* pD3D, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice8** ppReturnedDeviceInterface);
    decltype(&D3DCreateDevice) OriCreateDevice;

    HRESULT WINAPI D3DReset(IDirect3DDevice8* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters);
    decltype(&D3DReset) OriReset;

    HRESULT WINAPI D3DPresent(IDirect3DDevice8* pDevice, RECT* pSourceRect, RECT* pDestRect, HWND hDestWindowOverride, RGNDATA* pDirtyRegion);
    decltype(&D3DPresent) OriPresent;

    bool PopulateMethodRVAs() {
        ModuleHandle d3d8(LoadLibraryW(L"d3d8.dll"));
        if (!d3d8) {
            note::LastErrorToFile(TAG "Failed to load d3d8.dll.");
            return false;
        }
        auto _Direct3DCreate8 = (decltype(&Direct3DCreate8))GetProcAddress(d3d8.get(), "Direct3DCreate8");
        if (!_Direct3DCreate8) {
            note::LastErrorToFile(TAG "Failed to import d3d8.dll|Direct3DCreate8.");
            return false;
        }

        WindowHandle tmpWnd(CreateWindowA("BUTTON", "Temp Window",
            WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 300, 300, NULL, NULL, NULL, NULL));
        if (!tmpWnd) {
            helper::ReportLastError(ErrorMessageTitle);
            return false;
        }

        ComPtr<IDirect3D8> pD3D;
        pD3D.Attach(_Direct3DCreate8(D3D_SDK_VERSION));
        if (!pD3D) {
            auto message = "Failed to create an IDirect3D8 instance.";
            MessageBoxA(NULL, message, ErrorMessageTitle, MB_OK | MB_ICONERROR);
            return false;
        }

        auto baseAddress = (DWORD)d3d8.get();

        auto vtable = *(DWORD**)pD3D.Get();
        gs_d3d8_CreateDevice_RVA = vtable[CreateDeviceIdx] - baseAddress;

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
            auto d3dErr = GetD3dErrStr(rs);
            auto message = "Failed to create an IDirect3DDevice8 instance: ";
            MessageBoxA(NULL, (string(message) + d3dErr).c_str(), ErrorMessageTitle, MB_OK | MB_ICONERROR);
            return false;
        }

        vtable = *(DWORD**)pDevice.Get();
        gs_d3d8_Reset_RVA = vtable[ResetIdx] - baseAddress;
        gs_d3d8_Present_RVA = vtable[PresentIdx] - baseAddress;

        return true;
    }

    vector<minhook::HookConfig> HookConfig() {
        auto baseAddress = (DWORD)GetModuleHandleA("d3d8.dll");
        if (!baseAddress)
            return {};
        return {
            {PVOID(baseAddress + gs_d3d8_CreateDevice_RVA), &D3DCreateDevice, (PVOID*)&OriCreateDevice},
            {PVOID(baseAddress + gs_d3d8_Reset_RVA), &D3DReset, (PVOID*)&OriReset},
            {PVOID(baseAddress + gs_d3d8_Present_RVA), &D3DPresent, (PVOID*)&OriPresent},
        };
    }

    // job flags
    bool firstStepPrepared;
    bool measurementPrepared;
    bool cursorStatePrepared;
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

    void CleanUp() {
        SAFE_RELEASE(cursorSprite);
        SAFE_RELEASE(cursorTexture);
        firstStepPrepared = false;
        measurementPrepared = false;
        cursorStatePrepared = false;
    }

    void ShutdownImGui() {
        ImGui_ImplDX8_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }

    HRESULT WINAPI D3DCreateDevice(IDirect3D8* pD3D, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice8** ppReturnedDeviceInterface) {
        CleanUp();
        return OriCreateDevice(
            pD3D, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);
    }

    void TearDownCallback(bool isProcessTerminating) {
        if (isProcessTerminating)
            return;
        ShutdownImGui();
        CleanUp();
    }

    void Initialize() {
        callbackstore::RegisterUninitializeCallback(TearDownCallback);
        callbackstore::RegisterClearMeasurementFlagsCallback(ClearMeasurementFlags);
    }

    void PrepareFirstStep(IDirect3DDevice8* device) {
        if (firstStepPrepared)
            return;
        firstStepPrepared = true;

        D3DDEVICE_CREATION_PARAMETERS params;
        auto rs = device->GetCreationParameters(&params);
        if (FAILED(rs)) {
            note::HResultToFile(TAG "PrepareFirstStep: device->GetCreationParameters failed", rs);
            return;
        }
        g_hFocusWindow = params.hFocusWindow;

        if (gs_textureFilePath[0] && SUCCEEDED(D3DXCreateTextureFromFileW(device, gs_textureFilePath, &cursorTexture))) {
            D3DXCreateSprite(device, &cursorSprite);
            D3DSURFACE_DESC cursorSize;
            cursorTexture->GetLevelDesc(0, &cursorSize);
            cursorPivot = {(cursorSize.Height - 1) / 2.f, (cursorSize.Width - 1) / 2.f};
        }
    }

    HRESULT WINAPI D3DReset(IDirect3DDevice8* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters) {
        ImGui_ImplDX8_InvalidateDeviceObjects();
        CleanUp();
        auto result = OriReset(pDevice, pPresentationParameters);
        if (FAILED(result))
            ImGui_ImplDX8_CreateDeviceObjects();
        return result;
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
            note::HResultToFile(TAG "PrepareMeasurement: pDevice->GetRenderTarget failed", rs);
            return;
        }

        D3DSURFACE_DESC d3dSize;
        rs = pSurface->GetDesc(&d3dSize);
        if (FAILED(rs)) {
            note::HResultToFile(TAG "PrepareMeasurement: pSurface->GetDesc failed", rs);
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
            note::HResultToFile(TAG "PrepareCursorState: pDevice->GetRenderTarget failed", rs);
            return;
        }
        D3DSURFACE_DESC d3dSize;
        rs = pSurface->GetDesc(&d3dSize);
        if (FAILED(rs)) {
            d3dScale = 0.f;
            note::HResultToFile(TAG "PrepareCursorState: pSurface->GetDesc failed", rs);
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
        POINT pointerPosition = helper::GetPointerPosition();
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

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        auto& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        ImGui::StyleColorsDark();
        ImGui_ImplWin32_Init(g_hFocusWindow);
        ImGui_ImplDX8_Init(pDevice);
        auto font = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/tahoma.ttf", 20);
        if (!font)
            io.Fonts->AddFontDefault();
    }

    void RenderImGui(IDirect3DDevice8* pDevice) {
        ImGui_ImplDX8_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        ImGui::ShowDemoWindow();
        ImGui::EndFrame();
        pDevice->BeginScene();
        ImGui::Render();
        ImGui_ImplDX8_RenderDrawData(ImGui::GetDrawData());
        pDevice->EndScene();
    }

    HRESULT WINAPI D3DPresent(IDirect3DDevice8* pDevice, RECT* pSourceRect, RECT* pDestRect, HWND hDestWindowOverride, RGNDATA* pDirtyRegion) {
        PrepareFirstStep(pDevice);
        PrepareMeasurement(pDevice);
        PrepareCursorState(pDevice);
        PrepareImGui(pDevice);
        RenderCursor(pDevice);
        RenderImGui(pDevice);
        callbackstore::TriggerPostRenderCallbacks();
        return OriPresent(pDevice, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
    }
}