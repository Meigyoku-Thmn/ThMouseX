module;

#include "framework.h"
#include "Include/d3d9.h"
#include "Include/d3dx9core.h"

export module core.directx9hook;

import core.apihijack;
import common.var;
import common.datatype;

// Function pointer types.
typedef IDirect3D9* (WINAPI *Direct3DCreate9_t)(UINT sdk_version);

IDirect3D9* WINAPI MyDirect3DCreate9(UINT sdk_version);

void Create3DDevice(IDirect3D9 *d3d, IDirect3DDevice9 **device, HWND hFocusWindow);

enum {
    D3DFN_Direct3DCreate9 = 0
};
export SDLLHook D3DHook = {
    .Name = "d3d9.DLL",
    .UseDefault = false,
    .DefaultFn = NULL,
    .Functions = {
        {.Name = "Direct3DCreate9", .HookFn = (DWORD*)MyDirect3DCreate9},
        {}
    }
};

class MyDirect3D9: public IDirect3D9 {
public:
    MyDirect3D9(IDirect3D9* d3d): m_d3d(d3d) {}
#pragma region Scaffolding
    /*** IUnknown methods ***/
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj) {
        return m_d3d->QueryInterface(riid, ppvObj);
    }

    ULONG STDMETHODCALLTYPE AddRef() {
        return m_d3d->AddRef();
    }

    ULONG STDMETHODCALLTYPE Release() {
        ULONG count = m_d3d->Release();
        if (0 == count)
            delete this;

        return count;
    }

    /*** IDirect3D9 methods ***/
    HRESULT STDMETHODCALLTYPE RegisterSoftwareDevice(void* pInitializeFunction) {
        return m_d3d->RegisterSoftwareDevice(pInitializeFunction);
    }

    UINT STDMETHODCALLTYPE GetAdapterCount() {
        return m_d3d->GetAdapterCount();
    }

    HRESULT STDMETHODCALLTYPE GetAdapterIdentifier(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9* pIdentifier) {
        return m_d3d->GetAdapterIdentifier(Adapter, Flags, pIdentifier);
    }

    UINT STDMETHODCALLTYPE GetAdapterModeCount(UINT Adapter, D3DFORMAT Format) {
        return m_d3d->GetAdapterModeCount(Adapter, Format);
    }

    HRESULT STDMETHODCALLTYPE EnumAdapterModes(UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE* pMode) {
        return m_d3d->EnumAdapterModes(Adapter, Format, Mode, pMode);
    }

    HRESULT STDMETHODCALLTYPE GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE* pMode) {
        return m_d3d->GetAdapterDisplayMode(Adapter, pMode);
    }

    HRESULT STDMETHODCALLTYPE CheckDeviceType(UINT Adapter, D3DDEVTYPE DevType, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, BOOL bWindowed) {
        return m_d3d->CheckDeviceType(Adapter, DevType, AdapterFormat, BackBufferFormat, bWindowed);
    }

    HRESULT STDMETHODCALLTYPE CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat) {
        return m_d3d->CheckDeviceFormat(Adapter, DeviceType, AdapterFormat, Usage, RType, CheckFormat);
    }

    HRESULT STDMETHODCALLTYPE CheckDeviceMultiSampleType(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD* pQualityLevels) {
        return m_d3d->CheckDeviceMultiSampleType(Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType, pQualityLevels);
    }

    HRESULT STDMETHODCALLTYPE CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat) {
        return m_d3d->CheckDepthStencilMatch(Adapter, DeviceType, AdapterFormat, RenderTargetFormat, DepthStencilFormat);
    }

    HRESULT STDMETHODCALLTYPE CheckDeviceFormatConversion(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SourceFormat, D3DFORMAT TargetFormat) {
        return m_d3d->CheckDeviceFormatConversion(Adapter, DeviceType, SourceFormat, TargetFormat);
    }

    HRESULT STDMETHODCALLTYPE GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9* pCaps) {
        return m_d3d->GetDeviceCaps(Adapter, DeviceType, pCaps);
    }

    HMONITOR STDMETHODCALLTYPE GetAdapterMonitor(UINT Adapter) {
        return m_d3d->GetAdapterMonitor(Adapter);
    }
#pragma endregion
    HRESULT STDMETHODCALLTYPE CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface) {
        auto hr = m_d3d->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags,
            pPresentationParameters, ppReturnedDeviceInterface);
        if (SUCCEEDED(hr))
            Create3DDevice(this, ppReturnedDeviceInterface, hFocusWindow);
        return hr;
    }

private:
    IDirect3D9* m_d3d;
};

class MyDirect3DDevice9: public IDirect3DDevice9 {
public:
    // We need d3d so that we'd use a pointer to MyDirect3D9 instead of the original IDirect3D9 implementor
    // in functions like GetDirect3D9
    MyDirect3DDevice9(IDirect3D9* d3d, IDirect3DDevice9* device, HWND hFocusWindow): m_d3d(d3d), m_device(device), m_hFocusWindow(hFocusWindow) {
        D3DXCreateSprite(device, &m_sprite);
        if (gs_textureFilePath[0]) {
            HRESULT rs = D3DXCreateTextureFromFile(device, gs_textureFilePath, &m_texture);
            if (rs == D3D_OK) {
                D3DSURFACE_DESC desc;
                m_texture->GetLevelDesc(0, &desc);
                this->cursorHeightCenter = (desc.Height - 1) / 2.0f;
                this->cursorWidthCenter = (desc.Width - 1) / 2.0f;
            }
        }
        SystemParametersInfo(SPI_SETCURSORSHADOW, 0, (PVOID)TRUE, SPIF_SENDCHANGE);
        D3DXCreateFont(this, 16, 0, 400, 1, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Courier New", &m_pFont);
    }
#pragma region Scaffolding
    /*** IUnknown methods ***/
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) {
        return m_device->QueryInterface(riid, ppvObj);
    }

    STDMETHOD_(ULONG, AddRef)(THIS) {
        return m_device->AddRef();
    }

    STDMETHOD_(ULONG, Release)(THIS) {
        ULONG count = m_device->Release();
        if (0 == count) {
            m_pFont->Release();
            m_texture->Release();
            m_sprite->Release();
            delete this;
        }

        return count;
    }

    /*** IDirect3DDevice9 methods ***/
    STDMETHOD(TestCooperativeLevel)(THIS) {
        return m_device->TestCooperativeLevel();
    }

    STDMETHOD_(UINT, GetAvailableTextureMem)(THIS) {
        return m_device->GetAvailableTextureMem();
    }

    STDMETHOD(EvictManagedResources)(THIS) {
        return m_device->EvictManagedResources();
    }

    STDMETHOD(GetDirect3D)(THIS_ IDirect3D9** ppD3D9) {
        // Let the device validate the incoming pointer for us
        HRESULT hr = m_device->GetDirect3D(ppD3D9);
        if (SUCCEEDED(hr))
            *ppD3D9 = m_d3d;

        return hr;
    }

    STDMETHOD(GetDeviceCaps)(THIS_ D3DCAPS9* pCaps) {
        return m_device->GetDeviceCaps(pCaps);
    }

    STDMETHOD(GetDisplayMode)(THIS_ UINT iSwapChain, D3DDISPLAYMODE* pMode) {
        return m_device->GetDisplayMode(iSwapChain, pMode);

    }

    STDMETHOD(GetCreationParameters)(THIS_ D3DDEVICE_CREATION_PARAMETERS *pParameters) {
        return m_device->GetCreationParameters(pParameters);
    }

    STDMETHOD(SetCursorProperties)(THIS_ UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9* pCursorBitmap) {
        return m_device->SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap);
    }

    STDMETHOD_(void, SetCursorPosition)(THIS_ int X, int Y, DWORD Flags) {
        m_device->SetCursorPosition(X, Y, Flags);
    }

    STDMETHOD_(BOOL, ShowCursor)(THIS_ BOOL bShow) {
        return m_device->ShowCursor(bShow);
    }

    STDMETHOD(CreateAdditionalSwapChain)(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DSwapChain9** pSwapChain) {
        return m_device->CreateAdditionalSwapChain(pPresentationParameters, pSwapChain);
    }

    STDMETHOD(GetSwapChain)(THIS_ UINT iSwapChain, IDirect3DSwapChain9** pSwapChain) {
        return m_device->GetSwapChain(iSwapChain, pSwapChain);
    }

    STDMETHOD_(UINT, GetNumberOfSwapChains)(THIS) {
        return m_device->GetNumberOfSwapChains();
    }
#pragma endregion
    STDMETHOD(Reset)(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters) {
        if (pPresentationParameters->hDeviceWindow != NULL)
            this->m_hFocusWindow = pPresentationParameters->hDeviceWindow;
        g_hFocusWindow = this->m_hFocusWindow;
        this->allowGetRenderData = true;
        this->init = FALSE;
        return m_device->Reset(pPresentationParameters);
    }
#pragma region Scaffolding
    STDMETHOD(Present)(THIS_ CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) {
        return m_device->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
    }

    STDMETHOD(GetBackBuffer)(THIS_ UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer) {
        return m_device->GetBackBuffer(iSwapChain, iBackBuffer, Type, ppBackBuffer);
    }

    STDMETHOD(GetRasterStatus)(THIS_ UINT iSwapChain, D3DRASTER_STATUS* pRasterStatus) {
        return m_device->GetRasterStatus(iSwapChain, pRasterStatus);
    }

    STDMETHOD(SetDialogBoxMode)(THIS_ BOOL bEnableDialogs) {
        return m_device->SetDialogBoxMode(bEnableDialogs);
    }

    STDMETHOD_(void, SetGammaRamp)(THIS_ UINT iSwapChain, DWORD Flags, CONST D3DGAMMARAMP* pRamp) {
        return m_device->SetGammaRamp(iSwapChain, Flags, pRamp);
    }

    STDMETHOD_(void, GetGammaRamp)(THIS_ UINT iSwapChain, D3DGAMMARAMP* pRamp) {
        return m_device->GetGammaRamp(iSwapChain, pRamp);
    }

    STDMETHOD(CreateTexture)(THIS_ UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle) {
        return m_device->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
    }

    STDMETHOD(CreateVolumeTexture)(THIS_ UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle) {
        return m_device->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, pSharedHandle);
    }

    STDMETHOD(CreateCubeTexture)(THIS_ UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle) {
        return m_device->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pSharedHandle);
    }

    STDMETHOD(CreateVertexBuffer)(THIS_ UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle) {
        return m_device->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle);
    }

    STDMETHOD(CreateIndexBuffer)(THIS_ UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle) {
        return m_device->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle);
    }

    STDMETHOD(CreateRenderTarget)(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) {
        return m_device->CreateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle);
    }

    STDMETHOD(CreateDepthStencilSurface)(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) {
        return m_device->CreateDepthStencilSurface(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle);
    }

    STDMETHOD(UpdateSurface)(THIS_ IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestinationSurface, CONST POINT* pDestPoint) {
        return m_device->UpdateSurface(pSourceSurface, pSourceRect, pDestinationSurface, pDestPoint);
    }

    STDMETHOD(UpdateTexture)(THIS_ IDirect3DBaseTexture9* pSourceTexture, IDirect3DBaseTexture9* pDestinationTexture) {
        return m_device->UpdateTexture(pSourceTexture, pDestinationTexture);
    }

    STDMETHOD(GetRenderTargetData)(THIS_ IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface) {
        return m_device->GetRenderTargetData(pRenderTarget, pDestSurface);
    }

    STDMETHOD(GetFrontBufferData)(THIS_ UINT iSwapChain, IDirect3DSurface9* pDestSurface) {
        return m_device->GetFrontBufferData(iSwapChain, pDestSurface);
    }

    STDMETHOD(StretchRect)(THIS_ IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter) {
        return m_device->StretchRect(pSourceSurface, pSourceRect, pDestSurface, pDestRect, Filter);
    }

    STDMETHOD(ColorFill)(THIS_ IDirect3DSurface9* pSurface, CONST RECT* pRect, D3DCOLOR color) {
        return m_device->ColorFill(pSurface, pRect, color);
    }

    STDMETHOD(CreateOffscreenPlainSurface)(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) {
        return m_device->CreateOffscreenPlainSurface(Width, Height, Format, Pool, ppSurface, pSharedHandle);
    }

    STDMETHOD(SetRenderTarget)(THIS_ DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget) {
        return m_device->SetRenderTarget(RenderTargetIndex, pRenderTarget);
    }

    STDMETHOD(GetRenderTarget)(THIS_ DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget) {
        return m_device->GetRenderTarget(RenderTargetIndex, ppRenderTarget);
    }

    STDMETHOD(SetDepthStencilSurface)(THIS_ IDirect3DSurface9* pNewZStencil) {
        return m_device->SetDepthStencilSurface(pNewZStencil);
    }

    STDMETHOD(GetDepthStencilSurface)(THIS_ IDirect3DSurface9** ppZStencilSurface) {
        return m_device->GetDepthStencilSurface(ppZStencilSurface);
    }

    STDMETHOD(BeginScene)(THIS) {
        return m_device->BeginScene();
    }
#pragma endregion
    STDMETHOD(EndScene)(THIS) {
        POINT pos = {0, 0};
        GetCursorPos(&pos);
        if (this->init == FALSE) {
            g_windowed = TRUE;
            this->init = TRUE;
            do {
                RECT hwndClientRect;
                if (GetClientRect(this->m_hFocusWindow, &hwndClientRect) == 0)
                    break;
                IDirect3DSurface9* pSurface;
                HRESULT rs = m_device->GetRenderTarget(0, &pSurface);
                if (rs != D3D_OK) break;
                pSurface->Release();
                D3DSURFACE_DESC SurfaceDesc;
                rs = pSurface->GetDesc(&SurfaceDesc);
                if (rs != D3D_OK) break;
                if (SurfaceDesc.Width > hwndClientRect.right ||
                    SurfaceDesc.Height > hwndClientRect.bottom) {
                    g_windowed = FALSE; // so-so way to determine "windowed or fullscreen"
                    // clear border to avoid "click-out-of-bound"
                    LONG style = GetWindowLongPtrW(this->m_hFocusWindow, GWL_STYLE);
                    if (style == 0) break;
                    style &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);
                    LONG lExStyle = GetWindowLongPtrW(this->m_hFocusWindow, GWL_EXSTYLE);
                    if (lExStyle == 0) break;
                    lExStyle &= ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
                    if (SetWindowLongPtrW(this->m_hFocusWindow, GWL_STYLE, style) == 0) break;
                    if (SetWindowLongPtrW(this->m_hFocusWindow, GWL_EXSTYLE, lExStyle) == 0) break;
                    SetWindowPos(this->m_hFocusWindow, NULL, 0, 0,
                        SurfaceDesc.Width, SurfaceDesc.Height,
                        SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOOWNERZORDER);
                }
                if (GetClientRect(this->m_hFocusWindow, &hwndClientRect) == 0)
                    break;
                g_currentGameConfig.PixelRate = (float)g_currentGameConfig.BaseResolutionX / hwndClientRect.right;
                g_currentGameConfig.PixelOffset.X = g_currentGameConfig.BasePixelOffset.X / g_currentGameConfig.PixelRate;
                g_currentGameConfig.PixelOffset.Y = g_currentGameConfig.BasePixelOffset.Y / g_currentGameConfig.PixelRate;
            } while (0);
        }
        if (g_windowed == TRUE)
            ScreenToClient(this->m_hFocusWindow, &pos);

        if (!m_texture)
            return m_device->EndScene();

        if (this->allowGetRenderData == true) {
            this->allowGetRenderData = false;
            if (g_windowed == TRUE) {
                do // this is not a loop, I do this instead of goto statement
                {
                    IDirect3DSurface9* pSurface;
                    HRESULT rs = m_device->GetRenderTarget(0, &pSurface);
                    if (rs != D3D_OK) {
                        this->currentScale = 0.0f;
                        g_currentScale = this->currentScale;
                        break;
                    }
                    D3DSURFACE_DESC SurfaceDesc;
                    rs = pSurface->GetDesc(&SurfaceDesc);
                    if (rs != D3D_OK) {
                        this->currentScale = 0.0f;
                        g_currentScale = this->currentScale;
                        break;
                    }
                    backBufferSize.width = SurfaceDesc.Width;
                    backBufferSize.height = SurfaceDesc.Height;
                    this->scalingFactor = D3DXVECTOR2(backBufferSize.width / 640.0f, backBufferSize.height / 480.0f);
                    pSurface->Release();

                    RECT rect;
                    BOOL rs2 = GetClientRect(this->m_hFocusWindow, &rect);
                    if (rs2 == 0) {
                        this->currentScale = 0.0f;
                        g_currentScale = this->currentScale;
                        break;
                    }
                    frontBufferSize.width = rect.right - rect.left;
                    frontBufferSize.height = rect.bottom - rect.top;

                    this->currentScale = (float)frontBufferSize.width / backBufferSize.width;
                    g_currentScale = this->currentScale;
                } while (0);
            } else {
                this->currentScale = 1.0f;
                g_currentScale = this->currentScale;
            }
        }

        D3DXVECTOR3 position((float)pos.x, (float)pos.y, 0.0);
        if (g_windowed == TRUE && this->currentScale != 0.0f && this->currentScale != 1.0f) {
            position /= this->currentScale;
        }
        D3DXVECTOR3 center(this->cursorWidthCenter, this->cursorHeightCenter, 0.0);

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
        if (this->allowGetRenderData == false) {
            D3DXVECTOR2 scalingCenter;
            scalingCenter.x = position.x;
            scalingCenter.y = position.y;
            D3DXMatrixTransformation2D(&this->spriteMatrix,
                &scalingCenter,
                0,
                &scalingFactor,
                NULL,
                0,
                NULL);
            m_sprite->SetTransform(&this->spriteMatrix);
        }

        if (g_working) {
            if (white) {
                m_device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_ADD);
                m_device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
                m_device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

                m_sprite->Draw(m_texture, NULL, &center, &position, D3DCOLOR_RGBA(c, c, c, 255));
            } else {
                m_sprite->Draw(m_texture, NULL, &center, &position, D3DCOLOR_RGBA(c, c, c, 255));
            }
        } else
            m_sprite->Draw(m_texture, NULL, &center, &position, D3DCOLOR_RGBA(255, 200, 200, 128));
        m_sprite->End();

        return m_device->EndScene();
    }
#pragma region Scaffolding
    STDMETHOD(Clear)(THIS_ DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil) {
        return m_device->Clear(Count, pRects, Flags, Color, Z, Stencil);
    }

    STDMETHOD(SetTransform)(THIS_ D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix) {
        return m_device->SetTransform(State, pMatrix);
    }

    STDMETHOD(GetTransform)(THIS_ D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix) {
        return m_device->GetTransform(State, pMatrix);
    }

    STDMETHOD(MultiplyTransform)(THIS_ D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix) {
        return m_device->MultiplyTransform(State, pMatrix);
    }

    STDMETHOD(SetViewport)(THIS_ CONST D3DVIEWPORT9* pViewport) {
        return m_device->SetViewport(pViewport);
    }

    STDMETHOD(GetViewport)(THIS_ D3DVIEWPORT9* pViewport) {
        return m_device->GetViewport(pViewport);
    }

    STDMETHOD(SetMaterial)(THIS_ CONST D3DMATERIAL9* pMaterial) {
        return m_device->SetMaterial(pMaterial);
    }

    STDMETHOD(GetMaterial)(THIS_ D3DMATERIAL9* pMaterial) {
        return m_device->GetMaterial(pMaterial);
    }

    STDMETHOD(SetLight)(THIS_ DWORD Index, CONST D3DLIGHT9* pLight) {
        return m_device->SetLight(Index, pLight);
    }

    STDMETHOD(GetLight)(THIS_ DWORD Index, D3DLIGHT9* pLight) {
        return m_device->GetLight(Index, pLight);
    }

    STDMETHOD(LightEnable)(THIS_ DWORD Index, BOOL Enable) {
        return m_device->LightEnable(Index, Enable);
    }

    STDMETHOD(GetLightEnable)(THIS_ DWORD Index, BOOL* pEnable) {
        return m_device->GetLightEnable(Index, pEnable);
    }

    STDMETHOD(SetClipPlane)(THIS_ DWORD Index, CONST float* pPlane) {
        return m_device->SetClipPlane(Index, pPlane);
    }

    STDMETHOD(GetClipPlane)(THIS_ DWORD Index, float* pPlane) {
        return m_device->GetClipPlane(Index, pPlane);
    }

    STDMETHOD(SetRenderState)(THIS_ D3DRENDERSTATETYPE State, DWORD Value) {
        return m_device->SetRenderState(State, Value);
    }

    STDMETHOD(GetRenderState)(THIS_ D3DRENDERSTATETYPE State, DWORD* pValue) {
        return m_device->GetRenderState(State, pValue);
    }

    STDMETHOD(CreateStateBlock)(THIS_ D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB) {
        return m_device->CreateStateBlock(Type, ppSB);
    }

    STDMETHOD(BeginStateBlock)(THIS) {
        return m_device->BeginStateBlock();
    }

    STDMETHOD(EndStateBlock)(THIS_ IDirect3DStateBlock9** ppSB) {
        return m_device->EndStateBlock(ppSB);
    }

    STDMETHOD(SetClipStatus)(THIS_ CONST D3DCLIPSTATUS9* pClipStatus) {
        return m_device->SetClipStatus(pClipStatus);
    }

    STDMETHOD(GetClipStatus)(THIS_ D3DCLIPSTATUS9* pClipStatus) {
        return m_device->GetClipStatus(pClipStatus);
    }

    STDMETHOD(GetTexture)(THIS_ DWORD Stage, IDirect3DBaseTexture9** ppTexture) {
        return m_device->GetTexture(Stage, ppTexture);
    }

    STDMETHOD(SetTexture)(THIS_ DWORD Stage, IDirect3DBaseTexture9* pTexture) {
        return m_device->SetTexture(Stage, pTexture);
    }

    STDMETHOD(GetTextureStageState)(THIS_ DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue) {
        return m_device->GetTextureStageState(Stage, Type, pValue);
    }

    STDMETHOD(SetTextureStageState)(THIS_ DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value) {
        return m_device->SetTextureStageState(Stage, Type, Value);
    }

    STDMETHOD(GetSamplerState)(THIS_ DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue) {
        return m_device->GetSamplerState(Sampler, Type, pValue);
    }

    STDMETHOD(SetSamplerState)(THIS_ DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value) {
        return m_device->SetSamplerState(Sampler, Type, Value);
    }

    STDMETHOD(ValidateDevice)(THIS_ DWORD* pNumPasses) {
        return m_device->ValidateDevice(pNumPasses);
    }

    STDMETHOD(SetPaletteEntries)(THIS_ UINT PaletteNumber, CONST PALETTEENTRY* pEntries) {
        return m_device->SetPaletteEntries(PaletteNumber, pEntries);
    }

    STDMETHOD(GetPaletteEntries)(THIS_ UINT PaletteNumber, PALETTEENTRY* pEntries) {
        return m_device->GetPaletteEntries(PaletteNumber, pEntries);
    }

    STDMETHOD(SetCurrentTexturePalette)(THIS_ UINT PaletteNumber) {
        return m_device->SetCurrentTexturePalette(PaletteNumber);
    }

    STDMETHOD(GetCurrentTexturePalette)(THIS_ UINT *PaletteNumber) {
        return m_device->GetCurrentTexturePalette(PaletteNumber);
    }

    STDMETHOD(SetScissorRect)(THIS_ CONST RECT* pRect) {
        return m_device->SetScissorRect(pRect);
    }

    STDMETHOD(GetScissorRect)(THIS_ RECT* pRect) {
        return m_device->GetScissorRect(pRect);
    }

    STDMETHOD(SetSoftwareVertexProcessing)(THIS_ BOOL bSoftware) {
        return m_device->SetSoftwareVertexProcessing(bSoftware);
    }

    STDMETHOD_(BOOL, GetSoftwareVertexProcessing)(THIS) {
        return m_device->GetSoftwareVertexProcessing();
    }

    STDMETHOD(SetNPatchMode)(THIS_ float nSegments) {
        return m_device->SetNPatchMode(nSegments);
    }

    STDMETHOD_(float, GetNPatchMode)(THIS) {
        return m_device->GetNPatchMode();
    }

    STDMETHOD(DrawPrimitive)(THIS_ D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount) {
        return m_device->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
    }

    STDMETHOD(DrawIndexedPrimitive)(THIS_ D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount) {
        return m_device->DrawIndexedPrimitive(PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
    }

    STDMETHOD(DrawPrimitiveUP)(THIS_ D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) {
        return m_device->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
    }

    STDMETHOD(DrawIndexedPrimitiveUP)(THIS_ D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) {
        return m_device->DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride);
    }

    STDMETHOD(ProcessVertices)(THIS_ UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer9* pDestBuffer, IDirect3DVertexDeclaration9* pVertexDecl, DWORD Flags) {
        return m_device->ProcessVertices(SrcStartIndex, DestIndex, VertexCount, pDestBuffer, pVertexDecl, Flags);
    }

    STDMETHOD(CreateVertexDeclaration)(THIS_ CONST D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl) {
        return m_device->CreateVertexDeclaration(pVertexElements, ppDecl);
    }

    STDMETHOD(SetVertexDeclaration)(THIS_ IDirect3DVertexDeclaration9* pDecl) {
        return m_device->SetVertexDeclaration(pDecl);
    }

    STDMETHOD(GetVertexDeclaration)(THIS_ IDirect3DVertexDeclaration9** ppDecl) {
        return m_device->GetVertexDeclaration(ppDecl);
    }

    STDMETHOD(SetFVF)(THIS_ DWORD FVF) {
        return m_device->SetFVF(FVF);
    }

    STDMETHOD(GetFVF)(THIS_ DWORD* pFVF) {
        return m_device->GetFVF(pFVF);
    }

    STDMETHOD(CreateVertexShader)(THIS_ CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader) {
        return m_device->CreateVertexShader(pFunction, ppShader);
    }

    STDMETHOD(SetVertexShader)(THIS_ IDirect3DVertexShader9* pShader) {
        return m_device->SetVertexShader(pShader);
    }

    STDMETHOD(GetVertexShader)(THIS_ IDirect3DVertexShader9** ppShader) {
        return m_device->GetVertexShader(ppShader);
    }

    STDMETHOD(SetVertexShaderConstantF)(THIS_ UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount) {
        return m_device->SetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
    }

    STDMETHOD(GetVertexShaderConstantF)(THIS_ UINT StartRegister, float* pConstantData, UINT Vector4fCount) {
        return m_device->GetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
    }

    STDMETHOD(SetVertexShaderConstantI)(THIS_ UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount) {
        return m_device->SetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
    }

    STDMETHOD(GetVertexShaderConstantI)(THIS_ UINT StartRegister, int* pConstantData, UINT Vector4iCount) {
        return m_device->GetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
    }

    STDMETHOD(SetVertexShaderConstantB)(THIS_ UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount) {
        return m_device->SetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
    }

    STDMETHOD(GetVertexShaderConstantB)(THIS_ UINT StartRegister, BOOL* pConstantData, UINT BoolCount) {
        return m_device->GetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
    }

    STDMETHOD(SetStreamSource)(THIS_ UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride) {
        return m_device->SetStreamSource(StreamNumber, pStreamData, OffsetInBytes, Stride);
    }

    STDMETHOD(GetStreamSource)(THIS_ UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData, UINT* pOffsetInBytes, UINT* pStride) {
        return m_device->GetStreamSource(StreamNumber, ppStreamData, pOffsetInBytes, pStride);
    }

    STDMETHOD(SetStreamSourceFreq)(THIS_ UINT StreamNumber, UINT Setting) {
        return m_device->SetStreamSourceFreq(StreamNumber, Setting);
    }

    STDMETHOD(GetStreamSourceFreq)(THIS_ UINT StreamNumber, UINT* pSetting) {
        return m_device->GetStreamSourceFreq(StreamNumber, pSetting);
    }

    STDMETHOD(SetIndices)(THIS_ IDirect3DIndexBuffer9* pIndexData) {
        return m_device->SetIndices(pIndexData);
    }

    STDMETHOD(GetIndices)(THIS_ IDirect3DIndexBuffer9** ppIndexData) {
        return m_device->GetIndices(ppIndexData);
    }

    STDMETHOD(CreatePixelShader)(THIS_ CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader) {
        return m_device->CreatePixelShader(pFunction, ppShader);
    }

    STDMETHOD(SetPixelShader)(THIS_ IDirect3DPixelShader9* pShader) {
        return m_device->SetPixelShader(pShader);
    }

    STDMETHOD(GetPixelShader)(THIS_ IDirect3DPixelShader9** ppShader) {
        return m_device->GetPixelShader(ppShader);
    }

    STDMETHOD(SetPixelShaderConstantF)(THIS_ UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount) {
        return m_device->SetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
    }

    STDMETHOD(GetPixelShaderConstantF)(THIS_ UINT StartRegister, float* pConstantData, UINT Vector4fCount) {
        return m_device->GetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
    }

    STDMETHOD(SetPixelShaderConstantI)(THIS_ UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount) {
        return m_device->SetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
    }

    STDMETHOD(GetPixelShaderConstantI)(THIS_ UINT StartRegister, int* pConstantData, UINT Vector4iCount) {
        return m_device->GetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
    }

    STDMETHOD(SetPixelShaderConstantB)(THIS_ UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount) {
        return m_device->SetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
    }

    STDMETHOD(GetPixelShaderConstantB)(THIS_ UINT StartRegister, BOOL* pConstantData, UINT BoolCount) {
        return m_device->GetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
    }

    STDMETHOD(DrawRectPatch)(THIS_ UINT Handle, CONST float* pNumSegs, CONST D3DRECTPATCH_INFO* pRectPatchInfo) {
        return m_device->DrawRectPatch(Handle, pNumSegs, pRectPatchInfo);
    }

    STDMETHOD(DrawTriPatch)(THIS_ UINT Handle, CONST float* pNumSegs, CONST D3DTRIPATCH_INFO* pTriPatchInfo) {
        return m_device->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo);
    }

    STDMETHOD(DeletePatch)(THIS_ UINT Handle) {
        return m_device->DeletePatch(Handle);
    }

    STDMETHOD(CreateQuery)(THIS_ D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery) {
        return m_device->CreateQuery(Type, ppQuery);
    }
#pragma endregion

private:
    BOOL init = FALSE;
    IDirect3DDevice9* m_device;
    IDirect3D9* m_d3d;
    HWND m_hFocusWindow;
    BOOL allowGetRenderData = true;
    UINTSIZE backBufferSize;
    UINTSIZE frontBufferSize;
    FLOAT currentScale = 1.0f;
    FLOAT cursorWidthCenter = 0;
    FLOAT cursorHeightCenter = 0;
    D3DXMATRIX spriteMatrix;
    D3DXVECTOR2 scalingFactor;
    LPD3DXFONT m_pFont;
    LPD3DXSPRITE m_sprite;
    LPDIRECT3DTEXTURE9 m_texture;
};

IDirect3D9* WINAPI MyDirect3DCreate9(UINT sdk_version) {
    auto old_func = (Direct3DCreate9_t)D3DHook.Functions[D3DFN_Direct3DCreate9].OrigFn;
    auto d3d = old_func(sdk_version);
    return d3d ? new MyDirect3D9(d3d) : 0;
}

void Create3DDevice(IDirect3D9 *d3d, IDirect3DDevice9 **device, HWND hFocusWindow) {
    *device = new MyDirect3DDevice9(d3d, *device, hFocusWindow);
    g_hFocusWindow = hFocusWindow;
}