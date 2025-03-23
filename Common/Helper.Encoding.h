#pragma once
#include <Windows.h>
#include "macro.h"
#include <string>

namespace common::helper::encoding {
    std::wstring ConvertToUtf16(PCSTR utf8str);
    std::wstring ConvertToUtf16(const std::string& utf8str);
    std::string ConvertToUtf8(const wchar_t* utf16str);
    std::string ConvertToUtf8(const std::wstring& utf16str);
}