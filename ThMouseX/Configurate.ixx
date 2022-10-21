module;

#include "framework.h"
#include <fstream>
#include <sstream>
#include <unordered_map>

export module main.config;

import core.directx9hook;
import core.directinputhook;
import dx8.hook;
import common.datatype;
import common.helper;
import common.helper.encoding;
import common.var;

namespace helper = common::helper;
namespace encoding = common::helper::encoding;
namespace directx8 = dx8::hook;
namespace directx9 = core::directx9hook;
namespace directinput = core::directinputhook;

using namespace std;

export namespace main::config {
    bool PopulateMethodRVAs() {
        if (!directx8::PopulateMethodRVAs())
            return false;
        if (!directx9::PopulateMethodRVAs())
            return false;
        if (!directinput::PopulateMethodRVAs())
            return false;
        return true;
    }

    bool ReadGamesFile() {
        ifstream gamesFile("games.txt");
        auto& pConfig = gs_gameConfigArray;
        pConfig = {};
        if (!gamesFile) {
            MessageBoxA(NULL, "Can not find games file.", "ThMouseX", MB_OK | MB_ICONERROR);
            return false;
        }
        int configIdx;
        // read each line of config file
        for (configIdx = 0; configIdx < ARRAYSIZE(pConfig.Configs) && !gamesFile.eof(); configIdx++) {
            string _line;
            getline(gamesFile, _line);
            auto lineView = helper::Trim(_line);

#pragma region ignore blank line and comment line
            // ignore empty line and comment line
            if (lineView.empty() || lineView[0] == ';') {
                configIdx--;
                continue;
            }
#pragma endregion

            // use a "tokenizer"
            stringstream lineStream{string(lineView)};
            stringstream converter;
            auto& currentConfig = pConfig.Configs[configIdx];

#pragma region read process name
            string processName;
            lineStream >> processName;
            if (processName.size() == 0) {
                configIdx--;
                continue;
            }
            auto wProcessName = encoding::ConvertToUtf16(processName.c_str());
            if (wProcessName.size() > ARRAYSIZE(currentConfig.ProcessName) - 1) {
                configIdx--;
                continue;
            }
            memcpy(currentConfig.ProcessName, wProcessName.c_str(), wProcessName.size() * sizeof(wProcessName[0]));
#pragma endregion

#pragma region read position address
            string pointerChainStr;
            lineStream >> pointerChainStr;
            currentConfig.ScriptingMethodToFindAddress = ScriptingMethod::None;
            if (_stricmp(pointerChainStr.c_str(), "LuaJIT") == 0) {
                currentConfig.ScriptingMethodToFindAddress = ScriptingMethod::LuaJIT;
            } else if (_stricmp(pointerChainStr.c_str(), "NeoLua") == 0) {
                currentConfig.ScriptingMethodToFindAddress = ScriptingMethod::NeoLua;
            } else {
                size_t leftBoundIdx = 0, rightBoundIdx = -1;
                for (size_t addrLevelIdx = 0; addrLevelIdx < ARRAYSIZE(currentConfig.Address.Level); addrLevelIdx++) {
                    DWORD address;
                    leftBoundIdx = pointerChainStr.find('[', rightBoundIdx + 1);
                    if (leftBoundIdx == string::npos)
                        break;
                    rightBoundIdx = pointerChainStr.find(']', leftBoundIdx + 1);
                    if (rightBoundIdx == string::npos)
                        break;
                    auto memoryOffsetStr = pointerChainStr.substr(leftBoundIdx + 1, rightBoundIdx - leftBoundIdx - 1);
                    converter.clear();
                    converter << memoryOffsetStr;
                    converter >> hex >> address;
                    currentConfig.Address.Level[addrLevelIdx] = address;
                    currentConfig.Address.Length++;
                }
                if (currentConfig.Address.Level[0] == 0) {
                    configIdx--;
                    continue;
                }
            }
#pragma endregion

#pragma region read data type
            string dataType;
            lineStream >> dataType;
            if (_stricmp(dataType.c_str(), "Int") == 0)
                currentConfig.PosDataType = PointDataType::Int;
            else if (_stricmp(dataType.c_str(), "Float") == 0)
                currentConfig.PosDataType = PointDataType::Float;
            else if (_stricmp(dataType.c_str(), "Short") == 0)
                currentConfig.PosDataType = PointDataType::Short;
            else {
                configIdx--;
                continue;
            }
#pragma endregion

#pragma region read offset (X,Y)
            string posOffsetStr;
            lineStream >> posOffsetStr;
            if (posOffsetStr[0] != '(' || posOffsetStr[posOffsetStr.length() - 1] != ')') {
                configIdx--;
                continue;
            }
            auto commaIdx = posOffsetStr.find(',');
            if (commaIdx == string::npos) {
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
            string aspectRatioStr;
            lineStream >> aspectRatioStr;
            auto colonIdx = aspectRatioStr.find(':');
            if (colonIdx == string::npos) {
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
            string inputMethods;
            lineStream >> inputMethods;
            auto inputMethod = strtok(inputMethods.data(), "/");
            while (inputMethod != NULL) {
                if (_stricmp(inputMethod, "DirectInput") == 0)
                    currentConfig.InputMethods |= InputMethod::DirectInput;
                else if (_stricmp(inputMethod, "GetKeyboardState") == 0)
                    currentConfig.InputMethods |= InputMethod::GetKeyboardState;
                else if (_stricmp(inputMethod, "SendKey") == 0)
                    currentConfig.InputMethods |= InputMethod::SendKey;
                inputMethod = strtok(NULL, "/");
            }
            if (currentConfig.InputMethods == InputMethod::None) {
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

    bool ReadIniFile() {
        string _line;
        string_view lineView;

        ifstream vkcodeFile("virtual-key-codes.txt");
        if (!vkcodeFile) {
            MessageBoxA(NULL, "Can not find virtual-key-codes.txt file.", "ThMouseX", MB_OK | MB_ICONERROR);
            return false;
        }
        unordered_map<string, BYTE, string_hash, equal_to<>> vkCodes;
        while (!vkcodeFile.eof()) {
            getline(vkcodeFile, _line);
            lineView = helper::Trim(_line);
            if (lineView.empty() || lineView[0] == ';')
                continue;
            stringstream lineStream{string(lineView)};
            stringstream converter;

            string key;
            lineStream >> key;
            string valueStr;
            lineStream >> valueStr;
            if (key.size() == 0 || valueStr.size() == 0) {
                MessageBoxA(NULL, "virtual-key-codes.txt has invalid format.", "ThMouseX", MB_OK | MB_ICONERROR);
                return false;
            }
            if (valueStr.find("0x") != 0) {
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

        ifstream iniFile("ThMouseX.ini");
        if (!iniFile) {
            MessageBoxA(NULL, "Can not find ThMouseX.ini file.", "ThMouseX", MB_OK | MB_ICONERROR);
            return false;
        }
        getline(iniFile, _line);
        lineView = helper::Trim(_line);
        if (lineView.compare("[ThMouseX]") != 0) {
            MessageBoxA(NULL, "ThMouseX.ini has invalid format.", "ThMouseX", MB_OK | MB_ICONERROR);
            return false;
        }
        while (!iniFile.eof()) {
            getline(iniFile, _line);
            lineView = helper::Trim(_line);
            if (lineView.find("CursorTexture") != string::npos) {
                auto texturePath = helper::Trim(lineView.substr(lineView.find('=') + 1));
                if (texturePath.size() == 0) {
                    MessageBoxA(NULL, "ThMouseX.ini: Invalid CursorTexture.", "ThMouseX", MB_OK | MB_ICONERROR);
                    return false;
                }
                auto wTexturePath = encoding::ConvertToUtf16(string(texturePath).c_str());
                GetFullPathNameW(wTexturePath.c_str(), ARRAYSIZE(gs_textureFilePath), gs_textureFilePath, NULL);
            } else if (lineView.find("CursorBaseHeight") != string::npos) {
                stringstream ss;
                ss << lineView.substr(lineView.find('=') + 1);
                ss >> gs_textureBaseHeight;
                if (gs_textureBaseHeight == 0) {
                    MessageBoxA(NULL, "ThMouseX.ini: Invalid CursorBaseHeight.", "ThMouseX", MB_OK | MB_ICONERROR);
                    return false;
                }
            } else if (lineView.find("BombButton") != string::npos) {
                auto key = helper::Trim(lineView.substr(lineView.find('=') + 1));
                auto value = vkCodes.find(key);
                if (value == vkCodes.end()) {
                    MessageBoxA(NULL, "ThMouseX.ini: Invalid BombButton.", "ThMouseX", MB_OK | MB_ICONERROR);
                    return false;
                }
                gs_bombButton = value->second;
            } else if (lineView.find("ExtraButton") != string::npos) {
                auto key = helper::Trim(lineView.substr(lineView.find('=') + 1));
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
}