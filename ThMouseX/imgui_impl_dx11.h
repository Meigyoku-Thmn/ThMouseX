// Based on imgui_impl_dx11 from ImGui's repository, but remove D3DCompiler dependency

// dear imgui: Renderer Backend for DirectX11
// This needs to be used along with a Platform Backend (e.g. Win32)

// Implemented features:
//  [X] Renderer: User texture binding. Use 'ID3D11ShaderResourceView*' as ImTextureID. Read the FAQ about ImTextureID!
//  [X] Renderer: Large meshes support (64k+ vertices) with 16-bit indices.

// You can use unmodified imgui_impl_* files in your project. See examples/ folder for examples of using this. 
// Prefer including the entire imgui/ repository into your project (either as a copy or as a submodule), and only build the backends you need.
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#pragma once
#include <imgui.h>      // IMGUI_IMPL_API
#include <d3d11.h>

struct ID3D11Device;
struct ID3D11DeviceContext;

// Backup DX state that will be modified to restore it afterwards (unfortunately this is very ugly looking and verbose. Close your eyes!)
struct BACKUP_DX11_STATE {
    ID3D11RenderTargetView* /**/    renderTargetViews[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
    ID3D11DepthStencilView* /**/    depthStencilView;
    UINT                            ScissorRectsCount;
    UINT                            ViewportsCount;
    D3D11_RECT                      ScissorRects[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
    D3D11_VIEWPORT                  Viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
    ID3D11RasterizerState* /**/     RS;
    ID3D11BlendState* /**/          BlendState;
    FLOAT                           BlendFactor[4];
    UINT                            SampleMask;
    UINT                            StencilRef;
    ID3D11DepthStencilState* /**/   DepthStencilState;
    ID3D11ShaderResourceView* /**/  PSShaderResource;
    ID3D11SamplerState* /**/        PSSampler;
    ID3D11PixelShader* /**/         PS;
    ID3D11VertexShader* /**/        VS;
    ID3D11GeometryShader* /**/      GS;
    UINT                            PSInstancesCount;
    UINT                            VSInstancesCount;
    UINT                            GSInstancesCount;
    // 256 is max according to PSSetShader documentation
    ID3D11ClassInstance* /**/       PSInstances[256];
    ID3D11ClassInstance* /**/       VSInstances[256];
    ID3D11ClassInstance* /**/       GSInstances[256];
    D3D11_PRIMITIVE_TOPOLOGY        PrimitiveTopology;
    ID3D11Buffer* /**/              IndexBuffer;
    ID3D11Buffer* /**/              VertexBuffer;
    ID3D11Buffer* /**/              VSConstantBuffer;
    UINT                            IndexBufferOffset;
    UINT                            VertexBufferStride;
    UINT                            VertexBufferOffset;
    DXGI_FORMAT                     IndexBufferFormat;
    ID3D11InputLayout* /**/         InputLayout;
};

IMGUI_IMPL_API bool     ImGui_ImplDX11_Init(ID3D11Device* device, ID3D11DeviceContext* device_context);
IMGUI_IMPL_API void     ImGui_ImplDX11_Shutdown();
IMGUI_IMPL_API void     ImGui_ImplDX11_NewFrame();
IMGUI_IMPL_API void     ImGui_ImplDX11_RenderDrawData(ImDrawData* draw_data);

// Use if you want to reset your rendering device without losing Dear ImGui state.
IMGUI_IMPL_API void     ImGui_ImplDX11_InvalidateDeviceObjects();
IMGUI_IMPL_API bool     ImGui_ImplDX11_CreateDeviceObjects();

IMGUI_IMPL_API BACKUP_DX11_STATE    ImGui_ImplDX11_BackupDx11State();
IMGUI_IMPL_API void                 ImGui_ImplDX11_RestoreDx11State(const BACKUP_DX11_STATE& old);