#pragma once

#include "DX8.h"
#include "subGlobal.h"

// #define DEBUG_MESSAGE

#define VERTEX_COUNT 12
#define CURSOR_SIZE1 3.5f
#define CURSOR_SIZE2 16.5f
#define CURSOR_SIZE3 0.5f
#define COLOR_COUNT 32

typedef struct tagTLVertex {
    float x, y, z, rhw;
    DWORD color;
} TLVertex;

class MyDirect3DDevice8: public IDirect3DDevice8 {
public:
    // We need d3d so that we'd use a pointer to MyDirect3D9 instead of the original IDirect3D9 implementor
    // in functions like GetDirect3D9
    MyDirect3DDevice8(IDirect3D8* d3d, IDirect3DDevice8* device, HWND hFocusWindow): m_d3d(d3d), m_device(device), m_hFocusWindow(hFocusWindow) {
        D3DXCreateSprite(device, &m_sprite);
        if (gs_textureFilePath2[0]) {
            HRESULT rs = D3DXCreateTextureFromFile(device, gs_textureFilePath2, &m_texture);
            if (rs == D3D_OK) {
                D3DSURFACE_DESC desc;
                m_texture->GetLevelDesc(0, &desc);
                this->cursorHeightCenter = (desc.Height - 1) / 2.0f;
                this->cursorWidthCenter = (desc.Width - 1) / 2.0f;
            }
        }
        SystemParametersInfo(SPI_SETCURSORSHADOW, 0, (PVOID)TRUE, SPIF_SENDCHANGE);

#ifdef DEBUG_MESSAGE
        D3DXCreateFont(this, 16, 0, 400, 1, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Courier New", &m_pFont);
#endif
    }

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
#ifdef DEBUG_MESSAGE
            m_pFont->Release();
#endif
            m_texture->Release();
            m_sprite->Release();
            delete this;
        }

        return count;
    }

    /*** IDirect3DDevice9 methods ***/
    STDMETHOD(ResourceManagerDiscardBytes)(THIS_ DWORD Bytes) {
        return m_device->ResourceManagerDiscardBytes(Bytes);
    }

    STDMETHOD_(void, SetCursorPosition)(THIS_ UINT XScreenSpace, UINT YScreenSpace, DWORD Flags) {
        return m_device->SetCursorPosition(XScreenSpace, YScreenSpace, Flags);
    }

    STDMETHOD(CreateImageSurface)(THIS_ UINT Width, UINT Height, D3DFORMAT Format, IDirect3DSurface8** ppSurface) {
        return m_device->CreateImageSurface(Width, Height, Format, ppSurface);
    }

    STDMETHOD(TestCooperativeLevel)(THIS) {
        return m_device->TestCooperativeLevel();
    }

    STDMETHOD_(UINT, GetAvailableTextureMem)(THIS) {
        return m_device->GetAvailableTextureMem();
    }

    STDMETHOD(GetDirect3D)(THIS_ IDirect3D8** ppD3D9) {
        // Let the device validate the incoming pointer for us
        HRESULT hr = m_device->GetDirect3D(ppD3D9);
        if (SUCCEEDED(hr))
            *ppD3D9 = m_d3d;

        return hr;
    }

    STDMETHOD(GetDeviceCaps)(THIS_ D3DCAPS8* pCaps) {
        return m_device->GetDeviceCaps(pCaps);
    }

    STDMETHOD(GetDisplayMode)(THIS_ D3DDISPLAYMODE* pMode) {
        return m_device->GetDisplayMode(pMode);

    }

    STDMETHOD(GetCreationParameters)(THIS_ D3DDEVICE_CREATION_PARAMETERS *pParameters) {
        return m_device->GetCreationParameters(pParameters);
    }

    STDMETHOD(SetCursorProperties)(THIS_ UINT XHotSpot, UINT YHotSpot, IDirect3DSurface8* pCursorBitmap) {
        return m_device->SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap);
    }

    STDMETHOD_(void, SetCursorPosition)(THIS_ int X, int Y, DWORD Flags) {
        m_device->SetCursorPosition(X, Y, Flags);
    }

    STDMETHOD_(BOOL, ShowCursor)(THIS_ BOOL bShow) {
        return m_device->ShowCursor(bShow);
    }

    STDMETHOD(CreateAdditionalSwapChain)(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DSwapChain8** pSwapChain) {
        return m_device->CreateAdditionalSwapChain(pPresentationParameters, pSwapChain);
    }

    STDMETHOD(Reset)(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters) {
        if (pPresentationParameters->hDeviceWindow != NULL)
            this->m_hFocusWindow = pPresentationParameters->hDeviceWindow;
        g_hFocusWindow2 = this->m_hFocusWindow;
        this->allowGetRenderData = true;
        this->init = FALSE;
        return m_device->Reset(pPresentationParameters);
    }

    STDMETHOD(Present)(THIS_ CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) {
        return m_device->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
    }

    STDMETHOD(GetBackBuffer)(THIS_ UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface8** ppBackBuffer) {
        return m_device->GetBackBuffer(iBackBuffer, Type, ppBackBuffer);
    }

    STDMETHOD(GetRasterStatus)(THIS_ D3DRASTER_STATUS* pRasterStatus) {
        return m_device->GetRasterStatus(pRasterStatus);
    }

    STDMETHOD_(void, SetGammaRamp)(THIS_ DWORD Flags, CONST D3DGAMMARAMP* pRamp) {
        return m_device->SetGammaRamp(Flags, pRamp);
    }

    STDMETHOD_(void, GetGammaRamp)(THIS_ D3DGAMMARAMP* pRamp) {
        return m_device->GetGammaRamp(pRamp);
    }

    STDMETHOD(CreateTexture)(THIS_ UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture8** ppTexture) {
        return m_device->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture);
    }

    STDMETHOD(CreateVolumeTexture)(THIS_ UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture8** ppVolumeTexture) {
        return m_device->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture);
    }

    STDMETHOD(CreateCubeTexture)(THIS_ UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture8** ppCubeTexture) {
        return m_device->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture);
    }

    STDMETHOD(CreateVertexBuffer)(THIS_ UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer8** ppVertexBuffer) {
        return m_device->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer);
    }

    STDMETHOD(CreateIndexBuffer)(THIS_ UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer8** ppIndexBuffer) {
        return m_device->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer);
    }

    STDMETHOD(CreateRenderTarget)(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, BOOL Lockable, IDirect3DSurface8** ppSurface) {
        return m_device->CreateRenderTarget(Width, Height, Format, MultiSample, Lockable, ppSurface);
    }

    STDMETHOD(CreateDepthStencilSurface)(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, IDirect3DSurface8** ppSurface) {
        return m_device->CreateDepthStencilSurface(Width, Height, Format, MultiSample, ppSurface);
    }

    STDMETHOD(UpdateTexture)(THIS_ IDirect3DBaseTexture8* pSourceTexture, IDirect3DBaseTexture8* pDestinationTexture) {
        return m_device->UpdateTexture(pSourceTexture, pDestinationTexture);
    }

    STDMETHOD(SetRenderTarget)(THIS_ IDirect3DSurface8* pRenderTarget, IDirect3DSurface8* pNewZStencil) {
        return m_device->SetRenderTarget(pRenderTarget, pNewZStencil);
    }

    STDMETHOD(GetRenderTarget)(THIS_ IDirect3DSurface8** ppRenderTarget) {
        return m_device->GetRenderTarget(ppRenderTarget);
    }

    STDMETHOD(GetDepthStencilSurface)(THIS_ IDirect3DSurface8** ppZStencilSurface) {
        return m_device->GetDepthStencilSurface(ppZStencilSurface);
    }

    STDMETHOD(BeginScene)(THIS) {
        return m_device->BeginScene();
    }

    STDMETHOD(EndScene)(THIS) {
#ifdef DEBUG_MESSAGE
        char str[64];
        RECT rec;
        DWORD address = g_currentGameConfig.Posistion.Chain[0];
        sprintf_s(str, sizeof(str), "0x%08X", address);
        rec.left = 0; rec.top = 0; rec.right = 200; rec.bottom = 16;
        m_pFont->DrawTextA(0, str, 10, &rec, DT_LEFT, 0xff00ff00);
        for (int i = 1; i < g_currentGameConfig.Posistion.Length; i++) {
            address = *((DWORD*)address);
            sprintf_s(str, sizeof(str), "0x%08X + 0x%08X", address, g_currentGameConfig.Posistion.Chain[i]);
            rec.left = 0; rec.top = i * 16; rec.right = 200; rec.bottom = (i + 1) * 16;
            m_pFont->DrawTextA(0, str, 23, &rec, DT_LEFT, 0xffffff00);
            if (address == 0)
                break;
            address += g_currentGameConfig.Posistion.Chain[i];
        }
        if (address != 0) {
            LPPOINT position = (LPPOINT)(address);
            sprintf_s(str, sizeof(str), "(%8d,%8d)", position->x, position->y);
            RECT rec1 = {0, 64, 200, 80};

            m_pFont->DrawTextA(0, str, 19, &rec1, DT_LEFT, 0xffffffff);

        }
#endif
        POINT pos = {0, 0};
        GetCursorPos(&pos);
        if (this->init == FALSE) {
            windowed2 = TRUE;
            this->init = TRUE;
            do {
                RECT hwndClientRect;
                if (GetClientRect(this->m_hFocusWindow, &hwndClientRect) == 0)
                    break;
                IDirect3DSurface8* pSurface;
                HRESULT rs = m_device->GetRenderTarget(&pSurface);
                if (rs != D3D_OK) break;
                D3DSURFACE_DESC SurfaceDesc;
                rs = pSurface->GetDesc(&SurfaceDesc);
                pSurface->Release();
                if (rs != D3D_OK) break;
                if (SurfaceDesc.Width > hwndClientRect.right ||
                    SurfaceDesc.Height > hwndClientRect.bottom) {
                    windowed2 = FALSE; // so-so way to determine "windowed or fullscreen"
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
                *pixelRate = (float)baseResolutionX / hwndClientRect.right;
                pixelOffset->X = basePixelOffset->X / (*pixelRate);
                pixelOffset->Y = basePixelOffset->Y / (*pixelRate);
            } while (0);
        }
        if (windowed2 == TRUE)
            ScreenToClient(this->m_hFocusWindow, &pos);

        if (!m_texture)
            return m_device->EndScene();

#ifdef DEBUG_MESSAGE
        {
            sprintf_s(str, sizeof(str), "(%8d,%8d)", pos.x, pos.y);
            RECT rec1 = {0, 80, 200, 96};
            m_pFont->DrawTextA(0, str, 19, &rec1, DT_LEFT, 0xffffffff);
        }
#endif

        if (this->allowGetRenderData == true) {
            this->allowGetRenderData = false;
            if (windowed2 == TRUE) {
                do // this is not a loop, I do this instead of goto statement
                {
                    IDirect3DSurface8* pSurface;
                    HRESULT rs = m_device->GetRenderTarget(&pSurface);
                    if (rs != D3D_OK) {
                        this->currentScale = 1.0f;
                        g_currentScale2 = this->currentScale;
                        break;
                    }
                    D3DSURFACE_DESC SurfaceDesc;
                    rs = pSurface->GetDesc(&SurfaceDesc);
                    if (rs != D3D_OK) {
                        this->currentScale = 1.0f;
                        g_currentScale2 = this->currentScale;
                        break;
                    }
                    backBufferSize.width = SurfaceDesc.Width;
                    backBufferSize.height = SurfaceDesc.Height;
                    pSurface->Release();

                    RECT rect;
                    BOOL rs2 = GetClientRect(this->m_hFocusWindow, &rect);
                    if (rs2 == 0) {
                        this->currentScale = 1.0f;
                        g_currentScale2 = this->currentScale;
                        break;
                    }
                    frontBufferSize.width = rect.right - rect.left;
                    frontBufferSize.height = rect.bottom - rect.top;

                    this->currentScale = (float)frontBufferSize.width / backBufferSize.width;
                    g_currentScale2 = this->currentScale;
                } while (0);
            } else {
                this->currentScale = 1.0f;
            }
        }

        D3DXVECTOR2 position((float)(pos.x), (float)(pos.y));
        if (windowed2 == TRUE && this->currentScale != 0.0f && this->currentScale != 1.0f) {
            position /= this->currentScale;
        }
        position.x -= this->cursorWidthCenter;
        position.y -= this->cursorHeightCenter;

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
                m_device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_ADD);
                m_device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
                m_device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

                m_sprite->Draw(m_texture, NULL, NULL, &center, 0, &position, D3DCOLOR_RGBA(c, c, c, 255));
            } else {
                m_sprite->Draw(m_texture, NULL, NULL, &center, 0, &position, D3DCOLOR_RGBA(c, c, c, 255));
            }
        } else
            m_sprite->Draw(m_texture, NULL, NULL, &center, 0, &position, D3DCOLOR_RGBA(255, 200, 200, 128));
        m_sprite->End();


        return m_device->EndScene();
    }

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

    STDMETHOD(SetViewport)(THIS_ CONST D3DVIEWPORT8* pViewport) {
        return m_device->SetViewport(pViewport);
    }

    STDMETHOD(GetViewport)(THIS_ D3DVIEWPORT8* pViewport) {
        return m_device->GetViewport(pViewport);
    }

    STDMETHOD(SetMaterial)(THIS_ CONST D3DMATERIAL8* pMaterial) {
        return m_device->SetMaterial(pMaterial);
    }

    STDMETHOD(GetMaterial)(THIS_ D3DMATERIAL8* pMaterial) {
        return m_device->GetMaterial(pMaterial);
    }

    STDMETHOD(SetLight)(THIS_ DWORD Index, CONST D3DLIGHT8* pLight) {
        return m_device->SetLight(Index, pLight);
    }

    STDMETHOD(GetLight)(THIS_ DWORD Index, D3DLIGHT8* pLight) {
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

    STDMETHOD(CreateStateBlock)(THIS_ D3DSTATEBLOCKTYPE Type, DWORD* pToken) {
        return m_device->CreateStateBlock(Type, pToken);
    }

    STDMETHOD(BeginStateBlock)(THIS) {
        return m_device->BeginStateBlock();
    }

    STDMETHOD(EndStateBlock)(THIS_ DWORD* pToken) {
        return m_device->EndStateBlock(pToken);
    }

    STDMETHOD(SetClipStatus)(THIS_ CONST D3DCLIPSTATUS8* pClipStatus) {
        return m_device->SetClipStatus(pClipStatus);
    }

    STDMETHOD(GetClipStatus)(THIS_ D3DCLIPSTATUS8* pClipStatus) {
        return m_device->GetClipStatus(pClipStatus);
    }

    STDMETHOD(GetTexture)(THIS_ DWORD Stage, IDirect3DBaseTexture8** ppTexture) {
        return m_device->GetTexture(Stage, ppTexture);
    }

    STDMETHOD(SetTexture)(THIS_ DWORD Stage, IDirect3DBaseTexture8* pTexture) {
        return m_device->SetTexture(Stage, pTexture);
    }

    STDMETHOD(GetTextureStageState)(THIS_ DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue) {
        return m_device->GetTextureStageState(Stage, Type, pValue);
    }

    STDMETHOD(SetTextureStageState)(THIS_ DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value) {
        return m_device->SetTextureStageState(Stage, Type, Value);
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

    STDMETHOD(DrawPrimitive)(THIS_ D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount) {
        return m_device->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
    }

    STDMETHOD(DrawIndexedPrimitive)(THIS_ D3DPRIMITIVETYPE PrimitiveType, UINT minIndex, UINT NumVertices, UINT startIndex, UINT primCount) {
        return m_device->DrawIndexedPrimitive(PrimitiveType, minIndex, NumVertices, startIndex, primCount);
    }

    STDMETHOD(DrawPrimitiveUP)(THIS_ D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) {
        return m_device->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
    }

    STDMETHOD(DrawIndexedPrimitiveUP)(THIS_ D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) {
        return m_device->DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride);
    }

    STDMETHOD(ProcessVertices)(THIS_ UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer8* pDestBuffer, DWORD Flags) {
        return m_device->ProcessVertices(SrcStartIndex, DestIndex, VertexCount, pDestBuffer, Flags);
    }

    STDMETHOD(CreateVertexShader)(THIS_ CONST DWORD* pDeclaration, CONST DWORD* pFunction, DWORD* pHandle, DWORD Usage) {
        return m_device->CreateVertexShader(pDeclaration, pFunction, pHandle, Usage);
    }

    STDMETHOD(SetVertexShader)(THIS_ DWORD Handle) {
        return m_device->SetVertexShader(Handle);
    }

    STDMETHOD(GetVertexShader)(THIS_ DWORD* pHandle) {
        return m_device->GetVertexShader(pHandle);
    }

    STDMETHOD(SetStreamSource)(THIS_ UINT StreamNumber, IDirect3DVertexBuffer8* pStreamData, UINT Stride) {
        return m_device->SetStreamSource(StreamNumber, pStreamData, Stride);
    }

    STDMETHOD(GetStreamSource)(THIS_ UINT StreamNumber, IDirect3DVertexBuffer8** ppStreamData, UINT* pStride) {
        return m_device->GetStreamSource(StreamNumber, ppStreamData, pStride);
    }

    STDMETHOD(SetIndices)(THIS_ IDirect3DIndexBuffer8* pIndexData, UINT BaseVertexIndex) {
        return m_device->SetIndices(pIndexData, BaseVertexIndex);
    }

    STDMETHOD(GetIndices)(THIS_ IDirect3DIndexBuffer8** ppIndexData, UINT* pBaseVertexIndex) {
        return m_device->GetIndices(ppIndexData, pBaseVertexIndex);
    }

    STDMETHOD(CreatePixelShader)(THIS_ CONST DWORD* pFunction, DWORD* pHandle) {
        return m_device->CreatePixelShader(pFunction, pHandle);
    }

    STDMETHOD(SetPixelShader)(THIS_ DWORD Handle) {
        return m_device->SetPixelShader(Handle);
    }

    STDMETHOD(GetPixelShader)(THIS_ DWORD* pHandle) {
        return m_device->GetPixelShader(pHandle);
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

private:
    BOOL init = FALSE;
    IDirect3DDevice8* m_device;
    IDirect3D8* m_d3d;
    HWND m_hFocusWindow;
    BOOL allowGetRenderData = true;
    UINTSIZE backBufferSize;
    UINTSIZE frontBufferSize;
    FLOAT currentScale = 1.0f;
    FLOAT cursorWidthCenter = 0;
    FLOAT cursorHeightCenter = 0;

#ifdef DEBUG_MESSAGE
    LPD3DXFONT m_pFont;
#endif
    LPD3DXSPRITE m_sprite;
    LPDIRECT3DTEXTURE8 m_texture;

public:
    // Inherited via IDirect3DDevice8
    STDMETHOD(CopyRects)(THIS_ IDirect3DSurface8 * pSourceSurface, CONST RECT * pSourceRectsArray, UINT cRects, IDirect3DSurface8 * pDestinationSurface, CONST POINT * pDestPointsArray) {
        return m_device->CopyRects(pSourceSurface, pSourceRectsArray, cRects, pDestinationSurface, pDestPointsArray);
    }
    STDMETHOD(GetFrontBuffer)(THIS_ IDirect3DSurface8 * pDestSurface) {
        return m_device->GetFrontBuffer(pDestSurface);
    }
    STDMETHOD(ApplyStateBlock)(THIS_ DWORD Token) {
        return m_device->ApplyStateBlock(Token);
    }
    STDMETHOD(CaptureStateBlock)(THIS_ DWORD Token) {
        return m_device->CaptureStateBlock(Token);
    }
    STDMETHOD(DeleteStateBlock)(THIS_ DWORD Token) {
        return m_device->DeleteStateBlock(Token);
    }
    STDMETHOD(GetInfo)(THIS_ DWORD DevInfoID, void * pDevInfoStruct, DWORD DevInfoStructSize) {
        return m_device->GetInfo(DevInfoID, pDevInfoStruct, DevInfoStructSize);
    }
    STDMETHOD(DeleteVertexShader)(THIS_ DWORD Handle) {
        return m_device->DeleteVertexShader(Handle);
    }
    STDMETHOD(SetVertexShaderConstant)(THIS_ DWORD Register, CONST void * pConstantData, DWORD ConstantCount) {
        return m_device->SetVertexShaderConstant(Register, pConstantData, ConstantCount);
    }
    STDMETHOD(GetVertexShaderConstant)(THIS_ DWORD Register, void * pConstantData, DWORD ConstantCount) {
        return m_device->GetVertexShaderConstant(Register, pConstantData, ConstantCount);
    }
    STDMETHOD(GetVertexShaderDeclaration)(THIS_ DWORD Handle, void * pData, DWORD * pSizeOfData) {
        return m_device->GetVertexShaderDeclaration(Handle, pData, pSizeOfData);
    }
    STDMETHOD(GetVertexShaderFunction)(THIS_ DWORD Handle, void * pData, DWORD * pSizeOfData) {
        return m_device->GetVertexShaderFunction(Handle, pData, pSizeOfData);
    }
    STDMETHOD(DeletePixelShader)(THIS_ DWORD Handle) {
        return m_device->DeletePixelShader(Handle);
    }
    STDMETHOD(SetPixelShaderConstant)(THIS_ DWORD Register, CONST void * pConstantData, DWORD ConstantCount) {
        return m_device->SetPixelShaderConstant(Register, pConstantData, ConstantCount);
    }
    STDMETHOD(GetPixelShaderConstant)(THIS_ DWORD Register, void * pConstantData, DWORD ConstantCount) {
        return m_device->GetPixelShaderConstant(Register, pConstantData, ConstantCount);
    }
    STDMETHOD(GetPixelShaderFunction)(THIS_ DWORD Handle, void * pData, DWORD * pSizeOfData) {
        return m_device->GetPixelShaderFunction(Handle, pData, pSizeOfData);
    }
};