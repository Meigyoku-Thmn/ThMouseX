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

static CommonConfig& g_c = g_commonConfig;

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
        auto fontSize = round(float(g_c.ImGuiBaseFontSize) * fontScale);
        if (fontSize < 13)
            io.Fonts->AddFontDefault();
        else if (!io.Fonts->AddFontFromFileTTF(encoding::ConvertToUtf8(g_c.ImGuiFontPath).c_str(), fontSize))
            io.Fonts->AddFontDefault();
    }
    ImDrawData* Render(unsigned int renderWidth, unsigned int renderHeight, float mouseScaleX, float mouseScaleY) {
        auto& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(float(renderWidth), float(renderHeight));
        ImGui_ImplWin32_SetMousePosScale(mouseScaleX, mouseScaleY);
        ImGui::NewFrame();

        static auto showVariableViewer = false;

        static auto useRawInput = false;
        static auto mouseSensitivity = 1.0f;
        static auto movementAlgorithm = 0;

        if (ImGui::Begin("ThMouseX")) {
            ImGui::Checkbox("Use Raw Input", &useRawInput);

            ImGui::AlignTextToFramePadding();
            ImGui::Text("Sensitivity:");
            ImGui::InvisibleButton("##padding-left", ImVec2(10, 1)); ImGui::SameLine();
            ImGui::SliderFloat("##Sensitivity", &mouseSensitivity, 0.4f, 3.0f, "%.2fx", ImGuiSliderFlags_AlwaysClamp);

            ImGui::AlignTextToFramePadding();
            ImGui::Text("Movement Algorithm:");
            ImGui::InvisibleButton("##padding-left", ImVec2(5, 1)); ImGui::SameLine();
            ImGui::RadioButton("Bresenham's Line", &movementAlgorithm, 0); ImGui::SameLine();
            ImGui::RadioButton("Simple", &movementAlgorithm, 1);

            ImGui::Checkbox("Show Variable Viewer", &showVariableViewer);
        }
        ImGui::End();

        if (showVariableViewer) {
            if (ImGui::Begin("ThMouseX's Variable Viewer", &showVariableViewer)) {
                if (ImGui::CollapsingHeader("Directory Paths")) {
                    static auto thMouseDir = encoding::ConvertToUtf8(g_currentModuleDirPath);
                    ImGui::Text("ThMouseX:\t%s", thMouseDir.c_str());

                    static auto systemDir = encoding::ConvertToUtf8(g_systemDirPath);
                    ImGui::Text("System:\t%s", systemDir.c_str());

                    static auto gameDir = encoding::ConvertToUtf8(g_currentProcessDirPath);
                    ImGui::Text("Game:\t%s", gameDir.c_str());
                }
                if (ImGui::CollapsingHeader("State")) {
                    auto mousePos = helper::GetPointerPosition();
                    ImGui::Text("Mouse Position:\t(%d,%d)", mousePos.x, mousePos.y);
                    ImGui::Text("Player Position (Scaled):\t(%d,%d)", g_playerPos.x, g_playerPos.y);
                    ImGui::Text("Player Position (Raw):\t(%g,%g)", g_playerPosRaw.X, g_playerPosRaw.Y);

                    auto simulatedInput = NAMEOF_ENUM_FLAG(g_gameInput);
                    ImGui::Text("Simulated Input:\t%s", simulatedInput.c_str());
                    ImGui::Text("Pixel Rate:\t%g", g_pixelRate);
                    ImGui::Text("Pixel Offset:\t(%g,%g)", g_pixelOffset.X, g_pixelOffset.Y);
                }
                if (ImGui::CollapsingHeader("Game Config")) {
                    static auto procName = encoding::ConvertToUtf8(g_gameConfig.processName);
                    ImGui::Text("Process Name:\t%s", procName.c_str());

                    static auto procAddr = memory::GetAddressConfigAsString();
                    ImGui::Text("Position Address:\t%s", procAddr.c_str());

                    static auto scriptEngine = NAMEOF_ENUM(g_gameConfig.ScriptType);
                    ImGui::Text("Script Engine:\t%s", scriptEngine.data());

                    static auto posDataType = NAMEOF_ENUM(g_gameConfig.PosDataType);
                    ImGui::Text("Position Data Type:\t%s", posDataType.data());
                    ImGui::Text("Base Pixel Offset:\t(%g,%g)", g_gameConfig.BasePixelOffset.X, g_gameConfig.BasePixelOffset.Y);
                    ImGui::Text("Base Height:\t%d", g_gameConfig.BaseHeight);
                    ImGui::Text("Aspect Ratio:\t(%g,%g)", g_gameConfig.AspectRatio.X, g_gameConfig.AspectRatio.Y);

                    static auto inputMethod = NAMEOF_ENUM_FLAG(g_gameConfig.InputMethods);
                    ImGui::Text("Input Method(s):\t%s", inputMethod.c_str());
                }
                if (ImGui::CollapsingHeader("UI/UX Config")) {
                    static auto bombBtn = ImGui_ImplWin32_VirtualKeyToImGuiKey(g_c.VkCodeForLeftClick);
                    ImGui::Text("Left Click Map:\t\"%s\" 0x%X", ImGui::GetKeyName(bombBtn), g_c.VkCodeForLeftClick);

                    static auto extraBtn = ImGui_ImplWin32_VirtualKeyToImGuiKey(g_c.VkCodeForMiddleClick);
                    ImGui::Text("Middle Click Map:\t\"%s\" 0x%X", ImGui::GetKeyName(extraBtn), g_c.VkCodeForMiddleClick);

                    static auto toggleCurBtn = ImGui_ImplWin32_VirtualKeyToImGuiKey(g_c.ToggleOsCursorButton);
                    ImGui::Text("Toggle Os Cursor Button:\t\"%s\" 0x%X", ImGui::GetKeyName(toggleCurBtn), g_c.ToggleOsCursorButton);

                    static auto toggleImGBtn = ImGui_ImplWin32_VirtualKeyToImGuiKey(g_c.ToggleImGuiButton);
                    ImGui::Text("Toggle ImGUI Button:\t\"%s\" 0x%X", ImGui::GetKeyName(toggleImGBtn), g_c.ToggleImGuiButton);

                    static auto texturePath = encoding::ConvertToUtf8(g_c.TextureFilePath);
                    ImGui::Text("Cursor Texture File Path:\t%s", texturePath.c_str());
                    ImGui::Text("Cursor Texture Base Height:\t%d", g_c.TextureBaseHeight);

                    static auto imGuiFontPath = encoding::ConvertToUtf8(g_c.ImGuiFontPath);
                    ImGui::Text("ImGUI Font Path:\t%s", imGuiFontPath.c_str());
                    ImGui::Text("ImGUI Base Font Size:\t%d", g_c.ImGuiBaseFontSize);
                    ImGui::Text("ImGUI Base Vertical Resolution:\t%d", g_c.ImGuiBaseVerticalResolution);
                }
            }
            ImGui::End();
        }

        ImGui::EndFrame();
        ImGui::Render();
        return ImGui::GetDrawData();
    }
}