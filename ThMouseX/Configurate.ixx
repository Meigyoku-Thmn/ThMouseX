module;

#include "framework.h"
#include <fstream>
#include <sstream>
#include <locale>
#include <codecvt>

export module main.config;

import core.directx9hook;
import core.directinputhook;
import dx8.hook;
import common.datatype;
import common.helper;
import common.var;

using namespace std;

export bool PopulateMethodRVAs() {
    if (!PopulateD3D9MethodRVAs())
        return false;
    if (!PopulateD3D8MethodRVAs())
        return false;
    if (!PopulateDInputMethodRVAs())
        return false;
    return true;
}

export bool ReadGamesFile() {
    locale loc(locale(), new codecvt_utf8<wchar_t>);
    wifstream gamesFile("games.txt");
    gamesFile.imbue(loc);
    auto& pConfig = gs_gameConfigArray;
    pConfig = {};
    if (!gamesFile) {
        MessageBoxA(NULL, "Can not find games file.", "ThMouseX", MB_OK | MB_ICONERROR);
        return false;
    }
    int configIdx;
    // read each line of config file
    for (configIdx = 0; configIdx < GAME_CONFIG_MAX_LEN && !gamesFile.eof(); configIdx++) {
        wstring _line;
        getline(gamesFile, _line);
        auto lineView = Trim(_line);

#pragma region ignore blank line and comment line
        // ignore empty line
        if (lineView.empty()) {
            configIdx--;
            continue;
        }

        // ignore comment line
        if (lineView[0] == ';') {
            configIdx--;
            continue;
        }
#pragma endregion

        // use a "tokenizer"
        wstringstream lineStream{wstring(lineView)};
        wstringstream converter;
        auto& currentConfig = pConfig.Configs[configIdx];

#pragma region read process name
        wstring processName;
        lineStream >> currentConfig.ProcessName;
#pragma endregion

#pragma region read pointer chain
        wstring pointerChainStr;
        lineStream >> pointerChainStr;
        size_t leftBoundIdx = 0, rightBoundIdx = -1;
        for (size_t addressLevelIdx = 0; addressLevelIdx < ADDRESS_CHAIN_MAX_LEN; addressLevelIdx++) {
            DWORD address;
            leftBoundIdx = pointerChainStr.find('[', rightBoundIdx + 1);
            if (leftBoundIdx == wstring::npos)
                break;
            rightBoundIdx = pointerChainStr.find(']', leftBoundIdx + 1);
            if (rightBoundIdx == wstring::npos)
                break;
            auto memoryOffsetStr = pointerChainStr.substr(leftBoundIdx + 1, rightBoundIdx - leftBoundIdx - 1);
            converter.clear();
            converter << memoryOffsetStr;
            converter >> hex >> address;
            currentConfig.Address.Level[addressLevelIdx] = address;
            currentConfig.Address.Length++;
        }
        if (currentConfig.Address.Level[0] == 0) {
            configIdx--;
            continue;
        }
#pragma endregion

#pragma region read data type
        wstring dataType;
        lineStream >> dataType;
        if (_wcsicmp(dataType.c_str(), L"Int") == 0)
            currentConfig.PosDataType = PointDataType::Int;
        else if (_wcsicmp(dataType.c_str(), L"Float") == 0)
            currentConfig.PosDataType = PointDataType::Float;
        else if (_wcsicmp(dataType.c_str(), L"Short") == 0)
            currentConfig.PosDataType = PointDataType::Short;
        else {
            configIdx--;
            continue;
        }
#pragma endregion
        
#pragma region read offset (X,Y)
        wstring posOffsetStr;
        lineStream >> posOffsetStr;
        if (posOffsetStr[0] != '(' || posOffsetStr[posOffsetStr.length() - 1] != ')') {
            configIdx--;
            continue;
        }
        auto commaIdx = posOffsetStr.find(',');
        if (commaIdx == wstring::npos) {
            configIdx--;
            continue;
        }
        auto offsetXStr = posOffsetStr.substr(1, commaIdx - 1);
        float offsetX;
        converter.clear();
        converter << offsetXStr;
        converter >> dec >> offsetX;
        currentConfig.BasePixelOffset.X = offsetX;
        auto offsetYStr = posOffsetStr.substr(commaIdx + 1, posOffsetStr.length() - commaIdx - 2);
        float offsetY;
        converter.clear();
        converter << offsetYStr;
        converter >> dec >> offsetY;
        currentConfig.BasePixelOffset.Y = offsetY;
        if (lineStream.eof() == true) {
            configIdx--;
            continue;
        }
#pragma endregion

#pragma region read game-internal base resolution
        lineStream >> dec >> currentConfig.BaseHeight;
        if (lineStream.eof() == true) {
            configIdx--;
            continue;
        }
#pragma endregion

#pragma region read aspect ratio w:h
        wstring aspectRatioStr;
        lineStream >> aspectRatioStr;
        auto colonIdx = aspectRatioStr.find(':');
        if (colonIdx == wstring::npos) {
            configIdx--;
            continue;
        }
        auto ratioXStr = aspectRatioStr.substr(0, colonIdx);
        float ratioX;
        converter.clear();
        converter << ratioXStr;
        converter >> ratioX;
        currentConfig.AspectRatio.X = ratioX;
        auto ratioYStr = aspectRatioStr.substr(colonIdx + 1, aspectRatioStr.length() - colonIdx - 1);
        float ratioY;
        converter.clear();
        converter << ratioYStr;
        converter >> ratioY;
        currentConfig.AspectRatio.Y = ratioY;
#pragma endregion

#pragma region read input method
        wstring inputMethod;
        lineStream >> inputMethod;
        if (_wcsicmp(inputMethod.c_str(), L"DirectInput") == 0)
            currentConfig.InputMethod = InputMethod::DirectInput;
        else if (_wcsicmp(inputMethod.c_str(), L"GetKeyboardState") == 0)
            currentConfig.InputMethod = InputMethod::GetKeyboardState;
        else if (_wcsicmp(inputMethod.c_str(), L"SendInput") == 0)
            currentConfig.InputMethod = InputMethod::SendInput;
        else {
            configIdx--;
            continue;
        }
#pragma endregion
    }

    if (configIdx == 0) {
        MessageBoxA(NULL, "No valid data in config file.", "ThMouseX", MB_OK | MB_ICONERROR);
        return false;
    }
    pConfig.Length = configIdx;

    return true;
}

export bool ReadIniFile() {
    locale loc(locale(), new codecvt_utf8<wchar_t>);
    wifstream iniFile("ThMouseX.ini");
    iniFile.imbue(loc);
    if (!iniFile) {
        MessageBoxA(NULL, "Can not find ThMouseX.ini file.", "ThMouseX", MB_OK | MB_ICONERROR);
        return false;
    }
    wstring _line;
    getline(iniFile, _line);
    auto lineView = Trim(_line);
    if (lineView.compare(L"[ThMouseX]") != 0) {
        MessageBoxA(NULL, "ThMouseX.ini file error.", "ThMouseX", MB_OK | MB_ICONERROR);
        return false;
    }
    while (!iniFile.eof()) {
        getline(iniFile, _line);
        lineView = Trim(_line);
        if (lineView.find(L"CursorTexture") != wstring::npos) {
            auto eqIndex = lineView.find('=');
            auto pathStr = lineView.substr(eqIndex + 1);
            wstringstream ss;
            ss << pathStr;
            wstring texturePath;
            ss >> texturePath;
            GetFullPathNameW(texturePath.c_str(), MAX_PATH, gs_textureFilePath, NULL);
        } else if (lineView.find(L"CursorOnResolutionX") != wstring::npos) {
            auto eqIndex = lineView.find('=');
            auto numStr = lineView.substr(eqIndex + 1);
            wstringstream ss;
            ss << numStr;
            ss >> gs_textureBaseHeight;
        }
    }
    return true;
}
