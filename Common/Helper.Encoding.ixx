module;

#include "framework.h"
#include "macro.h"
#include <string>

export module common.helper.encoding;

using namespace std;

namespace common::helper::encoding {
    export DLLEXPORT wstring ConvertToUtf16(const char* utf8str) {
        auto chrCount = MultiByteToWideChar(CP_UTF8, 0, utf8str, -1, nullptr, 0);
        if (chrCount == 0)
            return wstring();
        wstring output(chrCount - 1, '\0');
        MultiByteToWideChar(CP_UTF8, 0, utf8str, -1, output.data(), chrCount);
        return output;
    }

    export DLLEXPORT string ConvertToUtf8(const wchar_t* utf16str) {
        auto byteCount = WideCharToMultiByte(CP_UTF8, 0, utf16str, -1, nullptr, 0, nullptr, nullptr);
        if (byteCount == 0)
            return string();
        string output(byteCount - 1, '\0');
        WideCharToMultiByte(CP_UTF8, 0, utf16str, -1, output.data(), byteCount, nullptr, nullptr);
        return output;
    }
}