#include "framework.h"
#include "macro.h"
#include "Helper.Graphics.h"
#include "Helper.h"

namespace common::helper::graphics {
    Dx11BackupState SaveDx11State(IDXGISwapChain* swapChain, ID3D11DeviceContext* context) {
        Dx11BackupState state{};
        context->OMGetRenderTargets(ARRAYSIZE(state.renderTargetViews), state.renderTargetViews, &state.depthStencilView);
        context->IAGetIndexBuffer(&state.indexBuffer, &state.indexBufferFormat, &state.indexBufferOffset);
        state.nViewPorts = ARRAYSIZE(state.viewPorts);
        context->RSGetViewports(&state.nViewPorts, state.viewPorts);
        return state;
    }

    void LoadDx11State(ID3D11DeviceContext* context, Dx11BackupState& state) {
        context->OMSetRenderTargets(ARRAYSIZE(state.renderTargetViews), state.renderTargetViews, state.depthStencilView);
        for (auto renderTargetView : state.renderTargetViews)
            helper::SafeRelease(renderTargetView);
        helper::SafeRelease(state.depthStencilView);
        context->IASetIndexBuffer(state.indexBuffer, state.indexBufferFormat, state.indexBufferOffset);
        helper::SafeRelease(state.indexBuffer);
        context->RSSetViewports(state.nViewPorts, state.viewPorts);
    }
}