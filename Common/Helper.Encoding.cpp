#include "framework.h"
#include <string>

#include "Helper.Encoding.h"

using namespace std;

namespace common::helper::encoding {
    wstring ConvertToUtf16(const char* utf8str) {
        auto chrCount = MultiByteToWideChar(CP_UTF8, 0, utf8str, -1, nil, 0);
        if (chrCount == 0)
            return wstring();
        wstring output(chrCount - 1, '\0');
        MultiByteToWideChar(CP_UTF8, 0, utf8str, -1, output.data(), chrCount);
        return output;
    }

    wstring ConvertToUtf16(const string& utf8str) {
        return ConvertToUtf16(utf8str.c_str());
    }

    string ConvertToUtf8(const wchar_t* utf16str) {
        auto byteCount = WideCharToMultiByte(CP_UTF8, 0, utf16str, -1, nil, 0, nil, nil);
        if (byteCount == 0)
            return string();
        string output(byteCount - 1, '\0');
        WideCharToMultiByte(CP_UTF8, 0, utf16str, -1, output.data(), byteCount, nil, nil);
        return output;
    }

    string ConvertToUtf8(const wstring& utf16str) {
        return ConvertToUtf8(utf16str.c_str());
    }
}