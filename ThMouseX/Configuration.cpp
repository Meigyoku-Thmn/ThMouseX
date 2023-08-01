#include "framework.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <unordered_map>
#include <format>
#include <tuple>
#include <cassert>
#include <inipp.h>

#include "../Common/macro.h"
#include "../Common/Variables.h"
#include "../Common/Helper.h"
#include "../Common/Helper.Encoding.h"
#include "Direct3D8.h"
#include "Direct3D9.h"
#include "Direct3D11.h"
#include "DirectInput.h"
#include "Configuration.h"

namespace helper = common::helper;
namespace encoding = common::helper::encoding;
namespace directx8 = core::directx8;
namespace directx9 = core::directx9;
namespace directx11 = core::directx11;
namespace directinput = core::directinput;

#define GameFile "Games.txt"
#define ThMouseXFile APP_NAME ".ini"
#define VirtualKeyCodesFile "VirtualKeyCodes.txt"

#define INI_GET_BUTTON(section, ini_key, buttonNames, output) INI_GET_BUTTON_IMPL(__COUNTER__, section, ini_key, buttonNames, output)
#define INI_GET_BUTTON_IMPL(counter, section, ini_key, buttonNames, output) \
std::string MAKE_UNIQUE_VAR(counter); \
if (!inipp::get_value(section, ini_key, MAKE_UNIQUE_VAR(counter))) { \
        MessageBoxA(NULL, ThMouseXFile ": Missing " ini_key " value.", APP_NAME, MB_OK | MB_ICONERROR); \
        return false; \
} \
else { \
    auto vkCode = buttonNames.find(MAKE_UNIQUE_VAR(counter)); \
    if (vkCode == buttonNames.end()) { \
        MessageBoxA(NULL, ThMouseXFile ": Invalid " ini_key " value.", APP_NAME, MB_OK | MB_ICONERROR); \
        return false; \
    } \
    output = vkCode->second; \
}0

#define INI_GET_WSTR_PATH(section, ini_key, output) INI_GET_WSTR_PATH_IMPL(__COUNTER__, section, ini_key, output)
#define INI_GET_WSTR_PATH_IMPL(counter, section, ini_key, output) \
std::string MAKE_UNIQUE_VAR(counter); \
if (!inipp::get_value(section, ini_key, MAKE_UNIQUE_VAR(counter))) { \
    MessageBoxA(NULL, ThMouseXFile ": Missing " ini_key " value.", APP_NAME, MB_OK | MB_ICONERROR); \
    return false; \
} \
else { \
    if (MAKE_UNIQUE_VAR(counter).size() == 0) { \
        MessageBoxA(NULL, ThMouseXFile ": Invalid " ini_key " value.", APP_NAME, MB_OK | MB_ICONERROR); \
        return false; \
    } \
    auto wStr = encoding::ConvertToUtf16(MAKE_UNIQUE_VAR(counter).c_str()); \
    GetFullPathNameW(wStr.c_str(), ARRAYSIZE(output), output, NULL); \
}0 \

#define INI_GET_ULONG(section, ini_key, output) INI_GET_ULONG_IMPL(__COUNTER__, section, ini_key, output)
#define INI_GET_ULONG_IMPL(counter, section, ini_key, output) \
std::string MAKE_UNIQUE_VAR(counter); \
if (!inipp::get_value(section, ini_key, MAKE_UNIQUE_VAR(counter))) { \
    MessageBoxA(NULL, ThMouseXFile ": Missing " ini_key " value.", APP_NAME, MB_OK | MB_ICONERROR); \
    return false; \
} \
else { \
    auto [value, convMessage] = helper::ConvertToULong(MAKE_UNIQUE_VAR(counter), 10); \
    if (convMessage != nullptr) { \
        MessageBoxA(NULL, std::format(ThMouseXFile ": Invalid " ini_key " value: {}.", convMessage).c_str(), \
            APP_NAME, MB_OK | MB_ICONERROR); \
        return false; \
    } \
    output = value; \
}0

using namespace std;

typedef unordered_map<string, BYTE, string_hash, equal_to<>> VkCodes;

#pragma region method declaration
bool IsCommentLine(stringstream& stream);
tuple<wstring, bool> ExtractProcessName(stringstream& stream, int lineCount);
tuple<vector<DWORD>, ScriptType, ScriptRunPlace, ScriptPositionGetMethod, bool> ExtractPositionRVA(stringstream& stream, int lineCount);
tuple<PointDataType, bool> ExtractDataType(stringstream& stream, int lineCount);
tuple<FloatPoint, bool> ExtractOffset(stringstream& stream, int lineCount);
tuple<DWORD, bool> ExtractBaseHeight(stringstream& stream, int lineCount);
tuple<FloatPoint, bool> ExtractAspectRatio(stringstream& stream, int lineCount);
tuple<InputMethod, bool> ExtractInputMethod(stringstream& stream, int lineCount);
tuple<VkCodes, bool> ReadVkCodes();
#pragma endregion

namespace core::configuration {
    bool ReadGamesFile() {
        ifstream gamesFile(GameFile);
        if (!gamesFile) {
            MessageBoxA(NULL, "Missing " GameFile " file.", APP_NAME, MB_OK | MB_ICONERROR);
            return false;
        }
        gs_gameConfigs.fill({});

        string line;
        int lineCount = 0;
        while (gs_gameConfigs.length() < gs_gameConfigs.capacity() && getline(gamesFile, line)) {
            lineCount++;
            stringstream lineStream(line);
            if (IsCommentLine(lineStream))
                continue;

            auto [processName, ok1] = ExtractProcessName(lineStream, lineCount);
            if (!ok1)
                return false;

            auto [addressOffsets, scriptType, scriptRunPlace, scriptPosGetMethod, ok2] = ExtractPositionRVA(lineStream, lineCount);
            if (!ok2)
                return false;

            auto [dataType, ok3] = ExtractDataType(lineStream, lineCount);
            if (!ok3)
                return false;

            auto [offset, ok4] = ExtractOffset(lineStream, lineCount);
            if (!ok4)
                return false;

            auto [baseHeight, ok5] = ExtractBaseHeight(lineStream, lineCount);
            if (!ok5)
                return false;

            auto [aspectRatio, ok6] = ExtractAspectRatio(lineStream, lineCount);
            if (!ok6)
                return false;

            auto [inputMethods, ok7] = ExtractInputMethod(lineStream, lineCount);
            if (!ok7)
                return false;

            auto& gameConfig = gs_gameConfigs.add_new();

            static_assert(is_same<decltype(&gameConfig.ProcessName[0]), decltype(processName.data())>());
            memcpy(gameConfig.ProcessName, processName.c_str(), processName.size() * sizeof(processName[0]));

            static_assert(is_same<decltype(&gameConfig.Address.Level[0]), decltype(addressOffsets.data())>());
            if (addressOffsets.size() > 0) {
                memcpy(gameConfig.Address.Level, addressOffsets.data(), addressOffsets.size() * sizeof(addressOffsets[0]));
                assert(addressOffsets.size() <= ARRAYSIZE(gameConfig.Address.Level));
                gameConfig.Address.Length = addressOffsets.size();
            }
            gameConfig.ScriptType = scriptType;
            gameConfig.ScriptRunPlace = scriptRunPlace;
            gameConfig.ScriptPositionGetMethod = scriptPosGetMethod;

            gameConfig.PosDataType = dataType;

            gameConfig.BasePixelOffset = offset;

            static_assert(is_same<decltype(gameConfig.BaseHeight), decltype(baseHeight)>());
            gameConfig.BaseHeight = baseHeight;

            gameConfig.AspectRatio = aspectRatio;

            gameConfig.InputMethods = inputMethods;
        }

        return true;
    }

    bool ReadGeneralConfigFile() {
        auto [vkCodes, succeeded] = ReadVkCodes();
        if (!succeeded)
            return false;

        ifstream iniFile(ThMouseXFile);
        if (!iniFile) {
            MessageBoxA(NULL, "Missing " ThMouseXFile " file.", APP_NAME, MB_OK | MB_ICONERROR);
            return false;
        }

        inipp::Ini<char> ini;
        ini.parse(iniFile);
        if (ini.errors.size() > 0) {
            auto& invalidLine = *ini.errors.begin();
            MessageBoxA(NULL, (ThMouseXFile ": Invalid syntax: \"" + invalidLine + "\"").c_str(), APP_NAME, MB_OK | MB_ICONERROR);
            return false;
        }

        ini.strip_trailing_comments();
        auto& defaultSection = ini.sections[""];

        INI_GET_BUTTON(defaultSection, "BombButton", vkCodes, gs_bombButton);
        INI_GET_BUTTON(defaultSection, "ExtraButton", vkCodes, gs_extraButton);

        INI_GET_BUTTON(defaultSection, "ToggleOsCursorButton", vkCodes, gs_toggleOsCursorButton);
        INI_GET_BUTTON(defaultSection, "ToggleImGuiButton", vkCodes, gs_toggleImGuiButton);

        INI_GET_WSTR_PATH(defaultSection, "ImGuiFontPath", gs_imGuiFontPath);
        INI_GET_ULONG(defaultSection, "ImGuiBaseFontSize", gs_imGuiBaseFontSize);
        INI_GET_ULONG(defaultSection, "ImGuiBaseVerticalResolution", gs_imGuiBaseVerticalResolution);

        INI_GET_WSTR_PATH(defaultSection, "CursorTexture", gs_textureFilePath);
        INI_GET_ULONG(defaultSection, "CursorBaseHeight", gs_textureBaseHeight);

        return true;
    }
}

bool IsCommentLine(stringstream& stream) {
    char firstChr{};
    stream >> ws >> firstChr;
    if (firstChr == ';' || firstChr == '\0')
        return true;

    stream.seekg(-1, ios_base::cur);
    return false;
}

tuple<wstring, bool> ExtractProcessName(stringstream& stream, int lineCount) {
    string processName;
    stream >> quoted(processName);
    auto wProcessName = encoding::ConvertToUtf16(processName.c_str());

    auto maxSize = ARRAYSIZE(gs_gameConfigs[0].ProcessName) - 1;
    if (wProcessName.size() > maxSize) {
        MessageBoxA(NULL, format("processName longer than {} characters at line {} in " GameFile ".",
            maxSize, lineCount).c_str(), APP_NAME, MB_OK | MB_ICONERROR);
        return {move(wProcessName), false};
    }
    return {move(wProcessName), true};
}

tuple<vector<DWORD>, ScriptType, ScriptRunPlace, ScriptPositionGetMethod, bool>
ExtractPositionRVA(stringstream& stream, int lineCount) {
    string pointerChainStr;
    stream >> pointerChainStr;
    vector<DWORD> addressOffsets;

    auto scriptType = ScriptType::None;
    auto scriptRunPlace = ScriptRunPlace::None;
    auto scriptPosGetMethod = ScriptPositionGetMethod::None;

    auto scriptingConfig = string(pointerChainStr);
    auto tok = strtok(scriptingConfig.data(), "/");
    const char* segments[3]{"", "", ""};
    auto segPos = 0;
    while (tok != NULL) {
        if (tok[0] != '\0')
            segments[segPos] = tok;
        segPos++;
        tok = strtok(NULL, "/");
        if (segPos == 3)
            break;
    }

    if (_stricmp(segments[0], "LuaJIT") == 0)
        scriptType = ScriptType::LuaJIT;
    else if (_stricmp(segments[0], "NeoLua") == 0)
        scriptType = ScriptType::NeoLua;
    else if (_stricmp(segments[0], "Lua") == 0)
        scriptType = ScriptType::Lua;

    if (_stricmp(segments[1], "Detached") == 0)
        scriptRunPlace = ScriptRunPlace::Detached;
    else if (_stricmp(segments[1], "Attached") == 0)
        scriptRunPlace = ScriptRunPlace::Attached;

    if (_stricmp(segments[2], "Pull") == 0)
        scriptPosGetMethod = ScriptPositionGetMethod::Pull;
    else if (_stricmp(segments[2], "Push") == 0)
        scriptPosGetMethod = ScriptPositionGetMethod::Push;

    if (scriptType == ScriptType::None) {
        auto maxSize = ARRAYSIZE(gs_gameConfigs[0].Address.Level);
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
                    convMessage, lineCount).c_str(), APP_NAME, MB_OK | MB_ICONERROR);
                return {move(addressOffsets), scriptType, scriptRunPlace, scriptPosGetMethod, false};
            }

            addressOffsets.push_back(offset);
        }

        if (addressOffsets.size() == 0) {
            MessageBoxA(NULL, format("Found no address offset for positionRVA at line {} in " GameFile ".",
                lineCount).c_str(), APP_NAME, MB_OK | MB_ICONERROR);
            return {move(addressOffsets), scriptType, scriptRunPlace, scriptPosGetMethod, false};
        }
    }

    return {move(addressOffsets), scriptType, scriptRunPlace, scriptPosGetMethod, true};
}

tuple<PointDataType, bool> ExtractDataType(stringstream& stream, int lineCount) {
    string dataTypeStr;
    stream >> dataTypeStr;
    auto dataType = PointDataType::None;

    if (_stricmp(dataTypeStr.c_str(), "Int") == 0)
        dataType = PointDataType::Int;
    else if (_stricmp(dataTypeStr.c_str(), "Float") == 0)
        dataType = PointDataType::Float;
    else if (_stricmp(dataTypeStr.c_str(), "Short") == 0)
        dataType = PointDataType::Short;
    else if (_stricmp(dataTypeStr.c_str(), "Double") == 0)
        dataType = PointDataType::Double;
    else {
        MessageBoxA(NULL, format("Invalid dataType at line {} in " GameFile ".", lineCount).c_str(),
            APP_NAME, MB_OK | MB_ICONERROR);
        return {move(dataType), false};
    }

    return {move(dataType), true};
}

tuple<FloatPoint, bool> ExtractOffset(stringstream& stream, int lineCount) {
    string posOffsetStr;
    stream >> posOffsetStr;

    if (posOffsetStr[0] != '(' || posOffsetStr[posOffsetStr.length() - 1] != ')') {
        MessageBoxA(NULL, format("Invalid offset: expected wrapping '(' and ')' at line {} in " GameFile ".",
            lineCount).c_str(), APP_NAME, MB_OK | MB_ICONERROR);
        return {FloatPoint(), false};
    }
    auto commaIdx = posOffsetStr.find(',');
    if (commaIdx == string::npos) {
        MessageBoxA(NULL, format("Invalid offset: expected separating comma ',' at line {} in " GameFile ".",
            lineCount).c_str(), APP_NAME, MB_OK | MB_ICONERROR);
        return {FloatPoint(), false};
    }

    auto offsetXStr = posOffsetStr.substr(1, commaIdx - 1);
    auto [offsetX, convMessage] = helper::ConvertToFloat(offsetXStr);
    if (convMessage != nullptr) {
        MessageBoxA(NULL, format("Invalid offset X: {} at line {} in " GameFile ".",
            convMessage, lineCount).c_str(), APP_NAME, MB_OK | MB_ICONERROR);
        return {FloatPoint(), false};
    }

    auto offsetYStr = posOffsetStr.substr(commaIdx + 1, posOffsetStr.length() - commaIdx - 2);
    auto [offsetY, convMessage2] = helper::ConvertToFloat(offsetYStr);
    if (convMessage2 != nullptr) {
        MessageBoxA(NULL, format("Invalid offset Y: {} at line {} in " GameFile ".",
            convMessage2, lineCount).c_str(), APP_NAME, MB_OK | MB_ICONERROR);
        return {FloatPoint(), false};
    }

    return {FloatPoint(offsetX, offsetY), true};
}

tuple<DWORD, bool> ExtractBaseHeight(stringstream& stream, int lineCount) {
    DWORD baseHeight;
    stream >> dec >> baseHeight;

    if (baseHeight == 0) {
        MessageBoxA(NULL, format("Invalid baseHeight at line {} in " GameFile ".", lineCount).c_str(),
            APP_NAME, MB_OK | MB_ICONERROR);
        return {baseHeight, false};
    }

    return {baseHeight, true};
}

tuple<FloatPoint, bool> ExtractAspectRatio(stringstream& stream, int lineCount) {
    string aspectRatioStr;
    stream >> aspectRatioStr;

    auto colonIdx = aspectRatioStr.find(':');
    if (colonIdx == string::npos) {
        MessageBoxA(NULL, format("Invalid aspectRatio: expected separating ':' at line {} in " GameFile ".",
            lineCount).c_str(), APP_NAME, MB_OK | MB_ICONERROR);
        return {FloatPoint(), false};
    }

    auto ratioXStr = aspectRatioStr.substr(0, colonIdx);
    auto [ratioX, convMessage] = helper::ConvertToFloat(ratioXStr);
    if (convMessage != nullptr) {
        MessageBoxA(NULL, format("Invalid aspectRatio X: {} at line {} in " GameFile ".",
            convMessage, lineCount).c_str(), APP_NAME, MB_OK | MB_ICONERROR);
        return {FloatPoint(), false};
    }

    auto ratioYStr = aspectRatioStr.substr(colonIdx + 1, aspectRatioStr.length() - colonIdx - 1);
    auto [ratioY, convMessage2] = helper::ConvertToFloat(ratioYStr);
    if (convMessage2 != nullptr) {
        MessageBoxA(NULL, format("Invalid aspectRatio Y: {} at line {} in " GameFile ".",
            convMessage2, lineCount).c_str(), APP_NAME, MB_OK | MB_ICONERROR);
        return {FloatPoint(), false};
    }

    return {FloatPoint(ratioX, ratioY), true};
}

tuple<InputMethod, bool> ExtractInputMethod(stringstream& stream, int lineCount) {
    string inputMethodStr;
    stream >> inputMethodStr;

    auto inputMethods = InputMethod::None;
    auto inputMethodIter = strtok(inputMethodStr.data(), "/");
    while (inputMethodIter != NULL) {
        if (_stricmp(inputMethodIter, "DirectInput") == 0)
            inputMethods |= InputMethod::DirectInput;
        else if (_stricmp(inputMethodIter, "GetKeyboardState") == 0)
            inputMethods |= InputMethod::GetKeyboardState;
        else if (_stricmp(inputMethodIter, "SendKey") == 0)
            inputMethods |= InputMethod::SendKey;
        inputMethodIter = strtok(NULL, "/");
    }

    if (inputMethods == InputMethod::None) {
        MessageBoxA(NULL, format("Invalid inputMethod at line {} in " GameFile ".", lineCount).c_str(),
            APP_NAME, MB_OK | MB_ICONERROR);
        return {inputMethods, false};
    }

    return {inputMethods, true};
}

tuple<VkCodes, bool> ReadVkCodes() {
    VkCodes vkCodes;
    int lineCount = 0;
    string line;

    ifstream vkcodeFile(VirtualKeyCodesFile);
    if (!vkcodeFile) {
        MessageBoxA(NULL, "Missing " VirtualKeyCodesFile " file.", APP_NAME, MB_OK | MB_ICONERROR);
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
                convMessage, lineCount).c_str(), APP_NAME, MB_OK | MB_ICONERROR);
            return {move(vkCodes), false};
        }
        vkCodes[key] = (BYTE)value;
    }

    return {move(vkCodes), true};
}