#include "framework.h"
#include "macro.h"
#include "Helper.Graphics.h"

namespace common::helper::graphics {
    Dx11BackupState SaveDx11State(IDXGISwapChain* swapChain, ID3D11DeviceContext* context) {
        Dx11BackupState state{};
        context->IAGetIndexBuffer(&state.indexBuffer, &state.indexBufferFormat, &state.indexBufferOffset);
        state.nViewPorts = ARRAYSIZE(state.viewPorts);
        DXGI_SWAP_CHAIN_DESC desc{};
        auto rs = swapChain->GetDesc(&desc);
        if (SUCCEEDED(rs)) {
            state.needRestoreViewport = true;
            context->RSGetViewports(&state.nViewPorts, state.viewPorts);
            auto myViewPort = D3D11_VIEWPORT{
                .TopLeftX = 0,
                .TopLeftY = 0,
                .Width = float(desc.BufferDesc.Width),
                .Height = float(desc.BufferDesc.Height),
            };
            context->RSSetViewports(1, &myViewPort);
        }
        return state;
    }

    void LoadDx11State(ID3D11DeviceContext* context, Dx11BackupState& state) {
        context->IASetIndexBuffer(state.indexBuffer, state.indexBufferFormat, state.indexBufferOffset);
        if (state.needRestoreViewport)
            context->RSSetViewports(state.nViewPorts, state.viewPorts);
        SAFE_RELEASE(state.indexBuffer);
    }
}