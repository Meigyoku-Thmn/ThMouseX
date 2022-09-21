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

        // use a "tokenizer"
        wstringstream lineStream{wstring(lineView)};
        wstringstream converter;
        auto& currentConfig = pConfig.Configs[configIdx];

        // read process name
        wstring processName;
        lineStream >> currentConfig.ProcessName;

        // read pointer chain
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

        // read data type
        wstring dataType;
        lineStream >> dataType;
        if (dataType.compare(L"int") == 0)
            currentConfig.PosDataType = Int_DataType;
        else if (dataType.compare(L"float") == 0)
            currentConfig.PosDataType = Float_DataType;
        else if (dataType.compare(L"short") == 0)
            currentConfig.PosDataType = Short_DataType;
        else {
            configIdx--;
            continue;
        }

        // read offset (X,Y)
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

        // read game-internal base resolution
        lineStream >> dec >> currentConfig.BaseResolutionX;
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
        if (lineView.find(L"LeftButton") != wstring::npos) {
            auto eqIndex = lineView.find('=');
            auto numStr = lineView.substr(eqIndex + 1);
            wstringstream ss;
            ss << numStr;
            ss >> gs_boomButton;
        } else if (lineView.find(L"CursorTexture") != wstring::npos) {
            auto eqIndex = lineView.find('=');
            auto pathStr = lineView.substr(eqIndex + 1);
            wstringstream ss;
            ss << pathStr;
            wstring texturePath;
            ss >> texturePath;
            GetFullPathNameW(texturePath.c_str(), MAX_PATH, gs_textureFilePath, NULL);
        } else if (lineView.find(L"MiddleButton") != wstring::npos) {
            auto eqIndex = lineView.find('=');
            auto numStr = lineView.substr(eqIndex + 1);
            wstringstream ss;
            ss << numStr;
            ss >> gs_extraButton;
        } else if (lineView.find(L"CursorOnResolutionX") != wstring::npos) {
            auto eqIndex = lineView.find('=');
            auto numStr = lineView.substr(eqIndex + 1);
            wstringstream ss;
            ss << numStr;
            ss >> gs_textureBaseResolutionX;
        }
    }
    return true;
}
