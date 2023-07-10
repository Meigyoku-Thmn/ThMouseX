#pragma once
#include "framework.h"
#include "macro.h"
#include <string>

namespace common::helper::encoding {
    std::wstring ConvertToUtf16(const char* utf8str);
    std::string ConvertToUtf8(const wchar_t* utf16str);
}