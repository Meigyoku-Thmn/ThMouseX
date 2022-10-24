module;

#include "framework.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <unordered_map>
#include <format>
#include <tuple>
#include <cassert>

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

#define GameFile "games.txt"
#define ThMouseXFile "ThMouseX.txt"
#define VirtualKeyCodesFile "VirtualKeyCodes.txt"

using namespace std;

typedef unordered_map<string, BYTE, string_hash, equal_to<>> VkCodes;

namespace main {
    export class config;
}
class main::config {
public:
    static bool PopulateMethodRVAs() {
        if (!directx8::PopulateMethodRVAs())
            return false;
        if (!directx9::PopulateMethodRVAs())
            return false;
        if (!directinput::PopulateMethodRVAs())
            return false;
        return true;
    }

    static bool ReadGamesFile() {
        ifstream gamesFile(GameFile);
        if (!gamesFile) {
            MessageBoxA(NULL, "Missing " GameFile " file.", "ThMouseX", MB_OK | MB_ICONERROR);
            return false;
        }
        gs_gameConfigArray = {};
        auto& gameConfigs = gs_gameConfigArray;

        string line;
        int lineCount = 0;
        while (gameConfigs.Length < ARRAYSIZE(gameConfigs.Configs) && getline(gamesFile, line)) {
            lineCount++;
            stringstream lineStream(line);
            if (TestCommentLine(lineStream))
                continue;
            bool succeeded;

            wstring processName;
            tie(processName, succeeded) = ExtractProcessName(lineStream, lineCount);
            if (!succeeded)
                return false;

            vector<DWORD> addressOffsets;
            ScriptingMethod scriptingMethod;
            tie(addressOffsets, scriptingMethod, succeeded) = ExtractPositionRVA(lineStream, lineCount);
            if (!succeeded)
                return false;

            PointDataType dataType;
            tie(dataType, succeeded) = ExtractDataType(lineStream, lineCount);
            if (!succeeded)
                return false;

            FloatPoint offset;
            tie(offset, succeeded) = ExtractOffset(lineStream, lineCount);
            if (!succeeded)
                return false;

            DWORD baseHeight;
            tie(baseHeight, succeeded) = ExtractBaseHeight(lineStream, lineCount);
            if (!succeeded)
                return false;

            FloatPoint aspectRatio;
            tie(aspectRatio, succeeded) = ExtractAspectRatio(lineStream, lineCount);
            if (!succeeded)
                return false;

            InputMethod inputMethods;
            tie(inputMethods, succeeded) = ExtractInputMethod(lineStream, lineCount);
            if (!succeeded)
                return false;

            auto& gameConfig = gameConfigs.Configs[gameConfigs.Length++];

            static_assert(is_same<decltype(&gameConfig.ProcessName[0]), decltype(processName.data())>());
            memcpy(gameConfig.ProcessName, processName.c_str(), processName.size() * sizeof(processName[0]));

            static_assert(is_same<decltype(&gameConfig.Address.Level[0]), decltype(addressOffsets.data())>());
            memcpy(gameConfig.Address.Level, addressOffsets.data(), addressOffsets.size() * sizeof(addressOffsets[0]));
            assert(addressOffsets.size() <= ARRAYSIZE(gameConfig.Address.Level));
            gameConfig.Address.Length = addressOffsets.size();

            gameConfig.PosDataType = dataType;

            gameConfig.BasePixelOffset = offset;

            static_assert(is_same<decltype(gameConfig.BaseHeight), decltype(baseHeight)>());
            gameConfig.BaseHeight = baseHeight;

            gameConfig.AspectRatio = aspectRatio;

            gameConfig.InputMethods = inputMethods;
        }

        return true;
    }

    static bool ReadGeneralConfigFile() {
        auto [vkCodes, succeeded] = ReadVkCodes();
        if (!succeeded)
            return false;

        ifstream iniFile(ThMouseXFile);
        if (!iniFile) {
            MessageBoxA(NULL, "Missing " ThMouseXFile " file.", "ThMouseX", MB_OK | MB_ICONERROR);
            return false;
        }

        int lineCount = 0;
        string line;
        while (getline(iniFile, line)) {
            lineCount++;
            stringstream lineStream(line);

            string key;
            getline(lineStream >> ws, key, '=');
            key = key.substr(0, key.find(' '));
            if (key.starts_with(";"))
                continue;

            string value;
            lineStream >> quoted(value);

            if (key.empty() && value.empty())
                continue;
            if (key == "BombButton") {
                auto vkCode = vkCodes.find(value);
                if (vkCode == vkCodes.end()) {
                    MessageBoxA(NULL, ThMouseXFile ": Invalid BombButton value.", "ThMouseX", MB_OK | MB_ICONERROR);
                    return false;
                }
                gs_bombButton = vkCode->second;
            } else if (key == "ExtraButton") {
                auto vkCode = vkCodes.find(value);
                if (vkCode == vkCodes.end()) {
                    MessageBoxA(NULL, ThMouseXFile ": Invalid ExtraButton.", "ThMouseX", MB_OK | MB_ICONERROR);
                    return false;
                }
                gs_extraButton = vkCode->second;
            } else if (key == "ToggleOsCursorButton") {
                auto vkCode = vkCodes.find(value);
                if (vkCode == vkCodes.end()) {
                    MessageBoxA(NULL, ThMouseXFile ": Invalid ToggleOsCursorButton.", "ThMouseX", MB_OK | MB_ICONERROR);
                    return false;
                }
                gs_toggleOsCursorButton = vkCode->second;
            } else if (key == "CursorTexture") {
                if (value.size() == 0) {
                    MessageBoxA(NULL, ThMouseXFile ": Invalid CursorTexture.", "ThMouseX", MB_OK | MB_ICONERROR);
                    return false;
                }
                auto wTexturePath = encoding::ConvertToUtf16(value.c_str());
                GetFullPathNameW(wTexturePath.c_str(), ARRAYSIZE(gs_textureFilePath), gs_textureFilePath, NULL);
            } else if (key == "CursorBaseHeight") {
                auto [height, convMessage] = helper::ConvertToULong(value, 10);
                if (convMessage != nullptr) {
                    MessageBoxA(NULL, format(ThMouseXFile ": Invalid CursorBaseHeight: {}.", convMessage).c_str(),
                        "ThMouseX", MB_OK | MB_ICONERROR);
                    return false;
                }
                gs_textureBaseHeight = height;
            } else {
                MessageBoxA(NULL, format("Invalid attribute at line {} in " ThMouseXFile ".", lineCount).c_str(),
                    "ThMouseX", MB_OK | MB_ICONERROR);
                return false;
            }
        }
        return true;
    }
private:
    static bool TestCommentLine(stringstream& stream) {
        char firstChr;
        stream >> ws >> firstChr;
        if (firstChr == ';' || firstChr == '\0')
            return true;

        stream.seekg(-1, ios_base::cur);
        return false;
    }

    static tuple<wstring, bool> ExtractProcessName(stringstream& stream, int lineCount) {
        string processName;
        stream >> quoted(processName);
        auto wProcessName = encoding::ConvertToUtf16(processName.c_str());

        auto maxSize = ARRAYSIZE(gs_gameConfigArray.Configs[0].ProcessName) - 1;
        if (wProcessName.size() > maxSize) {
            MessageBoxA(NULL, format("processName longer than {} characters at line {} in " GameFile ".",
                maxSize, lineCount).c_str(), "ThMouseX", MB_OK | MB_ICONERROR);
            return {move(wProcessName), false};
        }
        return {move(wProcessName), true};
    }

    static tuple<vector<DWORD>, ScriptingMethod, bool> ExtractPositionRVA(stringstream& stream, int lineCount) {
        string pointerChainStr;
        stream >> pointerChainStr;
        vector<DWORD> addressOffsets;
        auto scriptingEngine = ScriptingMethod::None;

        if (_stricmp(pointerChainStr.c_str(), "LuaJIT") == 0)
            scriptingEngine = ScriptingMethod::LuaJIT; 
        else if (_stricmp(pointerChainStr.c_str(), "NeoLua") == 0)
            scriptingEngine = ScriptingMethod::NeoLua; 
        else {
            auto maxSize = ARRAYSIZE(gs_gameConfigArray.Configs[0].Address.Level);
            addressOffsets.reserve(maxSize);
            size_t leftBoundIdx = 0, rightBoundIdx = -1;
            for (size_t addrLevelIdx = 0; addrLevelIdx < maxSize; addrLevelIdx++) {
                leftBoundIdx = pointerChainStr.find('[', rightBoundIdx + 1);
                if (leftBoundIdx == string::npos)
                    break;
                rightBoundIdx = pointerChainStr.find(']', leftBoundIdx + 1);
                if (rightBoundIdx == string::npos)
                    break;

                auto offsetStr = pointerChainStr.substr(leftBoundIdx + 1, rightBoundIdx - leftBoundIdx - 1);
                auto [offset, convMessage] = helper::ConvertToULong(offsetStr, 16);
                if (convMessage != nullptr) {
                    MessageBoxA(NULL, format("Invalid positionRVA: {} at line {} in " GameFile ".",
                        convMessage, lineCount).c_str(), "ThMouseX", MB_OK | MB_ICONERROR);
                    return {move(addressOffsets), scriptingEngine, false};
                }

                addressOffsets.push_back(offset);
            }

            if (addressOffsets.size() == 0) {
                MessageBoxA(NULL, format("Found no address offset for positionRVA at line {} in " GameFile ".",
                    lineCount).c_str(), "ThMouseX", MB_OK | MB_ICONERROR);
                return {move(addressOffsets), scriptingEngine, false};
            }
        }

        return {move(addressOffsets), scriptingEngine, true};
    }

    static tuple<PointDataType, bool> ExtractDataType(stringstream& stream, int lineCount) {
        string dataTypeStr;
        stream >> dataTypeStr;
        auto dataType = PointDataType::None;

        if (_stricmp(dataTypeStr.c_str(), "Int") == 0)
            dataType = PointDataType::Int;
        else if (_stricmp(dataTypeStr.c_str(), "Float") == 0)
            dataType = PointDataType::Float;
        else if (_stricmp(dataTypeStr.c_str(), "Short") == 0)
            dataType = PointDataType::Short;
        else {
            MessageBoxA(NULL, format("Invalid dataType at line {} in " GameFile ".", lineCount).c_str(),
                "ThMouseX", MB_OK | MB_ICONERROR);
            return {move(dataType), false};
        }

        return {move(dataType), true};
    }

    static tuple<FloatPoint, bool> ExtractOffset(stringstream& stream, int lineCount) {
        string posOffsetStr;
        stream >> posOffsetStr;

        if (posOffsetStr[0] != '(' || posOffsetStr[posOffsetStr.length() - 1] != ')') {
            MessageBoxA(NULL, format("Invalid offset: expected wrapping '(' and ')' at line {} in " GameFile ".",
                lineCount).c_str(), "ThMouseX", MB_OK | MB_ICONERROR);
            return {FloatPoint(), false};
        }
        auto commaIdx = posOffsetStr.find(',');
        if (commaIdx == string::npos) {
            MessageBoxA(NULL, format("Invalid offset: expected separating comma ',' at line {} in " GameFile ".",
                lineCount).c_str(), "ThMouseX", MB_OK | MB_ICONERROR);
            return {FloatPoint(), false};
        }

        const char* convMessage;
        FloatPoint offset{};

        auto offsetXStr = posOffsetStr.substr(1, commaIdx - 1);
        tie(offset.X, convMessage) = helper::ConvertToFloat(offsetXStr);
        if (convMessage != nullptr) {
            MessageBoxA(NULL, format("Invalid offset X: {} at line {} in " GameFile ".",
                convMessage, lineCount).c_str(), "ThMouseX", MB_OK | MB_ICONERROR);
            return {FloatPoint(), false};
        }

        auto offsetYStr = posOffsetStr.substr(commaIdx + 1, posOffsetStr.length() - commaIdx - 2);
        tie(offset.Y, convMessage) = helper::ConvertToFloat(offsetYStr);
        if (convMessage != nullptr) {
            MessageBoxA(NULL, format("Invalid offset Y: {} at line {} in " GameFile ".",
                convMessage, lineCount).c_str(), "ThMouseX", MB_OK | MB_ICONERROR);
            return {FloatPoint(), false};
        }

        return {offset, true};
    }

    static tuple<DWORD, bool> ExtractBaseHeight(stringstream& stream, int lineCount) {
        DWORD baseHeight;
        stream >> dec >> baseHeight;

        if (baseHeight == 0) {
            MessageBoxA(NULL, format("Invalid baseHeight at line {} in " GameFile ".", lineCount).c_str(),
                "ThMouseX", MB_OK | MB_ICONERROR);
            return {baseHeight, false};
        }

        return {baseHeight, true};
    }

    static tuple<FloatPoint, bool> ExtractAspectRatio(stringstream& stream, int lineCount) {
        string aspectRatioStr;
        stream >> aspectRatioStr;

        auto colonIdx = aspectRatioStr.find(':');
        if (colonIdx == string::npos) {
            MessageBoxA(NULL, format("Invalid aspectRatio: expected separating ':' at line {} in " GameFile ".",
                lineCount).c_str(), "ThMouseX", MB_OK | MB_ICONERROR);
            return {FloatPoint(), false};
        }

        FloatPoint ratio;
        const char* convMessage;

        auto ratioXStr = aspectRatioStr.substr(0, colonIdx);
        tie(ratio.X, convMessage) = helper::ConvertToFloat(ratioXStr);
        if (convMessage != nullptr) {
            MessageBoxA(NULL, format("Invalid aspectRatio X: {} at line {} in " GameFile ".",
                convMessage, lineCount).c_str(), "ThMouseX", MB_OK | MB_ICONERROR);
            return {FloatPoint(), false};
        }

        auto ratioYStr = aspectRatioStr.substr(colonIdx + 1, aspectRatioStr.length() - colonIdx - 1);
        tie(ratio.Y, convMessage) = helper::ConvertToFloat(ratioYStr);
        if (convMessage != nullptr) {
            MessageBoxA(NULL, format("Invalid aspectRatio Y: {} at line {} in " GameFile ".",
                convMessage, lineCount).c_str(), "ThMouseX", MB_OK | MB_ICONERROR);
            return {FloatPoint(), false};
        }

        return {ratio, true};
    }

    static tuple<InputMethod, bool> ExtractInputMethod(stringstream& stream, int lineCount) {
        string inputMethodStr;
        stream >> inputMethodStr;

        auto inputMethods = InputMethod::None;
        auto inputMethodIter = strtok(inputMethodStr.data(), "/");
        while (inputMethodIter != NULL) {
            if (_stricmp(inputMethodIter, "HookAll") == 0)
                inputMethods |= InputMethod::DirectInput | InputMethod::GetKeyboardState;
            else if (_stricmp(inputMethodIter, "DirectInput") == 0)
                inputMethods |= InputMethod::DirectInput;
            else if (_stricmp(inputMethodIter, "GetKeyboardState") == 0)
                inputMethods |= InputMethod::GetKeyboardState;
            else if (_stricmp(inputMethodIter, "SendKey") == 0)
                inputMethods |= InputMethod::SendKey;
            inputMethodIter = strtok(NULL, "/");
        }

        if (inputMethods == InputMethod::None) {
            MessageBoxA(NULL, format("Invalid inputMethod at line {} in " GameFile ".", lineCount).c_str(),
                "ThMouseX", MB_OK | MB_ICONERROR);
            return {inputMethods, false};
        }

        return {inputMethods, true};
    }

    static tuple<VkCodes, bool> ReadVkCodes() {
        VkCodes vkCodes;
        int lineCount = 0;
        string line;

        ifstream vkcodeFile(VirtualKeyCodesFile);
        if (!vkcodeFile) {
            MessageBoxA(NULL, "Missing " VirtualKeyCodesFile " file.", "ThMouseX", MB_OK | MB_ICONERROR);
            return {move(vkCodes), false};
        }

        while (getline(vkcodeFile, line)) {
            lineCount++;
            stringstream lineStream(line);

            string key;
            lineStream >> key;
            if (key.empty() || key.starts_with(";"))
                continue;

            string valueStr;
            lineStream >> valueStr;
            auto [value, convMessage] = helper::ConvertToULong(valueStr, 0);
            if (convMessage != nullptr) {
                MessageBoxA(NULL, format("Invalid value: {} at line {} in " VirtualKeyCodesFile ".",
                    convMessage, lineCount).c_str(), "ThMouseX", MB_OK | MB_ICONERROR);
                return {move(vkCodes), false};
            }
            vkCodes[key] = value;
        }

        return {move(vkCodes), true};
    }
};