#pragma once
#include <Windows.h>
#include "macro.h"
#include <tuple>
#include <string>
#include <cstdint>
#include "DataTypes.h"

namespace common::helper {
    using WParamPredicate = bool(*)(WPARAM wParam, BYTE vkCode);
    using LParamPredicate = bool(*)(LPARAM lParam, BYTE vkCode);
    using VkCodeMessage = std::tuple<
        UINT, WParamPredicate, LParamPredicate,
        UINT, WParamPredicate, LParamPredicate
    >;
    void ReportLastError(PCSTR title);
    std::tuple<float, PCSTR> ConvertToFloat(const std::string& input);
    std::tuple<long, PCSTR> ConvertToLong(const std::string& input, int base);
    std::tuple<unsigned long, PCSTR> ConvertToULong(const std::string& input, int base);
    void CalculateNextTone(UCHAR& tone, ModulateStage& toneStage);
    POINT GetPointerPosition();
    void RemoveWindowBorder(UINT width, UINT height);
    void FixWindowCoordinate(bool isExclusiveMode, UINT d3dWidth, UINT d3dHeight, UINT clientWidth, UINT clientHeight);
    // use for directx8
    bool TestFullscreenHeuristically();
    uintptr_t CalculateAddress();
    bool IsCurrentProcessThMouseX();
    std::tuple<DWORD, std::string> CallProcess(const std::wstring& processPath, const std::wstring& cmdLine);
    bool ShouldBeVkExtended(BYTE key);
    BYTE MapVk2Dik(BYTE vkCode, const BYTE* mappingTable, BYTE defaultDikCode = 0);
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
    void ComMethodTimeout(const std::function<void()>& comAction, DWORD timeout);
    TimerQueueTimerHandle CreateTimerQueueTimer(HANDLE TimerQueue, WAITORTIMERCALLBACK Callback, PVOID Parameter, DWORD DueTime, DWORD Period, ULONG Flags);
    ActCtxCookie ActivateActCtx(HANDLE hActCtx);
}
