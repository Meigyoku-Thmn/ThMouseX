#include "ImGuiOverlay.h"
#include <imgui.h>
#include <cmath>
#include <nameof.hpp>
#include "imgui_impl_win32.h"
#include "../Common/Variables.h"
#include "../Common/Helper.Encoding.h"
#include "../Common/Helper.Memory.h"
#include "../Common/Helper.h"
#include "Configuration.h"

namespace encoding = common::helper::encoding;
namespace memory = common::helper::memory;
namespace helper = common::helper;
namespace configuration = core::configuration;

using namespace std;

static CommonConfig& g_c = g_commonConfig;

static const char* GetKeyName(BYTE vkCode) {
    auto imGuiKey = ImGui_ImplWin32_VirtualKeyToImGuiKey(vkCode);
    auto rs = ImGui::GetKeyName(imGuiKey);
    if (imGuiKey != ImGuiKey_None)
        return rs;
    if (vkCode == SCROLL_UP_EVENT)
        rs = "SCROLL_UP_EVENT";
    if (vkCode == SCROLL_DOWN_EVENT)
        rs = "SCROLL_DOWN_EVENT";
    if (vkCode == SCROLL_LEFT_EVENT)
        rs = "SCROLL_LEFT_EVENT";
    if (vkCode == SCROLL_RIGHT_EVENT)
        rs = "SCROLL_RIGHT_EVENT";
    return rs;
}

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

        if (ImGui::Begin("ThMouseX")) {
            ImGui::Checkbox("Use Raw Input", &useRawInput);

            ImGui::AlignTextToFramePadding();
            ImGui::Text("Sensitivity:");
            ImGui::InvisibleButton("##padding-left", ImVec2(10, 1)); ImGui::SameLine();
            ImGui::SliderFloat("##Sensitivity", &mouseSensitivity, 0.4f, 3.0f, "%.2fx", ImGuiSliderFlags_AlwaysClamp);

            ImGui::AlignTextToFramePadding();
            ImGui::Text("Movement Algorithm:");
            ImGui::InvisibleButton("##padding-left", ImVec2(5, 1)); ImGui::SameLine();
            ImGui::RadioButton("Bresenham's Line", (int*)&g_movementAlgorithm, (int)MovementAlgorithm::Bresenham); ImGui::SameLine();
            ImGui::RadioButton("Simple", (int*)&g_movementAlgorithm, (int)MovementAlgorithm::Simple);

            if (ImGui::Button("Show Variable Viewer")) {
                showVariableViewer = true;
            }
        }
        ImGui::End();

        if (showVariableViewer) {
            if (ImGui::Begin("ThMouseX's Variable Viewer", &showVariableViewer)) {
                if (ImGui::CollapsingHeader("Exe Paths")) {
                    static auto thMousePath = encoding::ConvertToUtf8(g_currentModulePath);
                    ImGui::Text("ThMouseX:\t%s", thMousePath.c_str());

                    static auto gamePath = encoding::ConvertToUtf8(g_currentProcessName);
                    ImGui::Text("Game:\t%s", gamePath.c_str());
                }
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
                    static auto leftBtn = GetKeyName(g_c.VkCodeForLeftClick);
                    ImGui::Text("Left Click Map To:\t\"%s\" 0x%X", leftBtn, g_c.VkCodeForLeftClick);

                    static auto middleBtn = GetKeyName(g_c.VkCodeForMiddleClick);
                    ImGui::Text("Middle Click Map To:\t\"%s\" 0x%X", middleBtn, g_c.VkCodeForMiddleClick);

                    static auto rightBtn = GetKeyName(g_c.VkCodeForRightClick);
                    ImGui::Text("Right Click Map To:\t\"%s\" 0x%X", rightBtn, g_c.VkCodeForRightClick);

                    ImGui::Separator();

                    static auto x1Btn = GetKeyName(g_c.VkCodeForXButton1Click);
                    ImGui::Text("Extended Button 1 Map To:\t\"%s\" 0x%X", x1Btn, g_c.VkCodeForXButton1Click);

                    static auto x2Btn = GetKeyName(g_c.VkCodeForXButton2Click);
                    ImGui::Text("Extended Button 2 Map To:\t\"%s\" 0x%X", x2Btn, g_c.VkCodeForXButton2Click);

                    ImGui::Separator();

                    static auto scrollUpBtn = GetKeyName(g_c.VkCodeForScrollUp);
                    ImGui::Text("Scroll Up Map To:\t\"%s\" 0x%X", scrollUpBtn, g_c.VkCodeForScrollUp);

                    static auto scrollDownBtn = GetKeyName(g_c.VkCodeForScrollDown);
                    ImGui::Text("Scroll Down Map To:\t\"%s\" 0x%X", scrollDownBtn, g_c.VkCodeForScrollDown);

                    static auto scrollLeftBtn = GetKeyName(g_c.VkCodeForScrollLeft);
                    ImGui::Text("Scroll Left Map To:\t\"%s\" 0x%X", scrollLeftBtn, g_c.VkCodeForScrollLeft);

                    static auto scrollRightBtn = GetKeyName(g_c.VkCodeForScrollRight);
                    ImGui::Text("Scroll Right Map To:\t\"%s\" 0x%X", scrollRightBtn, g_c.VkCodeForScrollRight);

                    ///////////////////////////////////////////////////////////////////
                    ImGui::Separator();

                    static auto toggleMouseBtn = GetKeyName(g_c.ToggleMouseControl);
                    ImGui::Text("Toggle Mouse Control Button:\t\"%s\" 0x%X", toggleMouseBtn, g_c.ToggleMouseControl);

                    static auto toggleCurBtn = GetKeyName(g_c.ToggleOsCursorButton);
                    ImGui::Text("Toggle Os Cursor Button:\t\"%s\" 0x%X", toggleCurBtn, g_c.ToggleOsCursorButton);

                    static auto toggleImGBtn = GetKeyName(g_c.ToggleImGuiButton);
                    ImGui::Text("Toggle ImGUI Button:\t\"%s\" 0x%X", toggleImGBtn, g_c.ToggleImGuiButton);

                    ///////////////////////////////////////////////////////////////////
                    ImGui::Separator();

                    static auto texturePath = encoding::ConvertToUtf8(g_c.TextureFilePath);
                    ImGui::Text("Cursor Texture File Path:\t%s", texturePath.c_str());
                    ImGui::Text("Cursor Texture Base Height:\t%d", g_c.TextureBaseHeight);

                    ///////////////////////////////////////////////////////////////////
                    ImGui::Separator();

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