#include "framework.h"
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
#include <imgui.h>
#include <imgui_impl_win32.h>
#include "imgui_impl_dx11.h"

#include "../Common/macro.h"
#include "../Common/DataTypes.h"
#include "../Common/MinHook.h"
#include "../Common/CallbackStore.h"
#include "../Common/Variables.h"
#include "../Common/DataTypes.h"
#include "../Common/Helper.h"
#include "../Common/Log.h"
#include "Direct3D11.h"

#include "AdditiveToneShader.hshader"

namespace minhook = common::minhook;
namespace callbackstore = common::callbackstore;
namespace helper = common::helper;
namespace note = common::log;

#define TAG "[DirectX11] "

#define Vt0 XMVECTOR()

#define RGBA(r, g, b, a) XMVECTORF32{(r&0xFF)/255.f, (g&0xFF)/255.f, (b&0xFF)/255.f, (a&0xFF)/255.f}

#define ToneColor(i) RGBA(i, i, i, 255)

constexpr auto ResizeBuffersIdx = 13;
constexpr auto PresentIdx = 8;

using namespace std;
using namespace DirectX;
using namespace Microsoft::WRL;

using CallbackType = void (*)(void);

namespace core::directx11 {
    HRESULT WINAPI D3DResizeBuffers(IDXGISwapChain* swapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
    decltype(&D3DResizeBuffers) OriResizeBuffers;

    HRESULT WINAPI D3DPresent(IDXGISwapChain* swapChain, UINT SyncInterval, UINT Flags);
    decltype(&D3DPresent) OriPresent;

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
    ID3D11Device*               device;
    ID3D11DeviceContext*        context;
    ID3D11RenderTargetView*     renderTargetView;
    SpriteBatch*                spriteBatch;
    ID3D11PixelShader*          pixelShader;
    ID3D11ShaderResourceView*   cursorTexture;
    XMVECTORF32                 cursorPivot;
    XMVECTORF32                 cursorScale;
    float                       d3dScale = 1.f;

    void CleanUp() {
        SAFE_RELEASE(pixelShader);
        SAFE_DELETE(spriteBatch);
        SAFE_RELEASE(cursorTexture);
        SAFE_RELEASE(renderTargetView);
        SAFE_RELEASE(context);
        SAFE_RELEASE(device);
        firstStepPrepared = false;
        measurementPrepared = false;
        cursorStatePrepared = false;
    }

    void ShutdownImGui() {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }

    void TearDownCallback(bool isProcessTerminating) {
        if (isProcessTerminating)
            return;
        ShutdownImGui();
        CleanUp();
    }

    bool initialized;
    void Initialize() {
        if (initialized)
            return;
        auto d3d11 = GetModuleHandleW(L"d3d11.dll");
        if (!d3d11)
            return;
        initialized = true;

        auto _D3D11CreateDeviceAndSwapChain = (decltype(&D3D11CreateDeviceAndSwapChain))GetProcAddress(d3d11, "D3D11CreateDeviceAndSwapChain");
        if (!_D3D11CreateDeviceAndSwapChain) {
            note::LastErrorToFile(TAG "Failed to import d3d11.dll|D3D11CreateDeviceAndSwapChain.");
            return;
        }

        WindowHandle tmpWnd(CreateWindowA("BUTTON", "Temp Window", WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 300, 300, NULL, NULL, NULL, NULL));
        if (!tmpWnd) {
            note::LastErrorToFile(TAG "Failed to create a temporary window.");
            return;
        }

        ComPtr<ID3D11Device> device;
        ComPtr<IDXGISwapChain> swap_chain;
        DXGI_SWAP_CHAIN_DESC sd{
            .BufferDesc = DXGI_MODE_DESC{
                .Format = DXGI_FORMAT_R8G8B8A8_UNORM
            },
            .SampleDesc = DXGI_SAMPLE_DESC{
                .Count = 1
            },
            .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
            .BufferCount = 2,
            .OutputWindow = tmpWnd.get(),
            .Windowed = TRUE,
            .SwapEffect = DXGI_SWAP_EFFECT_DISCARD
        };
        const D3D_FEATURE_LEVEL feature_levels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
        auto rs = _D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, feature_levels, 2, D3D11_SDK_VERSION, &sd, &swap_chain, &device, NULL, NULL);
        if (FAILED(rs)) {
            note::DxErrToFile(TAG "Failed to create device and swapchain of DirectX 11.", rs);
            return;
        }

        auto baseAddress = (DWORD)d3d11;
        auto vtable = *(DWORD**)swap_chain.Get();

        callbackstore::RegisterUninitializeCallback(TearDownCallback);
        callbackstore::RegisterClearMeasurementFlagsCallback(ClearMeasurementFlags);

        minhook::CreateHook(vector<minhook::HookConfig>{
            { PVOID(vtable[PresentIdx]), & D3DPresent, (PVOID*)&OriPresent },
            { PVOID(vtable[ResizeBuffersIdx]), &D3DResizeBuffers, (PVOID*)&OriResizeBuffers },
        });
    }

    void PrepareFirstStep(IDXGISwapChain* swapChain) {
        if (firstStepPrepared)
            return;
        firstStepPrepared = true;

        auto rs = swapChain->GetDevice(IID_PPV_ARGS(&device));
        if (FAILED(rs)) {
            note::DxErrToFile(TAG "PrepareFirstStep: swapChain->GetDevice failed", rs);
            return;
        }

        ComPtr<ID3D11Texture2D> pBackBuffer;
        rs = swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        if (FAILED(rs)) {
            note::DxErrToFile(TAG "PrepareFirstStep: swapChain->GetBuffer failed", rs);
            return;
        }
        rs = device->CreateRenderTargetView(pBackBuffer.Get(), NULL, &renderTargetView);
        if (FAILED(rs)) {
            note::DxErrToFile(TAG "PrepareFirstStep: device->CreateRenderTargetView failed", rs);
            return;
        }

        device->GetImmediateContext(&context);

        spriteBatch = new SpriteBatch(context);

        rs = device->CreatePixelShader(additiveToneShaderBlob, ARRAYSIZE(additiveToneShaderBlob), NULL, &pixelShader);
        if (FAILED(rs)) {
            note::DxErrToFile(TAG "PrepareFirstStep: device->CreatePixelShader failed", rs);
            return;
        }

        DXGI_SWAP_CHAIN_DESC desc{};
        rs = swapChain->GetDesc(&desc);
        if (FAILED(rs)) {
            note::DxErrToFile(TAG "PrepareFirstStep: swapChain->GetDesc failed", rs);
            return;
        }
        g_hFocusWindow = desc.OutputWindow;

        if (gs_textureFilePath[0] && SUCCEEDED(CreateWICTextureFromFile(device, gs_textureFilePath, NULL, &cursorTexture))) {
            ComPtr<ID3D11Resource> resource;
            cursorTexture->GetResource(&resource);
            ComPtr<ID3D11Texture2D> pTextureInterface;
            resource->QueryInterface<ID3D11Texture2D>(&pTextureInterface);
            D3D11_TEXTURE2D_DESC desc;
            pTextureInterface->GetDesc(&desc);
            cursorPivot = { (desc.Height - 1) / 2.f, (desc.Width - 1) / 2.f, 0.f };
        }
    }

    HRESULT WINAPI D3DResizeBuffers(IDXGISwapChain* swapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags) {
        ImGui_ImplDX11_InvalidateDeviceObjects();
        CleanUp();
        auto result = OriResizeBuffers(swapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
        if (SUCCEEDED(result))
            ImGui_ImplDX11_CreateDeviceObjects();
        return result;
    }

    /*
    this routine:
    - remove window's border if game is fullscreened (exclusive mode)
    - determine g_pixelRate
    - determine g_pixelOffset
    */
    void PrepareMeasurement(IDXGISwapChain* swapChain) {
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
            desc.BufferDesc.Width, desc.BufferDesc.Height, UINT(clientSize.width()), UINT(clientSize.height()));

        if (GetClientRect(g_hFocusWindow, &clientSize) == FALSE) {
            note::LastErrorToFile(TAG "PrepareMeasurement: GetClientRect failed");
            return;
        }
        g_pixelRate = float(g_currentConfig.BaseHeight) / clientSize.height();
        g_pixelOffset.X = g_currentConfig.BasePixelOffset.X / g_pixelRate;
        g_pixelOffset.Y = g_currentConfig.BasePixelOffset.Y / g_pixelRate;
    }

    /*
    Determine scaling
    */
    void PrepareCursorState(IDXGISwapChain* swapChain) {
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

        auto scale = float(desc.BufferDesc.Height) / gs_textureBaseHeight;
        cursorScale = XMVECTORF32{ scale, scale };

        RECTSIZE clientSize{};
        if (GetClientRect(g_hFocusWindow, &clientSize) == FALSE) {
            note::LastErrorToFile(TAG "PrepareCursorState: GetClientRect failed");
            return;
        }
        d3dScale = float(clientSize.width()) / desc.BufferDesc.Width;
    }

    void RenderCursor(IDXGISwapChain* swapChain) {
        if (!cursorTexture || !spriteBatch || !renderTargetView || !device || !context)
            return;

        auto needRestoreViewport = false;
        UINT nViewPorts = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
        D3D11_VIEWPORT viewPorts[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
        DXGI_SWAP_CHAIN_DESC desc{};
        auto rs = swapChain->GetDesc(&desc);
        if (SUCCEEDED(rs)) {
            needRestoreViewport = true;
            context->RSGetViewports(&nViewPorts, viewPorts);
            auto myViewPort = D3D11_VIEWPORT{
                .TopLeftX = 0,
                .TopLeftY = 0,
                .Width = (float)desc.BufferDesc.Width,
                .Height = (float)desc.BufferDesc.Height,
            };
            context->RSSetViewports(1, &myViewPort);
        }

        // scale mouse cursor's position from screen coordinate to D3D coordinate
        POINT pointerPosition = helper::GetPointerPosition();
        XMVECTOR cursorPositionD3D = XMVECTORF32{ float(pointerPosition.x), float(pointerPosition.y) };
        if (d3dScale != 0.f && d3dScale != 1.f) {
            cursorPositionD3D = XMVectorScale(cursorPositionD3D, d3dScale);
        }

        // scale cursor sprite to match the current render resolution
        auto scalingMatrixD3D = XMMatrixTransformation2D(cursorPositionD3D, 0, cursorScale, Vt0, 0, Vt0);

        context->OMSetRenderTargets(1, &renderTargetView, NULL);

        static UCHAR tone = 0;
        static auto toneStage = WhiteInc;
        auto usePixelShader = false;
        if (g_inputEnabled) {
            helper::CalculateNextTone(_ref tone, _ref toneStage);
            if (toneStage == WhiteInc || toneStage == WhiteDec)
                // default behaviour: texture color * diffuse color
                // this shader: texture color + diffuse color (except alpha)
                usePixelShader = true;
        }

        // draw the cursor
        auto setCustomShaders = usePixelShader ? []() { context->PSSetShader(pixelShader, NULL, 0); } : nullptr;
        auto sortMode = SpriteSortMode_Deferred;
        auto m_states = std::make_unique<CommonStates>(device);
        spriteBatch->Begin(sortMode, m_states->NonPremultiplied(), NULL, NULL, NULL, setCustomShaders, scalingMatrixD3D);
        auto color = g_inputEnabled ? ToneColor(tone) : RGBA(255, 200, 200, 128);
        spriteBatch->Draw(cursorTexture, cursorPositionD3D, NULL, color, 0, cursorPivot, 1, SpriteEffects_None);
        spriteBatch->End();

        if (needRestoreViewport)
            context->RSSetViewports(nViewPorts, viewPorts);
    }

    void PrepareImGui() {
        if (imGuiPrepared)
            return;
        imGuiPrepared = true;
        if (!device || !context || !g_hFocusWindow)
            return;

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        auto& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        ImGui::StyleColorsDark();
        ImGui_ImplWin32_Init(g_hFocusWindow);
        ImGui_ImplDX11_Init(device, context);
        auto font = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/tahoma.ttf", 20);
        if (!font)
            io.Fonts->AddFontDefault();
    }

    void RenderImGui() {
        if (!g_showImGui || !context || !renderTargetView)
            return;
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        ImGui::ShowDemoWindow();
        ImGui::EndFrame();
        ImGui::Render();
        context->OMSetRenderTargets(1, &renderTargetView, NULL);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }

    HRESULT WINAPI D3DPresent(IDXGISwapChain* swapChain, UINT SyncInterval, UINT Flags) {
        PrepareFirstStep(swapChain);
        PrepareMeasurement(swapChain);
        PrepareCursorState(swapChain);
        PrepareImGui();
        RenderCursor(swapChain);
        RenderImGui();
        callbackstore::TriggerPostRenderCallbacks();
        return OriPresent(swapChain, SyncInterval, Flags);
    }
}
