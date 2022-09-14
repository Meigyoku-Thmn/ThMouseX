module;

#include "framework.h"
#include "macro.h"
#include <stdio.h>
#include "d3d8.h"
#include "d3dx8core.h"
#include <string>
#include <vector>

export module dx8.hook;

import common.datatype;
import common.var;
import common.helper;
import common.minhook;

int (WINAPIV *__vsnprintf)(char*, size_t, const char*, va_list) = _vsnprintf;
int (WINAPIV *__snprintf)(char*, size_t, const char*, ...) = _snprintf;
int (WINAPIV *_sscanf)(const char*, const char*, ...) = sscanf;
int (WINAPIV *_sprintf)(char*, const char*, ...) = sprintf;

using namespace std;

constexpr auto ResetIdx = 14;
constexpr auto EndSceneIdx = 35;
constexpr auto ErrorMessageTitle = "D3D8 Hook Setup Error";

HRESULT WINAPI MyReset(IDirect3DDevice8 *pDevice, D3DPRESENT_PARAMETERS *pPresentationParameters);
decltype(&MyReset) OriReset;
HRESULT WINAPI MyEndScene(IDirect3DDevice8 *pDevice);
decltype(&MyEndScene) OriEndScene;

constexpr const char* GetD3dErrStr(int errorCode) {
    if (errorCode == D3DERR_INVALIDCALL)
        return "D3DERR_INVALIDCALL";
    if (errorCode == D3DERR_NOTAVAILABLE)
        return "D3DERR_NOTAVAILABLE";
    if (errorCode == D3DERR_OUTOFVIDEOMEMORY)
        return "D3DERR_OUTOFVIDEOMEMORY";
    return "Unknown error.";
}

export DLLEXPORT bool PopulateD3D8MethodRVAs() {
    bool result = false;
    DWORD *vtable{};
    HRESULT rs{};
    IDirect3D8 *pD3D{};
    IDirect3DDevice8 *pDevice{};
    D3DPRESENT_PARAMETERS d3dpp{};
    DWORD baseAddress{};

    auto tmpWnd = CreateWindowA("BUTTON", "Temp Window", WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 300, 300, NULL, NULL, NULL, NULL);
    if (tmpWnd == NULL) {
        ReportLastError(ErrorMessageTitle);
        goto CleanAndReturn;
    }

    pD3D = Direct3DCreate8(D3D_SDK_VERSION);
    if (!pD3D) {
        MessageBox(NULL, "Failed to create an IDirect3D8 instance.", ErrorMessageTitle, MB_OK | MB_ICONERROR);
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
        MessageBox(NULL, (string("Failed to create an IDirect3DDevice8 instance:") + GetD3dErrStr(rs)).c_str(), ErrorMessageTitle, MB_OK | MB_ICONERROR);
        goto CleanAndReturn;
    }

    vtable = *(DWORD**)pDevice;
    baseAddress = (DWORD)GetModuleHandleA("d3d8.dll");

    gs_d3d8_Reset_RVA = vtable[ResetIdx] - baseAddress;
    gs_d3d8_EndScene_RVA = vtable[EndSceneIdx] - baseAddress;

    result = true;
CleanAndReturn:
    pDevice && pDevice->Release();
    pD3D && pD3D->Release();
    tmpWnd && DestroyWindow(tmpWnd);
    return result;
}

export DLLEXPORT vector<MHookConfig> D3D8HookConfig() {
    auto baseAddress = (DWORD)GetModuleHandleA("d3d8.dll");
    return {
        {PVOID(baseAddress + gs_d3d8_Reset_RVA), &MyReset, (PVOID*)&OriReset},
        {PVOID(baseAddress + gs_d3d8_EndScene_RVA), &MyEndScene, (PVOID*)&OriEndScene},
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
LPD3DXSPRITE m_sprite;
LPDIRECT3DTEXTURE8 m_texture;

void Initialize(IDirect3DDevice8 *device) {
    if (initialized)
        return;
    initialized = true;
    D3DDEVICE_CREATION_PARAMETERS params;
    device->GetCreationParameters(&params);
    m_hFocusWindow = g_hFocusWindow = params.hFocusWindow;
    D3DXCreateSprite(device, &m_sprite);
    if (gs_textureFilePath2[0]) {
        auto rs = D3DXCreateTextureFromFile(device, gs_textureFilePath2, &m_texture);
        if (rs == D3D_OK) {
            D3DSURFACE_DESC desc;
            m_texture->GetLevelDesc(0, &desc);
            cursorHeightCenter = (desc.Height - 1) / 2.0f;
            cursorWidthCenter = (desc.Width - 1) / 2.0f;
        }
    }
    SystemParametersInfo(SPI_SETCURSORSHADOW, 0, (PVOID)TRUE, SPIF_SENDCHANGE);
}

HRESULT WINAPI MyReset(IDirect3DDevice8 *pDevice, D3DPRESENT_PARAMETERS *pPresentationParameters) {
    Initialize(pDevice);
    if (pPresentationParameters->hDeviceWindow != NULL)
        m_hFocusWindow = pPresentationParameters->hDeviceWindow;
    g_hFocusWindow2 = m_hFocusWindow;
    allowGetRenderData = true;
    prepared = false;
    return OriReset(pDevice, pPresentationParameters);
}

HRESULT WINAPI MyEndScene(IDirect3DDevice8 *pDevice) {
    Initialize(pDevice);
    POINT mousePos{};
    GetCursorPos(&mousePos);
    if (prepared == false) {
        g_windowed2 = true;
        prepared = true;
        do {
            RECT hwndClientRect;
            if (GetClientRect(m_hFocusWindow, &hwndClientRect) == 0)
                break;
            IDirect3DSurface8* pSurface;
            auto rs = pDevice->GetRenderTarget(&pSurface);
            if (rs != D3D_OK)
                break;
            D3DSURFACE_DESC SurfaceDesc;
            rs = pSurface->GetDesc(&SurfaceDesc);
            pSurface->Release();
            if (rs != D3D_OK)
                break;
            // guess "windowed or fullscreen"
            if (SurfaceDesc.Width > hwndClientRect.right || SurfaceDesc.Height > hwndClientRect.bottom) {
                g_windowed2 = false;
                // clear border to avoid "click-out-of-bound"
                auto style = GetWindowLongPtrW(m_hFocusWindow, GWL_STYLE);
                if (style == 0)
                    break;
                style &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);
                auto lExStyle = GetWindowLongPtrW(m_hFocusWindow, GWL_EXSTYLE);
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
            *g_pixelRate = (float)g_baseResolutionX / hwndClientRect.right;
            g_pixelOffset->X = g_basePixelOffset->X / (*g_pixelRate);
            g_pixelOffset->Y = g_basePixelOffset->Y / (*g_pixelRate);
        } while (0);
    }
    if (g_windowed2 == true)
        ScreenToClient(m_hFocusWindow, &mousePos);

    if (!m_texture)
        return OriEndScene(pDevice);

    if (allowGetRenderData == true) {
        allowGetRenderData = false;
        if (g_windowed2 == true) {
            do // this is not a loop, I do this instead of goto statement
            {
                IDirect3DSurface8* pSurface;
                auto rs = pDevice->GetRenderTarget(&pSurface);
                if (rs != D3D_OK) {
                    currentScale = 1.0f;
                    g_currentScale2 = currentScale;
                    break;
                }
                D3DSURFACE_DESC SurfaceDesc;
                rs = pSurface->GetDesc(&SurfaceDesc);
                if (rs != D3D_OK) {
                    currentScale = 1.0f;
                    g_currentScale2 = currentScale;
                    break;
                }
                backBufferSize.width = SurfaceDesc.Width;
                backBufferSize.height = SurfaceDesc.Height;
                pSurface->Release();

                RECT rect;
                BOOL rs2 = GetClientRect(m_hFocusWindow, &rect);
                if (rs2 == 0) {
                    currentScale = 1.0f;
                    g_currentScale2 = currentScale;
                    break;
                }
                frontBufferSize.width = rect.right - rect.left;
                frontBufferSize.height = rect.bottom - rect.top;

                currentScale = (float)frontBufferSize.width / backBufferSize.width;
                g_currentScale2 = currentScale;
            } while (0);
        } else {
            currentScale = 1.0f;
        }
    }

    D3DXVECTOR2 position((float)(mousePos.x), (float)(mousePos.y));
    if (g_windowed2 == true && currentScale != 0.0f && currentScale != 1.0f) {
        position /= currentScale;
    }
    position.x -= cursorWidthCenter;
    position.y -= cursorHeightCenter;

    D3DXVECTOR2 center(0, 0);
    //D3DXVECTOR2 center(63.5, 63.5); // does not work, i don't know why

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

    m_sprite->Begin();

    if (g_working2) {
        if (white) {
            pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_ADD);
            pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
            pDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

            m_sprite->Draw(m_texture, NULL, NULL, &center, 0, &position, D3DCOLOR_RGBA(c, c, c, 255));
        } else {
            m_sprite->Draw(m_texture, NULL, NULL, &center, 0, &position, D3DCOLOR_RGBA(c, c, c, 255));
        }
    } else
        m_sprite->Draw(m_texture, NULL, NULL, &center, 0, &position, D3DCOLOR_RGBA(255, 200, 200, 128));
    m_sprite->End();

    return OriEndScene(pDevice);
}
