#pragma once
#include "framework.h"
#include "macro.h"
#include <tuple>
#include <string>
#include "DataTypes.h"

namespace common::helper {
    using WParamPredicate = bool (*)(WPARAM wParam, BYTE vkCode);
    using LParamPredicate = bool (*)(LPARAM lParam, BYTE vkCode);
    using VkCodeMessage = std::tuple<
        UINT, WParamPredicate, LParamPredicate,
        UINT, WParamPredicate, LParamPredicate
    >;
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
    bool ShouldBeVkExtended(BYTE key);
    BYTE MapVk2Dik(BYTE vkCode, PBYTE mappingTable, BYTE defaultDikCode = 0);
    BYTE NormalizeLeftRightVkCode(BYTE vkCode);
    VkCodeMessage ConvertVkCodeToMessage(BYTE vkCode);
    HRESULT Win32FromHResult(HRESULT hr);
    template<typename ComPtrT>
    void SafeRelease(ComPtrT& p) {
        if (p) {
            p->Release();
            p = nil;
        }
    }
    template<typename PtrT>
    void SafeDelete(PtrT& a) {
        if (a) {
            delete a;
            a = nil;
        }
    }
    template<typename PtrT>
    void SafeDeleteArr(PtrT& arr) {
        if (arr) {
            delete[] arr;
            arr = nil;
        }
    }
    void SafeFreeLib(HMODULE& hLibModule);
    std::wstring ExpandEnvStr(const wchar_t* str);
    std::wstring ExpandEnvStr(const std::wstring& str);
}
