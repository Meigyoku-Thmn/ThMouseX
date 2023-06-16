#pragma once
#include "framework.h"
#include "macro.h"
#include <string>

#include "DllLoad.h"

namespace common::helper::encoding {
    EXPORT_FUNC(std::wstring, Helper_Encoding_, ConvertToUtf16, const char* utf8str);
    EXPORT_FUNC(std::string, Helper_Encoding_, ConvertToUtf8, const wchar_t* utf16str);
}