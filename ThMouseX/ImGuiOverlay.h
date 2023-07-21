#pragma once
#include <imgui.h>

namespace core::imguioverlay {
    void Prepare();
    void Configure(float fontScale);
    ImDrawData* Render(float mouseScaleX, float mouseScaleY);
}