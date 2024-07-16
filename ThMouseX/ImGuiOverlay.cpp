#include "ImGuiOverlay.h"
#include <imgui.h>
#include <cmath>
#include <nameof.hpp>
#include "imgui_impl_win32.h"
#include "../Common/Variables.h"
#include "../Common/Helper.Encoding.h"
#include "../Common/Helper.Memory.h"
#include "../Common/Helper.h"

namespace encoding = common::helper::encoding;
namespace memory = common::helper::memory;
namespace helper = common::helper;

using namespace std;

namespace core::imguioverlay {
    void Prepare() {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        auto& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        ImGui::StyleColorsDark();
    }
    void Configure(float fontScale) {
        auto& io = ImGui::GetIO();
        io.Fonts->Clear();
        auto fontSize = round(float(gs_imGuiBaseFontSize) * fontScale);
        if (fontSize < 13)
            io.Fonts->AddFontDefault();
        else if (!io.Fonts->AddFontFromFileTTF(encoding::ConvertToUtf8(gs_imGuiFontPath).c_str(), fontSize))
            io.Fonts->AddFontDefault();
    }
    ImDrawData* Render(unsigned int renderWidth, unsigned int renderHeight, float mouseScaleX, float mouseScaleY) {
        auto& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(float(renderWidth), float(renderHeight));
        ImGui_ImplWin32_SetMousePosScale(mouseScaleX, mouseScaleY);
        ImGui::NewFrame();

        static auto showVariableViewer = false;
        static auto showImGuiDemoWindow = false;

        if (ImGui::Begin("ThMouseX")) {
            ImGui::Checkbox("Show Variable Viewer", &showVariableViewer);
            ImGui::Checkbox("Show ImGui Demo Window", &showImGuiDemoWindow);
        }
        ImGui::End();

        if (showImGuiDemoWindow) {
            ImGui::ShowDemoWindow(&showImGuiDemoWindow);
        }

        if (showVariableViewer) {
            if (ImGui::Begin("ThMouseX's Variable Viewer", &showVariableViewer)) {
                static auto showDirPaths = false;
                ImGui::Checkbox("Show Directory Paths", &showDirPaths);
                if (showDirPaths) {
                    auto child_size = ImVec2(0, ImGui::GetTextLineHeightWithSpacing() * 3.5f);
                    if (ImGui::BeginChildFrame(ImGui::GetID("Debug_DirPaths"), child_size)) {
                        static auto thMouseDir = encoding::ConvertToUtf8(g_currentModuleDirPath);
                        static auto systemDir = encoding::ConvertToUtf8(g_systemDirPath);
                        static auto gameDir = encoding::ConvertToUtf8(g_currentProcessDirPath);
                        ImGui::Text("ThMouseX:\t%s", thMouseDir.c_str());
                        ImGui::Text("System:\t%s", systemDir.c_str());
                        ImGui::Text("Game:\t%s", gameDir.c_str());
                    }
                    ImGui::EndChildFrame();
                }

                static auto showState = false;
                ImGui::Checkbox("Show State", &showState);
                if (showState) {
                    auto child_size = ImVec2(0, ImGui::GetTextLineHeightWithSpacing() * 6.5f);
                    if (ImGui::BeginChildFrame(ImGui::GetID("Debug_State"), child_size)) {
                        auto mousePos = helper::GetPointerPosition();
                        auto simulatedInput = string(NAMEOF_ENUM_FLAG(g_gameInput));
                        ImGui::Text("Mouse Position:\t(%d,%d)", mousePos.x, mousePos.y);
                        ImGui::Text("Player Position (Scaled):\t(%d,%d)", g_playerPos.x, g_playerPos.y);
                        ImGui::Text("Player Position (Raw):\t(%g,%g)", g_playerPosRaw.X, g_playerPosRaw.Y);
                        ImGui::Text("Simulated Input:\t%s", simulatedInput.c_str());
                        ImGui::Text("Pixel Rate:\t%g", g_pixelRate);
                        ImGui::Text("Pixel Offset:\t(%g,%g)", g_pixelOffset.X, g_pixelOffset.Y);
                    }
                    ImGui::EndChildFrame();
                }
                static auto showGameConfig = false;
                ImGui::Checkbox("Show Game Config", &showGameConfig);
                if (showGameConfig) {
                    auto child_size = ImVec2(0, ImGui::GetTextLineHeightWithSpacing() * 10.5f);
                    if (ImGui::BeginChildFrame(ImGui::GetID("Debug_GameConfig"), child_size)) {
                        static auto procName = encoding::ConvertToUtf8(g_currentConfig.ProcessName);
                        static auto procAddr = memory::GetAddressConfigAsString();
                        static auto scriptType = string(NAMEOF_ENUM(g_currentConfig.ScriptType));
                        static auto posDataType = string(NAMEOF_ENUM(g_currentConfig.PosDataType));
                        static auto inputMethod = string(NAMEOF_ENUM_FLAG(g_currentConfig.InputMethods));
                        ImGui::Text("Process Name:\t%s", procName.c_str());
                        ImGui::Text("Position Address:\t%s", procAddr.c_str());
                        ImGui::Text("Script Type:\t%s", scriptType.c_str());
                        ImGui::Text("Position Data Type:\t%s", posDataType.c_str());
                        ImGui::Text("Base Pixel Offset:\t(%g,%g)", g_currentConfig.BasePixelOffset.X, g_currentConfig.BasePixelOffset.Y);
                        ImGui::Text("Base Height:\t%d", g_currentConfig.BaseHeight);
                        ImGui::Text("Aspect Ratio:\t(%g,%g)", g_currentConfig.AspectRatio.X, g_currentConfig.AspectRatio.Y);
                        ImGui::Text("Input Method(s):\t%s", inputMethod.c_str());
                    }
                    ImGui::EndChildFrame();
                }
                static auto showGlobalConfig = false;
                ImGui::Checkbox("Show Global Config", &showGlobalConfig);
                if (showGlobalConfig) {
                    auto child_size = ImVec2(0, ImGui::GetTextLineHeightWithSpacing() * 9.5f);
                    if (ImGui::BeginChildFrame(ImGui::GetID("Debug_GlobalConfig"), child_size)) {
                        static auto bombBtn = ImGui_ImplWin32_VirtualKeyToImGuiKey(gs_bombButton);
                        static auto extraBtn = ImGui_ImplWin32_VirtualKeyToImGuiKey(gs_extraButton);
                        static auto toggleCurBtn = ImGui_ImplWin32_VirtualKeyToImGuiKey(gs_toggleOsCursorButton);
                        static auto toggleImGBtn = ImGui_ImplWin32_VirtualKeyToImGuiKey(gs_toggleImGuiButton);
                        static auto texturePath = encoding::ConvertToUtf8(gs_textureFilePath);
                        static auto imGuiFontPath = encoding::ConvertToUtf8(gs_imGuiFontPath);
                        ImGui::Text("Bomb Button:\t\"%s\" 0x%X", ImGui::GetKeyName(bombBtn), gs_bombButton);
                        ImGui::Text("Extra Button:\t\"%s\" 0x%X", ImGui::GetKeyName(extraBtn), gs_extraButton);
                        ImGui::Text("Toggle Os Cursor Button:\t\"%s\" 0x%X", ImGui::GetKeyName(toggleCurBtn), gs_toggleOsCursorButton);
                        ImGui::Text("Toggle ImGUI Button:\t\"%s\" 0x%X", ImGui::GetKeyName(toggleImGBtn), gs_toggleImGuiButton);
                        ImGui::Text("Cursor Texture File Path:\t%s", texturePath.c_str());
                        ImGui::Text("Cursor Texture Base Height:\t%d", gs_textureBaseHeight);
                        ImGui::Text("ImGUI Font Path:\t%s", imGuiFontPath.c_str());
                        ImGui::Text("ImGUI Base Font Size:\t%d", gs_imGuiBaseFontSize);
                        ImGui::Text("ImGUI Base Vertical Resolution:\t%d", gs_imGuiBaseVerticalResolution);
                    }
                    ImGui::EndChildFrame();
                }
            }
            ImGui::End();
        }

        ImGui::EndFrame();
        ImGui::Render();
        return ImGui::GetDrawData();
    }
}