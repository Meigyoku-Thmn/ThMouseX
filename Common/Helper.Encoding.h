#pragma once
#include "framework.h"
#include "macro.h"
#include <string>

namespace common::helper::encoding {
    using namespace std;
    DLLEXPORT extern wstring ConvertToUtf16(const char* utf8str);
    DLLEXPORT extern string ConvertToUtf8(const wchar_t* utf16str);
}