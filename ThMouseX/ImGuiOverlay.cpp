#include "ImGuiOverlay.h"
#include <imgui.h>
#include <cmath>
#include "imgui_impl_win32.h"
#include "../Common/Variables.h"
#include "../Common/Helper.Encoding.h"

namespace encoding = common::helper::encoding;

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
        auto fontSize = round(gs_imGuiBaseFontSize * fontScale);
        if (fontSize < 13)
            io.Fonts->AddFontDefault();
        else if (!io.Fonts->AddFontFromFileTTF(encoding::ConvertToUtf8(gs_imGuiFontPath).c_str(), fontSize))
            io.Fonts->AddFontDefault();
    }
    ImDrawData* Render(float mouseScaleX, float mouseScaleY) {
        ImGui_ImplWin32_SetMousePosScale(mouseScaleX, mouseScaleY);
        ImGui::NewFrame();
        ImGui::ShowDemoWindow();
        ImGui::EndFrame();
        ImGui::Render();
        return ImGui::GetDrawData();
    }
}