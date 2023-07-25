#pragma once
#include <imgui.h>

namespace core::imguioverlay {
    void Prepare();
    void Configure(float fontScale);
    ImDrawData* Render(unsigned int renderWidth, unsigned int renderHeight, float mouseScaleX, float mouseScaleY);
}