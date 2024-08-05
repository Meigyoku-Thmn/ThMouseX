#pragma once
#include "framework.h"
#include "macro.h"
#include <tuple>
#include <string>
#include "DataTypes.h"

namespace common::helper {
    void ReportLastError(const char* title);
    std::string& Replace(std::string& input, const char* keyword, const char* replacement);
    std::tuple<float, const char*> ConvertToFloat(const std::string& input);
    std::tuple<long, const char*> ConvertToLong(const std::string& input, int base);
    std::tuple<unsigned long, const char*> ConvertToULong(const std::string& input, int base);
    void CalculateNextTone(UCHAR& tone, ModulateStage& toneStage);
    POINT GetPointerPosition();
    void RemoveWindowBorder(UINT width, UINT height);
    void FixWindowCoordinate(bool isExclusiveMode, UINT d3dWidth, UINT d3dHeight, UINT clientWidth, UINT clientHeight);
    // use for directx8
    bool TestFullscreenHeuristically();
    DWORD CalculateAddress();
    bool IsCurrentProcessThMouseX();
    std::tuple<DWORD, std::string> CallProcess(const std::wstring& processPath, const std::wstring& cmdLine);
    bool IsVKExtended(BYTE key);
    BYTE MapVk2Dik(BYTE vkCode, PBYTE mappingTable);
    BYTE NormalizeLeftRightVkCode(BYTE vkCode);
}
