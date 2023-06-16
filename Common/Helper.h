#pragma once
#include "framework.h"
#include "macro.h"
#include <tuple>
#include "DataTypes.h"

#include "DllLoad.h"

namespace common::helper {
    EXPORT_FUNC(void, Helper_, ReportLastError, const char* title);
    EXPORT_FUNC(SINGLE_ARG(std::tuple<float, const char*>), Helper_, ConvertToFloat, const std::string& input);
    EXPORT_FUNC(SINGLE_ARG(std::tuple<long, const char*>), Helper_, ConvertToLong, const std::string& input, int base);
    EXPORT_FUNC(SINGLE_ARG(std::tuple<unsigned long, const char*>), Helper_, ConvertToULong, const std::string& input, int base);
    EXPORT_FUNC(void, Helper_, CalculateNextModulate, UCHAR& modulate, ModulateStage& modulateStage);
    EXPORT_FUNC(POINT, Helper_, GetPointerPosition);
    EXPORT_FUNC(void, Helper_, RemoveWindowBorder, UINT width, UINT height);
    EXPORT_FUNC(void, Helper_, FixWindowCoordinate, bool isExclusiveMode, UINT d3dWidth, UINT d3dHeight, UINT clientWidth, UINT clientHeight);
    // use for directx8
    EXPORT_FUNC(bool, Helper_, TestFullscreenHeuristically);
    EXPORT_FUNC(DWORD, Helper_, CalculateAddress);
}
