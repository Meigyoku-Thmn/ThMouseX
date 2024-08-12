#pragma once
#include <d3d11.h>

namespace common::helper::graphics {
    struct Dx11BackupState {
        ID3D11RenderTargetView* renderTargetViews[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
        ID3D11DepthStencilView* depthStencilView;
        ID3D11Buffer*           indexBuffer;
        DXGI_FORMAT             indexBufferFormat;
        UINT                    indexBufferOffset;
        D3D11_VIEWPORT          viewPorts[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
        UINT                    nViewPorts;
        bool                    needRestoreViewport;
    };

    Dx11BackupState SaveDx11State(IDXGISwapChain* swapChain, ID3D11DeviceContext* context);
    void LoadDx11State(ID3D11DeviceContext* context, Dx11BackupState& state);
}