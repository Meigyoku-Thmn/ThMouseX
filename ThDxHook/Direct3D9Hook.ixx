module;

#include "framework.h"
#include "Include/d3d9.h"
#include "Include/d3dx9core.h"
#include <vector>
#include <string>

export module core.directx9hook;

import common.minhook;
import common.var;
import common.datatype;
import common.helper;

constexpr auto ResetIdx = 16;
constexpr auto EndSceneIdx = 42;
constexpr auto ErrorMessageTitle = "D3D9 Hook Setup Error";

using namespace std;

HRESULT WINAPI MyReset(IDirect3DDevice9 *pDevice, D3DPRESENT_PARAMETERS *pPresentationParameters);
decltype(&MyReset) OriReset;
HRESULT WINAPI MyEndScene(IDirect3DDevice9 *pDevice);
decltype(&MyEndScene) OriEndScene;

constexpr const char* GetD3dErrStr(int errorCode) {
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

export bool PopulateD3D9MethodRVAs() {
    bool result = false;
    DWORD *vtable{};
    HRESULT rs{};
    IDirect3D9 *pD3D{};
    IDirect3DDevice9 *pDevice{};
    D3DPRESENT_PARAMETERS d3dpp{};
    DWORD baseAddress{};

    auto tmpWnd = CreateWindowA("BUTTON", "Temp Window", WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 300, 300, NULL, NULL, NULL, NULL);
    if (tmpWnd == NULL) {
        ReportLastError(ErrorMessageTitle);
        goto CleanAndReturn;
    }

    pD3D = Direct3DCreate9(D3D_SDK_VERSION);
    if (!pD3D) {
        MessageBox(NULL, "Failed to create an IDirect3D9 instance.", ErrorMessageTitle, MB_OK | MB_ICONERROR);
        goto CleanAndReturn;
    }

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
        MessageBox(NULL, (string("Failed to create an IDirect3DDevice9 instance:") + GetD3dErrStr(rs)).c_str(), ErrorMessageTitle, MB_OK | MB_ICONERROR);
        goto CleanAndReturn;
    }

    vtable = *(DWORD**)pDevice;
    baseAddress = (DWORD)GetModuleHandleA("d3d9.dll");

    gs_d3d9_Reset_RVA = vtable[ResetIdx] - baseAddress;
    gs_d3d9_EndScene_RVA = vtable[EndSceneIdx] - baseAddress;

    result = true;
CleanAndReturn:
    pDevice && pDevice->Release();
    pD3D && pD3D->Release();
    tmpWnd && DestroyWindow(tmpWnd);
    return result;
}

export vector<MHookConfig> D3D9HookConfig() {
    auto baseAddress = (DWORD)GetModuleHandleA("d3d9.dll");
    return {
        {PVOID(baseAddress + gs_d3d9_Reset_RVA), &MyReset, (PVOID*)&OriReset},
        {PVOID(baseAddress + gs_d3d9_EndScene_RVA), &MyEndScene, (PVOID*)&OriEndScene},
    };
}

bool initialized = false;
bool prepared = false;
HWND m_hFocusWindow;
bool allowGetRenderData = true;
UINTSIZE backBufferSize;
UINTSIZE frontBufferSize;
float currentScale = 1.0f;
float cursorWidthCenter = 0;
float cursorHeightCenter = 0;
D3DXMATRIX spriteMatrix;
D3DXVECTOR2 scalingFactor;
LPD3DXSPRITE m_sprite;
LPDIRECT3DTEXTURE9 m_texture;

void Initialize(IDirect3DDevice9 *device) {
    if (initialized)
        return;
    initialized = true;
    D3DDEVICE_CREATION_PARAMETERS params;
    device->GetCreationParameters(&params);
    m_hFocusWindow = g_hFocusWindow = params.hFocusWindow;
    D3DXCreateSprite(device, &m_sprite);
    if (gs_textureFilePath[0]) {
        auto rs = D3DXCreateTextureFromFile(device, gs_textureFilePath, &m_texture);
        if (rs == D3D_OK) {
            D3DSURFACE_DESC desc;
            m_texture->GetLevelDesc(0, &desc);
            cursorHeightCenter = (desc.Height - 1) / 2.0f;
            cursorWidthCenter = (desc.Width - 1) / 2.0f;
        }
    }
    SystemParametersInfo(SPI_SETCURSORSHADOW, 0, (PVOID)TRUE, SPIF_SENDCHANGE);
}

HRESULT WINAPI MyReset(IDirect3DDevice9 *pDevice, D3DPRESENT_PARAMETERS *pPresentationParameters) {
    Initialize(pDevice);
    if (pPresentationParameters->hDeviceWindow != NULL)
        m_hFocusWindow = pPresentationParameters->hDeviceWindow;
    g_hFocusWindow = m_hFocusWindow;
    allowGetRenderData = true;
    prepared = false;
    return OriReset(pDevice, pPresentationParameters);
}

HRESULT WINAPI MyEndScene(IDirect3DDevice9 *pDevice) {
    Initialize(pDevice);
    POINT pos{};
    GetCursorPos(&pos);
    if (prepared == false) {
        g_windowed = true;
        prepared = true;
        do {
            RECT hwndClientRect;
            if (GetClientRect(m_hFocusWindow, &hwndClientRect) == 0)
                break;
            IDirect3DSurface9* pSurface;
            HRESULT rs = pDevice->GetRenderTarget(0, &pSurface);
            if (rs != D3D_OK)
                break;
            pSurface->Release();
            D3DSURFACE_DESC SurfaceDesc;
            rs = pSurface->GetDesc(&SurfaceDesc);
            if (rs != D3D_OK)
                break;
            if (SurfaceDesc.Width > hwndClientRect.right || SurfaceDesc.Height > hwndClientRect.bottom) {
                g_windowed = false; // so-so way to determine "windowed or fullscreen"
                // clear border to avoid "click-out-of-bound"
                LONG style = GetWindowLongPtrW(m_hFocusWindow, GWL_STYLE);
                if (style == 0)
                    break;
                style &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);
                LONG lExStyle = GetWindowLongPtrW(m_hFocusWindow, GWL_EXSTYLE);
                if (lExStyle == 0)
                    break;
                lExStyle &= ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
                if (SetWindowLongPtrW(m_hFocusWindow, GWL_STYLE, style) == 0)
                    break;
                if (SetWindowLongPtrW(m_hFocusWindow, GWL_EXSTYLE, lExStyle) == 0)
                    break;
                SetWindowPos(m_hFocusWindow, NULL, 0, 0,
                    SurfaceDesc.Width, SurfaceDesc.Height,
                    SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOOWNERZORDER);
            }
            if (GetClientRect(m_hFocusWindow, &hwndClientRect) == 0)
                break;
            g_currentGameConfig.PixelRate = (float)g_currentGameConfig.BaseResolutionX / hwndClientRect.right;
            g_currentGameConfig.PixelOffset.X = g_currentGameConfig.BasePixelOffset.X / g_currentGameConfig.PixelRate;
            g_currentGameConfig.PixelOffset.Y = g_currentGameConfig.BasePixelOffset.Y / g_currentGameConfig.PixelRate;
        } while (0);
    }
    if (g_windowed == true)
        ScreenToClient(m_hFocusWindow, &pos);

    if (!m_texture)
        return OriEndScene(pDevice);

    if (allowGetRenderData == true) {
        allowGetRenderData = false;
        if (g_windowed == true) {
            do // this is not a loop, I do this instead of goto statement
            {
                IDirect3DSurface9* pSurface;
                HRESULT rs = pDevice->GetRenderTarget(0, &pSurface);
                if (rs != D3D_OK) {
                    currentScale = 0.0f;
                    g_currentScale = currentScale;
                    break;
                }
                D3DSURFACE_DESC SurfaceDesc;
                rs = pSurface->GetDesc(&SurfaceDesc);
                if (rs != D3D_OK) {
                    currentScale = 0.0f;
                    g_currentScale = currentScale;
                    break;
                }
                backBufferSize.width = SurfaceDesc.Width;
                backBufferSize.height = SurfaceDesc.Height;
                scalingFactor = D3DXVECTOR2(backBufferSize.width / 640.0f, backBufferSize.height / 480.0f);
                pSurface->Release();

                RECT rect;
                BOOL rs2 = GetClientRect(m_hFocusWindow, &rect);
                if (rs2 == 0) {
                    currentScale = 0.0f;
                    g_currentScale = currentScale;
                    break;
                }
                frontBufferSize.width = rect.right - rect.left;
                frontBufferSize.height = rect.bottom - rect.top;

                currentScale = (float)frontBufferSize.width / backBufferSize.width;
                g_currentScale = currentScale;
            } while (0);
        } else {
            currentScale = 1.0f;
            g_currentScale = currentScale;
        }
    }

    D3DXVECTOR3 position((float)pos.x, (float)pos.y, 0.0);
    if (g_windowed == true && currentScale != 0.0f && currentScale != 1.0f) {
        position /= currentScale;
    }
    D3DXVECTOR3 center(cursorWidthCenter, cursorHeightCenter, 0.0);

    static unsigned char c = 0;
    const unsigned char d = 16;
    const unsigned char whiteMax = 128;
    const unsigned char blackMax = 16;
    enum ColorStage {
        WhiteInc, WhiteDec, BlackInc, BlackDec
    };
    static ColorStage colorStage = WhiteInc;
    static bool white = true;

    switch (colorStage) {
        case WhiteInc:
            if (c == whiteMax) {
                colorStage = WhiteDec;
            } else {
                c += d;
                break;
            }
        case WhiteDec:
            if (c == 0) {
                colorStage = BlackInc;
                white = false;
                c = blackMax;
            } else {
                c -= d;
            }
            break;
        case BlackInc:
            if (c == 0) {
                colorStage = BlackDec;
            } else {
                c -= d;
                break;
            }
        case BlackDec:
            if (c == blackMax) {
                colorStage = WhiteInc;
                white = true;
                c = 0;
            } else {
                c += d;
            }
            break;
    }

    m_sprite->Begin(D3DXSPRITE_ALPHABLEND);
    if (allowGetRenderData == false) {
        D3DXVECTOR2 scalingCenter;
        scalingCenter.x = position.x;
        scalingCenter.y = position.y;
        D3DXMatrixTransformation2D(&spriteMatrix,
            &scalingCenter,
            0,
            &scalingFactor,
            NULL,
            0,
            NULL);
        m_sprite->SetTransform(&spriteMatrix);
    }

    if (g_working) {
        if (white) {
            pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_ADD);
            pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
            pDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

            m_sprite->Draw(m_texture, NULL, &center, &position, D3DCOLOR_RGBA(c, c, c, 255));
        } else {
            m_sprite->Draw(m_texture, NULL, &center, &position, D3DCOLOR_RGBA(c, c, c, 255));
        }
    } else
        m_sprite->Draw(m_texture, NULL, &center, &position, D3DCOLOR_RGBA(255, 200, 200, 128));
    m_sprite->End();

    return OriEndScene(pDevice);
}
