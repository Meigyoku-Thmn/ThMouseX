#pragma once
#include "framework.h"
#include "macro.h"
#include <tuple>
#include "DataTypes.h"

namespace common::helper {
    using namespace std;
    DLLEXPORT void ReportLastError(const char* title);
    DLLEXPORT tuple<float, const char*> ConvertToFloat(const string& input);
    DLLEXPORT tuple<long, const char*> ConvertToLong(const string& input, int base);
    DLLEXPORT tuple<unsigned long, const char*> ConvertToULong(const string& input, int base);
    DLLEXPORT void CalculateNextModulate(UCHAR& modulate, ModulateStage& modulateStage);
    DLLEXPORT POINT GetPointerPosition();
    DLLEXPORT void RemoveWindowBorder(UINT width, UINT height);
    DLLEXPORT void FixWindowCoordinate(bool isExclusiveMode, UINT d3dWidth, UINT d3dHeight, UINT clientWidth, UINT clientHeight);
    // use for directx8
    DLLEXPORT bool TestFullscreenHeuristically();
    DLLEXPORT DWORD CalculateAddress();
}
