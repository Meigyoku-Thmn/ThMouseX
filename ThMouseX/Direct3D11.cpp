#include <Windows.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <directxtk/SpriteBatch.h>
#include <directxtk/WICTextureLoader.h>
#include <directxtk/CommonStates.h>
#include <vector>
#include <string>
#include <memory>
#include <wrl/client.h>
#include <comdef.h>
#include <mutex>
#include <cstdint>
#include <imgui.h>
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "ImGuiOverlay.h"

#include "../Common/macro.h"
#include "../Common/DataTypes.h"
#include "../Common/MinHook.h"
#include "../Common/CallbackStore.h"
#include "../Common/Variables.h"
#include "../Common/Helper.h"
#include "../Common/Helper.Encoding.h"
#include "../Common/Log.h"
#include "Direct3D11.h"

#include "AdditiveToneShader.hshader"

static CommonConfig& g_c = g_commonConfig;

namespace minhook = common::minhook;
namespace callbackstore = common::callbackstore;
namespace helper = common::helper;
namespace encoding = common::helper::encoding;
namespace note = common::log;
namespace imguioverlay = core::imguioverlay;

#define TAG "[DirectX11] "

using namespace std;
using namespace DirectX;
using namespace Microsoft::WRL;

static constexpr XMVECTORF32 RGBA(UCHAR r, UCHAR g, UCHAR b, UCHAR a) {
    return XMVECTORF32{ (r & 0xFF) / 255.f, (g & 0xFF) / 255.f, (b & 0xFF) / 255.f, (a & 0xFF) / 255.f };
}

static constexpr XMVECTORF32 ToneColor(UCHAR i) {
    return RGBA(i, i, i, 255);
}

constexpr auto ResizeBuffersIdx = 13;
constexpr auto PresentIdx = 8;

using ID3D11DevicePtr = ID3D11Device*;
using ID3D11DeviceContextPtr = ID3D11DeviceContext*;
using ID3D11RenderTargetViewPtr = ID3D11RenderTargetView*;
using SpriteBatchPtr = SpriteBatch*;
using ID3D11PixelShaderPtr = ID3D11PixelShader*;
using ID3D11ShaderResourceViewPtr = ID3D11ShaderResourceView*;
using ID3D11DepthStencilViewPtr = ID3D11DepthStencilView*;

namespace core::directx11 {
    HRESULT WINAPI D3DResizeBuffers(IDXGISwapChain* swapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
    decltype(&D3DResizeBuffers) OriResizeBuffers;

    HRESULT WINAPI D3DPresent(IDXGISwapChain* swapChain, UINT SyncInterval, UINT Flags);
    decltype(&D3DPresent) OriPresent;

    // job flags
    bool firstStepPrepared;
    bool measurementPrepared;
    bool cursorStatePrepared;
    bool imGuiConfigured;

    bool imGuiPrepared;
    bool imGuiCorePrepared;

    static void ClearMeasurementFlags() {
        measurementPrepared = false;
        cursorStatePrepared = false;
    }

    // cursor and screen state
    ID3D11DevicePtr             device;
    ID3D11DeviceContextPtr      context;
    ID3D11RenderTargetViewPtr   renderTargetView;
    SpriteBatchPtr              spriteBatch;
    ID3D11PixelShaderPtr        pixelShader;
    ID3D11ShaderResourceViewPtr cursorTexture;
    XMVECTORF32                 cursorPivot;
    XMVECTORF32                 cursorScale;
    float                       d3dScale = 1.f;
    float                       imGuiMousePosScaleX = 1.f;
    float                       imGuiMousePosScaleY = 1.f;

    HMODULE d3d11;

    static void CleanUp(bool forReal = false) {
        if (imGuiPrepared) {
            ImGui_ImplDX11_Shutdown();
            ImGui_ImplWin32_Shutdown();
        }
        helper::SafeRelease(pixelShader);
        helper::SafeDelete(spriteBatch);
        helper::SafeRelease(cursorTexture);
        helper::SafeRelease(renderTargetView);
        helper::SafeRelease(context);
        helper::SafeRelease(device);
        firstStepPrepared = false;
        measurementPrepared = false;
        cursorStatePrepared = false;
        imGuiConfigured = false;
        imGuiPrepared = false;
        if (forReal) {
            if (imGuiCorePrepared)
                ImGui::DestroyContext();
            helper::SafeFreeLib(d3d11);
        }
    }

    static void TearDownCallback(bool isProcessTerminating) {
        if (isProcessTerminating)
            return;
        CleanUp(true);
    }

    static bool initialized = false;
    static mutex mtx;
    void Initialize() {
        {
            const scoped_lock lock(mtx);
            if (initialized)
                return;
            GetModuleHandleExW(0, (g_systemDirPath + wstring(L"\\d3d11.dll")).c_str(), &d3d11);
            if (!d3d11)
                return;
            initialized = true;
        }

        auto _D3D11CreateDeviceAndSwapChain = rcast<decltype(&D3D11CreateDeviceAndSwapChain)>(GetProcAddress(d3d11, "D3D11CreateDeviceAndSwapChain"));
        if (!_D3D11CreateDeviceAndSwapChain) {
            note::LastErrorToFile(TAG "Failed to import d3d11.dll|D3D11CreateDeviceAndSwapChain");
            return;
        }

        WindowHandle tmpWnd(CreateWindowA("BUTTON", "Temp Window", WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 300, 300, nil, nil, nil, nil));
        if (!tmpWnd) {
            note::LastErrorToFile(TAG "Failed to create a temporary window");
            return;
        }

        ComPtr<ID3D11Device> _device;
        ComPtr<IDXGISwapChain> swap_chain;
        DXGI_SWAP_CHAIN_DESC sd{
            .BufferDesc = DXGI_MODE_DESC{
                .Format = DXGI_FORMAT_R8G8B8A8_UNORM/*
          */},
            .SampleDesc = DXGI_SAMPLE_DESC{
                .Count = 1/*
          */},
            .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
            .BufferCount = 2,
            .OutputWindow = tmpWnd.get(),
            .Windowed = TRUE,
            .SwapEffect = DXGI_SWAP_EFFECT_DISCARD
        };
        D3D_FEATURE_LEVEL feature_levels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
        auto rs = _D3D11CreateDeviceAndSwapChain(nil, D3D_DRIVER_TYPE_HARDWARE, nil, 0, feature_levels, 2, D3D11_SDK_VERSION, &sd, &swap_chain, &_device, nil, nil);
        if (FAILED(rs)) {
            note::DxErrToFile(TAG "Failed to create device and swapchain of DirectX 11", rs);
            return;
        }

        auto vtable = *rcast<uintptr_t**>(swap_chain.Get());

        callbackstore::RegisterUninitializeCallback(TearDownCallback);
        callbackstore::RegisterClearMeasurementFlagsCallback(ClearMeasurementFlags);

        minhook::CreateHook(vector<minhook::HookConfig>{
            { rcast<PVOID>(vtable[PresentIdx]), &D3DPresent, &OriPresent, APP_NAME "_D3DPresent" },
            { rcast<PVOID>(vtable[ResizeBuffersIdx]), &D3DResizeBuffers, &OriResizeBuffers, APP_NAME "_D3DResizeBuffers" },
        });
    }

    static void PrepareFirstStep(IDXGISwapChain* swapChain) {
        if (firstStepPrepared)
            return;
        firstStepPrepared = true;

        auto rs = swapChain->GetDevice(IID_PPV_ARGS(&device));
        if (FAILED(rs)) {
            note::DxErrToFile(TAG "PrepareFirstStep: swapChain->GetDevice failed", rs);
            return;
        }

        device->GetImmediateContext(&context);

        DXGI_SWAP_CHAIN_DESC desc{};
        rs = swapChain->GetDesc(&desc);
        if (FAILED(rs)) {
            note::DxErrToFile(TAG "PrepareFirstStep: swapChain->GetDesc failed", rs);
            return;
        }

        ComPtr<ID3D11Texture2D> pBackBuffer;
        rs = swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        if (FAILED(rs)) {
            note::DxErrToFile(TAG "PrepareFirstStep: swapChain->GetBuffer failed", rs);
            return;
        }

        rs = device->CreateRenderTargetView(pBackBuffer.Get(), nil, &renderTargetView);
        if (FAILED(rs)) {
            note::DxErrToFile(TAG "PrepareFirstStep: device->CreateRenderTargetView failed", rs);
            return;
        }

        spriteBatch = new SpriteBatch(context);

        rs = device->CreatePixelShader(additiveToneShaderBlob, ARRAYSIZE(additiveToneShaderBlob), nil, &pixelShader);
        if (FAILED(rs)) {
            note::DxErrToFile(TAG "PrepareFirstStep: device->CreatePixelShader failed", rs);
            return;
        }

        g_hFocusWindow = desc.OutputWindow;
        g_isMinimized = IsIconic(g_hFocusWindow);

        if (g_c.TextureFilePath[0] && SUCCEEDED(CreateWICTextureFromFile(device, g_c.TextureFilePath, nil, &cursorTexture))) {
            ComPtr<ID3D11Resource> resource;
            cursorTexture->GetResource(&resource);
            ComPtr<ID3D11Texture2D> pTextureInterface;
            resource->QueryInterface<ID3D11Texture2D>(&pTextureInterface);
            D3D11_TEXTURE2D_DESC _desc;
            pTextureInterface->GetDesc(&_desc);
            cursorPivot = { (_desc.Height - 1) / 2.f, (_desc.Width - 1) / 2.f, 0.f };
        }
    }

    HRESULT WINAPI D3DResizeBuffers(IDXGISwapChain* swapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags) {
        CleanUp();
        return OriResizeBuffers(swapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
    }

    /*
    this routine:
    - remove window's border if game is fullscreened (exclusive mode)
    - determine g_pixelRate
    - determine g_pixelOffset
    */
    static void PrepareMeasurement(IDXGISwapChain* swapChain) {
        if (measurementPrepared)
            return;
        measurementPrepared = true;

        if (!g_hFocusWindow)
            return;

        RECTSIZE clientSize{};
        if (GetClientRect(g_hFocusWindow, &clientSize) == FALSE) {
            note::LastErrorToFile(TAG "PrepareMeasurement: GetClientRect failed");
            return;
        }

        DXGI_SWAP_CHAIN_DESC desc;
        auto rs = swapChain->GetDesc(&desc);
        if (FAILED(rs)) {
            note::DxErrToFile(TAG "PrepareMeasurement: swapChain->GetDesc failed", rs);
            return;
        }

        helper::FixWindowCoordinate(!desc.Windowed,
            desc.BufferDesc.Width, desc.BufferDesc.Height, scast<UINT>(clientSize.width()), scast<UINT>(clientSize.height()));

        if (GetClientRect(g_hFocusWindow, &clientSize) == FALSE) {
            note::LastErrorToFile(TAG "PrepareMeasurement: GetClientRect failed");
            return;
        }
        g_pixelRate = scast<float>(g_gameConfig.BaseHeight) / scast<float>(clientSize.height());
        g_pixelOffset.X = g_gameConfig.BasePixelOffset.X / g_pixelRate;
        g_pixelOffset.Y = g_gameConfig.BasePixelOffset.Y / g_pixelRate;
        imGuiMousePosScaleX = scast<float>(clientSize.width()) / scast<float>(desc.BufferDesc.Width);
        imGuiMousePosScaleY = scast<float>(clientSize.height()) / scast<float>(desc.BufferDesc.Height);
    }

    /*
    Determine scaling
    */
    static void PrepareCursorState(IDXGISwapChain* swapChain) {
        if (cursorStatePrepared)
            return;
        cursorStatePrepared = true;

        if (!g_hFocusWindow)
            return;

        DXGI_SWAP_CHAIN_DESC desc;
        auto rs = swapChain->GetDesc(&desc);
        if (FAILED(rs)) {
            note::DxErrToFile(TAG "PrepareCursorState: swapChain->GetDesc failed", rs);
            return;
        }

        auto scale = scast<float>(desc.BufferDesc.Height) / scast<float>(g_c.TextureBaseHeight);
        cursorScale = XMVECTORF32{ scale, scale };

        RECTSIZE clientSize{};
        if (GetClientRect(g_hFocusWindow, &clientSize) == FALSE) {
            note::LastErrorToFile(TAG "PrepareCursorState: GetClientRect failed");
            return;
        }
        d3dScale = scast<float>(clientSize.width()) / scast<float>(desc.BufferDesc.Width);
    }

    static UCHAR tone = 0;
    static auto toneStage = ModulateStage::WhiteInc;
    static void RenderCursor(IDXGISwapChain* swapChain) {
        if (!cursorTexture || !spriteBatch || !renderTargetView || !device || !context)
            return;

        // scale mouse cursor's position from screen coordinate to D3D coordinate
        auto pointerPosition = helper::GetPointerPosition();
        XMVECTOR cursorPositionD3D = XMVECTORF32{ scast<float>(pointerPosition.x), scast<float>(pointerPosition.y) };
        if (d3dScale != 0.f && d3dScale != 1.f) {
            cursorPositionD3D = XMVectorScale(cursorPositionD3D, d3dScale);
        }

        // scale cursor sprite to match the current render resolution
        auto scalingMatrixD3D = XMMatrixTransformation2D(cursorPositionD3D, 0, cursorScale, XMVECTOR(), 0, XMVECTOR());

        DXGI_SWAP_CHAIN_DESC desc{};
        auto rs = swapChain->GetDesc(&desc);
        if (SUCCEEDED(rs)) {
            auto myViewPort = D3D11_VIEWPORT{
                .TopLeftX = 0,
                .TopLeftY = 0,
                .Width = scast<float>(desc.BufferDesc.Width),
                .Height = scast<float>(desc.BufferDesc.Height),
            };
            context->RSSetViewports(1, &myViewPort);
        }

        context->OMSetRenderTargets(1, &renderTargetView, nil);

        auto usePixelShader = false;
        if (g_inputEnabled) {
            using enum ModulateStage;
            helper::CalculateNextTone(tone, toneStage);
            if (toneStage == WhiteInc || toneStage == WhiteDec)
                // default behaviour: texture color * diffuse color
                // this shader: texture color + diffuse color (except alpha)
                usePixelShader = true;
        }

        // draw the cursor
        auto setCustomShaders = usePixelShader ? [] { context->PSSetShader(pixelShader, nil, 0); } : nil;
        auto sortMode = SpriteSortMode_Deferred;
        auto m_states = make_unique<CommonStates>(device);
        spriteBatch->Begin(sortMode, m_states->NonPremultiplied(), nil, nil, nil, setCustomShaders, scalingMatrixD3D);
        auto color = g_inputEnabled ? ToneColor(tone) : RGBA(255, 200, 200, 128);
        spriteBatch->Draw(cursorTexture, cursorPositionD3D, nil, color, 0, cursorPivot, 1, SpriteEffects_None);
        spriteBatch->End();
    }

    static void PrepareImGui() {
        if (imGuiPrepared)
            return;
        imGuiPrepared = true;
        if (!device || !context || !g_hFocusWindow)
            return;

        if (!imGuiCorePrepared) {
            imguioverlay::Prepare();
            imGuiCorePrepared = true;
        }
        ImGui_ImplWin32_Init(g_hFocusWindow);
        ImGui_ImplDX11_Init(device, context);
    }

    static void ConfigureImGui(IDXGISwapChain* swapChain) {
        if (imGuiConfigured)
            return;
        imGuiConfigured = true;

        auto& io = ImGui::GetIO();
        io.Fonts->Clear();

        DXGI_SWAP_CHAIN_DESC desc;
        auto rs = swapChain->GetDesc(&desc);
        if (FAILED(rs)) {
            note::DxErrToFile(TAG "PrepareCursorState: swapChain->GetDesc failed", rs);
            return;
        }

        imguioverlay::Configure(scast<float>(desc.BufferDesc.Height) / scast<float>(g_c.ImGuiBaseVerticalResolution));
    }

    static void RenderImGui(IDXGISwapChain* swapChain) {
        if (!g_showImGui || !context || !renderTargetView)
            return;
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        DXGI_SWAP_CHAIN_DESC desc;
        auto rs = swapChain->GetDesc(&desc);
        if (FAILED(rs)) {
            note::DxErrToFile(TAG "RenderImGui: swapChain->GetDesc failed", rs);
            return;
        }
        auto drawData = imguioverlay::Render(
            desc.BufferDesc.Width, desc.BufferDesc.Height,
            imGuiMousePosScaleX, imGuiMousePosScaleY
        );
        context->OMSetRenderTargets(1, &renderTargetView, nil);
        ImGui_ImplDX11_RenderDrawData(drawData);
    }

    HRESULT WINAPI D3DPresent(IDXGISwapChain* swapChain, UINT SyncInterval, UINT Flags) {
        PrepareFirstStep(swapChain);
        PrepareMeasurement(swapChain);
        PrepareCursorState(swapChain);
        PrepareImGui();
        ConfigureImGui(swapChain);

        auto dxState = ImGui_ImplDX11_BackupDx11State();
        RenderCursor(swapChain);
        RenderImGui(swapChain);
        ImGui_ImplDX11_RestoreDx11State(dxState);

        callbackstore::TriggerPostRenderCallbacks();
        return OriPresent(swapChain, SyncInterval, Flags);
    }
}
