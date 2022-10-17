module;

#include "framework.h"
#include <fstream>
#include <sstream>
#include <locale>
#include <codecvt>
#include <unordered_map>

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

#pragma region read position address
        wstring pointerChainStr;
        lineStream >> pointerChainStr;
        currentConfig.ScriptingMethodToFindAddress = ScriptingMethod::None;
        if (_wcsicmp(pointerChainStr.c_str(), L"LuaJIT") == 0) {
            currentConfig.ScriptingMethodToFindAddress = ScriptingMethod::LuaJIT;
        } else if (_wcsicmp(pointerChainStr.c_str(), L"NeoLua") == 0) {
            currentConfig.ScriptingMethodToFindAddress = ScriptingMethod::NeoLua;
        } else {
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
        else if (_wcsicmp(inputMethod.c_str(), L"SendKey") == 0)
            currentConfig.InputMethod = InputMethod::SendKey;
        else {
            configIdx--;
            continue;
        }
#pragma endregion
    }

    if (configIdx == 0) {
        MessageBoxA(NULL, "No valid data in games file.", "ThMouseX", MB_OK | MB_ICONERROR);
        return false;
    }
    pConfig.Length = configIdx;

    return true;
}

export bool ReadIniFile() {
    locale loc(locale(), new codecvt_utf8<wchar_t>);
    wstring _line;
    wstring_view lineView;

    wifstream vkcodeFile("virtual-key-codes.txt");
    vkcodeFile.imbue(loc);
    if (!vkcodeFile) {
        MessageBoxA(NULL, "Can not find virtual-key-codes.txt file.", "ThMouseX", MB_OK | MB_ICONERROR);
        return false;
    }
    unordered_map<wstring, BYTE, wstring_hash, equal_to<>> vkCodes;
    while (!vkcodeFile.eof()) {
        getline(vkcodeFile, _line);
        lineView = Trim(_line);
        if (lineView.empty())
            continue;
        if (lineView[0] == ';')
            continue;
        wstringstream lineStream{wstring(lineView)};
        wstringstream converter;

        wstring key;
        lineStream >> key;
        wstring valueStr;
        lineStream >> valueStr;
        if (key.size() == 0 || valueStr.size() == 0) {
            MessageBoxA(NULL, "virtual-key-codes.txt has invalid format.", "ThMouseX", MB_OK | MB_ICONERROR);
            return false;
        }
        if (valueStr.find(L"0x") != 0) {
            MessageBoxA(NULL, "virtual-key-codes.txt has invalid format.", "ThMouseX", MB_OK | MB_ICONERROR);
            return false;
        }
        converter << valueStr.substr(2);
        int value = -1;
        converter >> hex >> value;
        if (value < 0) {
            MessageBoxA(NULL, "virtual-key-codes.txt has invalid format.", "ThMouseX", MB_OK | MB_ICONERROR);
            return false;
        }
        vkCodes[key] = value;
    }

    wifstream iniFile("ThMouseX.ini");
    iniFile.imbue(loc);
    if (!iniFile) {
        MessageBoxA(NULL, "Can not find ThMouseX.ini file.", "ThMouseX", MB_OK | MB_ICONERROR);
        return false;
    }
    getline(iniFile, _line);
    lineView = Trim(_line);
    if (lineView.compare(L"[ThMouseX]") != 0) {
        MessageBoxA(NULL, "ThMouseX.ini has invalid format.", "ThMouseX", MB_OK | MB_ICONERROR);
        return false;
    }
    while (!iniFile.eof()) {
        getline(iniFile, _line);
        lineView = Trim(_line);
        if (lineView.find(L"CursorTexture") != wstring::npos) {
            auto texturePath = Trim(lineView.substr(lineView.find('=') + 1));
            if (texturePath.size() == 0) {
                MessageBoxA(NULL, "ThMouseX.ini: Invalid CursorTexture.", "ThMouseX", MB_OK | MB_ICONERROR);
                return false;
            }
            GetFullPathNameW(wstring(texturePath).c_str(), ARRAYSIZE(gs_textureFilePath), gs_textureFilePath, NULL);
        } else if (lineView.find(L"CursorBaseHeight") != wstring::npos) {
            wstringstream ss;
            ss << lineView.substr(lineView.find('=') + 1);
            ss >> gs_textureBaseHeight;
            if (gs_textureBaseHeight == 0) {
                MessageBoxA(NULL, "ThMouseX.ini: Invalid CursorBaseHeight.", "ThMouseX", MB_OK | MB_ICONERROR);
                return false;
            }
        } else if (lineView.find(L"BombButton") != wstring::npos) {
            auto key = Trim(lineView.substr(lineView.find('=') + 1));
            auto value = vkCodes.find(key);
            if (value == vkCodes.end()) {
                MessageBoxA(NULL, "ThMouseX.ini: Invalid BombButton.", "ThMouseX", MB_OK | MB_ICONERROR);
                return false;
            }
            gs_bombButton = value->second;
        } else if (lineView.find(L"ExtraButton") != wstring::npos) {
            auto key = Trim(lineView.substr(lineView.find('=') + 1));
            auto value = vkCodes.find(key);
            if (value == vkCodes.end()) {
                MessageBoxA(NULL, "ThMouseX.ini: Invalid ExtraButton.", "ThMouseX", MB_OK | MB_ICONERROR);
                return false;
            }
            gs_extraButton = value->second;
        }
    }
    return true;
}
