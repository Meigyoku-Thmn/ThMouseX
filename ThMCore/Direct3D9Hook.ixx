module;

#include "framework.h"
#include "macro.h"
#include "Include/d3d9.h"
#include "Include/d3dx9core.h"
#include <vector>
#include <string>
#include <comdef.h>

export module core.directx9hook;

import common.minhook;
import common.var;
import common.datatype;
import common.helper;
import common.log;

namespace minhook = common::minhook;
namespace helper = common::helper;
namespace note = common::log;

#define TAG "[DirectX9] "

#define ModulateColor(i) D3DCOLOR_RGBA(i, i, i, 255)
#define SetTextureColorStage(dev, i, op, arg1, arg2)      \
    dev->SetTextureStageState(i, D3DTSS_COLOROP, op);     \
    dev->SetTextureStageState(i, D3DTSS_COLORARG1, arg1); \
    dev->SetTextureStageState(i, D3DTSS_COLORARG2, arg2)

constexpr auto CreateDeviceIdx = 16;

constexpr auto ResetIdx = 16;
constexpr auto PresentIdx = 17;
constexpr auto ErrorMessageTitle = "D3D9 Hook Setup Error";

using namespace std;

inline const char* GetD3dErrStr(const int errorCode) {
    if (errorCode == D3DERR_DEVICELOST)
        return "D3DERR_DEVICELOST";
    if (errorCode == D3DERR_INVALIDCALL)
        return "D3DERR_INVALIDCALL";
    if (errorCode == D3DERR_NOTAVAILABLE)
        return "D3DERR_NOTAVAILABLE";
    if (errorCode == D3DERR_OUTOFVIDEOMEMORY)
        return "D3DERR_OUTOFVIDEOMEMORY";
    return "Unknown error.";
}

using CallbackType = void (*)(void);

namespace core::directx9hook {
    HRESULT WINAPI D3DCreateDevice(IDirect3D9* pD3D, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS *pPresentationParameters, IDirect3DDevice9 **ppReturnedDeviceInterface);
    decltype(&D3DCreateDevice) OriCreateDevice;

    HRESULT WINAPI D3DReset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters);
    decltype(&D3DReset) OriReset;

    HRESULT WINAPI D3DPresent(IDirect3DDevice9* pDevice, RECT* pSourceRect, RECT* pDestRect, HWND hDestWindowOverride, RGNDATA* pDirtyRegion);
    decltype(&D3DPresent) OriPresent;

    vector<CallbackType>& postRenderCallbacks() {
        static vector<CallbackType> backing;
        return backing;
    }
    export DLLEXPORT void RegisterPostRenderCallbacks(CallbackType callback) {
        postRenderCallbacks().push_back(callback);
    }

    export DLLEXPORT bool PopulateMethodRVAs() {
        bool                    result = false;
        DWORD*                  vtable{};
        HRESULT                 rs{};
        IDirect3D9*             pD3D{};
        IDirect3DDevice9*       pDevice{};
        D3DPRESENT_PARAMETERS   d3dpp{};
        DWORD                   baseAddress = (DWORD)GetModuleHandleA("d3d9.dll");

        auto tmpWnd = CreateWindowA("BUTTON", "Temp Window", WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 300, 300, NULL, NULL, NULL, NULL);
        if (tmpWnd == NULL) {
            helper::ReportLastError(ErrorMessageTitle);
            goto CleanAndReturn;
        }

        pD3D = Direct3DCreate9(D3D_SDK_VERSION);
        if (!pD3D) {
            MessageBoxA(NULL, "Failed to create an IDirect3D9 instance.", ErrorMessageTitle, MB_OK | MB_ICONERROR);
            goto CleanAndReturn;
        }

        vtable = *(DWORD**)pD3D;
        gs_d3d9_CreateDevice_RVA = vtable[CreateDeviceIdx] - baseAddress;

        d3dpp.Windowed = TRUE;
        d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
        d3dpp.hDeviceWindow = tmpWnd;
        d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
        d3dpp.BackBufferCount = 1;
        d3dpp.BackBufferWidth = 4;
        d3dpp.BackBufferHeight = 4;
        d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;

        rs = pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dpp.hDeviceWindow, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &pDevice);
        if (FAILED(rs)) {
            MessageBoxA(NULL, (string("Failed to create an IDirect3DDevice9 instance:") + GetD3dErrStr(rs)).c_str(), ErrorMessageTitle, MB_OK | MB_ICONERROR);
            goto CleanAndReturn;
        }

        vtable = *(DWORD**)pDevice;
        gs_d3d9_Reset_RVA = vtable[ResetIdx] - baseAddress;
        gs_d3d9_Present_RVA = vtable[PresentIdx] - baseAddress;

        result = true;
CleanAndReturn:
        pDevice && pDevice->Release();
        pD3D && pD3D->Release();
        tmpWnd && DestroyWindow(tmpWnd);
        return result;
    }

    export vector<minhook::HookConfig> HookConfig() {
        auto baseAddress = (DWORD)GetModuleHandleA("d3d9.dll");
        return {
            {PVOID(baseAddress + gs_d3d9_CreateDevice_RVA), &D3DCreateDevice, (PVOID*)&OriCreateDevice},
            {PVOID(baseAddress + gs_d3d9_Reset_RVA), &D3DReset, (PVOID*)&OriReset},
            {PVOID(baseAddress + gs_d3d9_Present_RVA), &D3DPresent, (PVOID*)&OriPresent},
        };
    }

    // job flags
    bool initialized;
    bool measurementPrepared;
    bool cursorStatePrepared;

    // cursor and screen state
    LPDIRECT3DTEXTURE9  cursorTexture;
    LPD3DXSPRITE        cursorSprite;
    D3DXVECTOR3         cursorPivot;
    D3DXVECTOR2         cursorScale;
    float               d3dScale = 1.f;

    void CleanUp() {
        if (cursorSprite)
            cursorSprite->Release();
        if (cursorTexture)
            cursorTexture->Release();
        cursorSprite = NULL;
        cursorTexture = NULL;
        initialized = false;
        measurementPrepared = false;
        cursorStatePrepared = false;
    }

    HRESULT WINAPI D3DCreateDevice(IDirect3D9* pD3D, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS *pPresentationParameters, IDirect3DDevice9 **ppReturnedDeviceInterface) {
        CleanUp();
        return OriCreateDevice(pD3D, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);
    }

    struct OnInit {
        OnInit() {
            minhook::RegisterUninitializeCallback(Callback);
        }
        static void Callback(bool isProcessTerminating) {
            if (isProcessTerminating)
                return;
            CleanUp();
        }
    } _;

    void Initialize(IDirect3DDevice9* device) {
        if (initialized)
            return;
        initialized = true;

        D3DDEVICE_CREATION_PARAMETERS params;
        device->GetCreationParameters(&params);
        g_hFocusWindow = params.hFocusWindow;

        if (gs_textureFilePath[0] && D3DXCreateTextureFromFileW(device, gs_textureFilePath, &cursorTexture) == D3D_OK) {
            D3DXCreateSprite(device, &cursorSprite);
            D3DSURFACE_DESC cursorSize;
            cursorTexture->GetLevelDesc(0, &cursorSize);
            cursorPivot = {(cursorSize.Height - 1) / 2.f, (cursorSize.Width - 1) / 2.f, 0.f};
        }

        SystemParametersInfoA(SPI_SETCURSORSHADOW, 0, (PVOID)TRUE, 0);
    }

    HRESULT WINAPI D3DReset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters) {
        CleanUp();
        return OriReset(pDevice, pPresentationParameters);
    }

    /*
    this routine:
    - remove window's border if game is fullscreened (exclusive mode)
    - determine g_pixelRate
    - determine g_pixelOffset
    */
    void PrepareMeasurement(IDirect3DDevice9* pDevice) {
        if (measurementPrepared)
            return;
        measurementPrepared = true;

        RECTSIZE clientSize;
        if (GetClientRect(g_hFocusWindow, &clientSize) == FALSE) {
            note::LastErrorToFile(TAG "GetClientRect failed");
            return;
        }

        IDirect3DSurface9* pSurface;
        auto rs = pDevice->GetRenderTarget(0, &pSurface);
        if (rs != D3D_OK) {
            note::HResultToFile(TAG "pDevice->GetRenderTarget failed", rs);
            return;
        }

        D3DSURFACE_DESC d3dSize;
        rs = pSurface->GetDesc(&d3dSize);
        pSurface->Release();
        if (rs != D3D_OK) {
            note::HResultToFile(TAG "pSurface->GetDesc failed", rs);
            return;
        }

        IDirect3DSwapChain9* pSwapChain;
        rs = pDevice->GetSwapChain(0, &pSwapChain);
        if (rs != D3D_OK) {
            note::HResultToFile(TAG "pDevice->GetSwapChain failed", rs);
            return;
        }

        D3DPRESENT_PARAMETERS presentParams;
        rs = pSwapChain->GetPresentParameters(&presentParams);
        pSwapChain->Release();
        if (rs != D3D_OK) {
            note::HResultToFile(TAG "pSwapChain->GetPresentParameters failed", rs);
            return;
        }

        helper::FixWindowCoordinate(!presentParams.Windowed,
            d3dSize.Width, d3dSize.Height, UINT(clientSize.width()), UINT(clientSize.height()));

        if (GetClientRect(g_hFocusWindow, &clientSize) == FALSE) {
            note::LastErrorToFile(TAG "GetClientRect failed");
            return;
        }
        g_pixelRate = float(g_currentConfig.BaseHeight) / clientSize.height();
        g_pixelOffset.X = g_currentConfig.BasePixelOffset.X / g_pixelRate;
        g_pixelOffset.Y = g_currentConfig.BasePixelOffset.Y / g_pixelRate;
    }

    /*
    Determine scaling
    */
    void PrepareCursorState(IDirect3DDevice9* pDevice) {
        if (cursorStatePrepared)
            return;
        cursorStatePrepared = true;
        IDirect3DSurface9* pSurface;
        auto rs = pDevice->GetRenderTarget(0, &pSurface);
        if (rs != D3D_OK) {
            d3dScale = 0.f;
            note::HResultToFile(TAG "pDevice->GetRenderTarget failed", rs);
            return;
        }
        D3DSURFACE_DESC d3dSize;
        rs = pSurface->GetDesc(&d3dSize);
        pSurface->Release();
        if (rs != D3D_OK) {
            d3dScale = 0.f;
            note::HResultToFile(TAG "pSurface->GetDesc failed", rs);
            return;
        }
        auto scale = float(d3dSize.Height) / gs_textureBaseHeight;
        cursorScale = D3DXVECTOR2(scale, scale);

        RECTSIZE clientSize;
        if (GetClientRect(g_hFocusWindow, &clientSize) == FALSE) {
            note::LastErrorToFile(TAG "GetClientRect failed");
            return;
        }
        d3dScale = float(clientSize.width()) / d3dSize.Width;
    }

    void RenderCursor(IDirect3DDevice9* pDevice) {
        if (!cursorTexture)
            return;

        bool needRestoreViewport = false;
        IDirect3DSurface9* pSurface = NULL;
        D3DSURFACE_DESC d3dSize;
        D3DVIEWPORT9 currentViewport;
        if (pDevice->GetRenderTarget(0, &pSurface) == D3D_OK && pSurface->GetDesc(&d3dSize) == D3D_OK) {
            needRestoreViewport = true;
            pDevice->GetViewport(&currentViewport);
            D3DVIEWPORT9 myViewport{
                .X = 0,
                .Y = 0,
                .Width = d3dSize.Width,
                .Height = d3dSize.Height,
            };
            pDevice->SetViewport(&myViewport);
        }
        if (pSurface != NULL)
            pSurface->Release();

        pDevice->BeginScene();

        // scale mouse cursor's position from screen coordinate to D3D coordinate
        POINT pointerPosition = helper::GetPointerPosition();
        D3DXVECTOR3 cursorPositionD3D(float(pointerPosition.x), float(pointerPosition.y), 0.f);
        if (d3dScale != 0.f && d3dScale != 1.f)
            cursorPositionD3D /= d3dScale;

        cursorSprite->Begin(D3DXSPRITE_ALPHABLEND);
        // scale cursor sprite to match the current render resolution
        if (cursorStatePrepared == true) {
            D3DXVECTOR2 scalingPivotD3D(cursorPositionD3D.x, cursorPositionD3D.y);
            D3DXMATRIX scalingMatrixD3D;
            D3DXMatrixTransformation2D(&scalingMatrixD3D, &scalingPivotD3D, 0, &cursorScale, NULL, 0, NULL);
            cursorSprite->SetTransform(&scalingMatrixD3D);
        }
        // draw the cursor
        if (g_inputEnabled) {
            static UCHAR modulate = 0;
            static auto modulateStage = WhiteInc;
            helper::CalculateNextModulate(_ref modulate, _ref modulateStage);
            if (modulateStage == WhiteInc || modulateStage == WhiteDec) {
                SetTextureColorStage(pDevice, 0, D3DTOP_ADD, D3DTA_TEXTURE, D3DTA_DIFFUSE);
                cursorSprite->Draw(cursorTexture, NULL, &cursorPivot, &cursorPositionD3D, ModulateColor(modulate));
            } else {
                cursorSprite->Draw(cursorTexture, NULL, &cursorPivot, &cursorPositionD3D, ModulateColor(modulate));
            }
        } else {
            cursorSprite->Draw(cursorTexture, NULL, &cursorPivot, &cursorPositionD3D, D3DCOLOR_RGBA(255, 200, 200, 128));
        }
        cursorSprite->End();

        if (needRestoreViewport)
            pDevice->SetViewport(&currentViewport);

        pDevice->EndScene();
    }

    HRESULT WINAPI D3DPresent(IDirect3DDevice9 * pDevice, RECT* pSourceRect, RECT* pDestRect, HWND hDestWindowOverride, RGNDATA* pDirtyRegion) {
        Initialize(pDevice);
        PrepareMeasurement(pDevice);
        PrepareCursorState(pDevice);
        RenderCursor(pDevice);
        for (auto& callback : postRenderCallbacks())
            callback();
        return OriPresent(pDevice, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
    }
}
