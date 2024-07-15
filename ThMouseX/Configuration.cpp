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
#define GameFile2 "Games2.txt"
#define VirtualKeyCodesFile "VirtualKeyCodes.txt"
#define ThMouseXFile APP_NAME ".ini"

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

using VkCodes = unordered_map<string, BYTE, string_hash, equal_to<>>;

#pragma region method declaration
bool IsCommentLine(stringstream& stream);
tuple<wstring, bool> ExtractProcessName(stringstream& stream, int lineCount, const char* gameConfigPath);
tuple<vector<DWORD>, ScriptType, bool> ExtractPositionRVA(stringstream& stream, int lineCount, const char* gameConfigPath);
tuple<PointDataType, bool> ExtractDataType(stringstream& stream, int lineCount, const char* gameConfigPath);
tuple<FloatPoint, bool> ExtractOffset(stringstream& stream, int lineCount, const char* gameConfigPath);
tuple<DWORD, bool> ExtractBaseHeight(stringstream& stream, int lineCount, const char* gameConfigPath);
tuple<FloatPoint, bool> ExtractAspectRatio(stringstream& stream, int lineCount, const char* gameConfigPath);
tuple<InputMethod, bool> ExtractInputMethod(stringstream& stream, int lineCount, const char* gameConfigPath);
tuple<VkCodes, bool> ReadVkCodes();
#pragma endregion

namespace core::configuration {
    bool ReadGamesFile(const char* gameConfigPath, bool overrding = false);
    bool ReadGamesFile() {
        auto rs = ReadGamesFile(GameFile);
        if (rs)
            rs = ReadGamesFile(GameFile2, true);
        return rs;
    }
    bool ReadGamesFile(const char* gameConfigPath, bool overrding) {
        ifstream gamesFile(gameConfigPath);
        if (!gamesFile) {
            if (!overrding) {
                MessageBoxA(nullptr, format("Missing {} file.", gameConfigPath).c_str(), APP_NAME, MB_OK | MB_ICONERROR);
                return false;
            }
            else
                return true;
        }
        if (!overrding)
            gs_gameConfigs.fill({});

        string line;
        int lineCount = 0;
        while (gs_gameConfigs.length() < gs_gameConfigs.capacity() && getline(gamesFile, line)) {
            lineCount++;
            stringstream lineStream(line);
            if (IsCommentLine(lineStream))
                continue;

            auto [processName, ok1] = ExtractProcessName(lineStream, lineCount, gameConfigPath);
            if (!ok1)
                return false;

            auto [addressOffsets, scriptType, ok2] = ExtractPositionRVA(lineStream, lineCount, gameConfigPath);
            if (!ok2)
                return false;

            auto [dataType, ok3] = ExtractDataType(lineStream, lineCount, gameConfigPath);
            if (!ok3)
                return false;

            auto [offset, ok4] = ExtractOffset(lineStream, lineCount, gameConfigPath);
            if (!ok4)
                return false;

            auto [baseHeight, ok5] = ExtractBaseHeight(lineStream, lineCount, gameConfigPath);
            if (!ok5)
                return false;

            auto [aspectRatio, ok6] = ExtractAspectRatio(lineStream, lineCount, gameConfigPath);
            if (!ok6)
                return false;

            auto [inputMethods, ok7] = ExtractInputMethod(lineStream, lineCount, gameConfigPath);
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
            MessageBoxA(nullptr, "Missing " ThMouseXFile " file.", APP_NAME, MB_OK | MB_ICONERROR);
            return false;
        }

        inipp::Ini<char> ini;
        ini.parse(iniFile);
        if (!ini.errors.empty()) {
            auto const& invalidLine = *ini.errors.begin();
            MessageBoxA(nullptr, (ThMouseXFile ": Invalid syntax: \"" + invalidLine + "\"").c_str(), APP_NAME, MB_OK | MB_ICONERROR);
            return false;
        }

        ini.strip_trailing_comments();
        auto const& defaultSection = ini.sections[""];

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

tuple<wstring, bool> ExtractProcessName(stringstream& stream, int lineCount, const char* gameConfigPath) {
    string processName;
    stream >> quoted(processName);
    auto wProcessName = encoding::ConvertToUtf16(processName.c_str());

    auto maxSize = ARRAYSIZE(gs_gameConfigs[0].ProcessName) - 1;
    if (wProcessName.size() > maxSize) {
        MessageBoxA(nullptr, format("processName longer than {} characters at line {} in {}.",
            maxSize, lineCount, gameConfigPath).c_str(), APP_NAME, MB_OK | MB_ICONERROR);
        return { move(wProcessName), false };
    }
    return { move(wProcessName), true };
}

tuple<vector<DWORD>, ScriptType, bool> ExtractPositionRVA(stringstream& stream, int lineCount, const char* gameConfigPath) {
    string pointerChainStr;
    stream >> pointerChainStr;
    vector<DWORD> addressOffsets;

    auto scriptType = ScriptType::None;

    if (pointerChainStr.compare("LuaJIT") == 0)
        scriptType = ScriptType::LuaJIT;
    else if (pointerChainStr.compare("NeoLua") == 0)
        scriptType = ScriptType::NeoLua;
    else if (pointerChainStr.compare("Lua") == 0)
        scriptType = ScriptType::Lua;

    if (scriptType == ScriptType::None) {
        auto maxSize = ARRAYSIZE(gs_gameConfigs[0].Address.Level);
        addressOffsets.reserve(maxSize);
        size_t leftBoundIdx = 0;
        size_t rightBoundIdx = -1;
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
                MessageBoxA(nullptr, format("Invalid positionRVA: {} at line {} in {}.",
                    convMessage, lineCount, gameConfigPath).c_str(), APP_NAME, MB_OK | MB_ICONERROR);
                return { move(addressOffsets), scriptType, false };
            }

            addressOffsets.push_back(offset);
        }

        if (addressOffsets.empty()) {
            MessageBoxA(nullptr, format("Found no address offset for positionRVA at line {} in {}.",
                lineCount, gameConfigPath).c_str(), APP_NAME, MB_OK | MB_ICONERROR);
            return { move(addressOffsets), scriptType, false };
        }
    }

    return { move(addressOffsets), scriptType, true };
}

tuple<PointDataType, bool> ExtractDataType(stringstream& stream, int lineCount, const char* gameConfigPath) {
    using enum PointDataType;
    string dataTypeStr;
    stream >> dataTypeStr;
    auto dataType = None;

    if (_stricmp(dataTypeStr.c_str(), "Int") == 0)
        dataType = Int;
    else if (_stricmp(dataTypeStr.c_str(), "Float") == 0)
        dataType = Float;
    else if (_stricmp(dataTypeStr.c_str(), "Short") == 0)
        dataType = Short;
    else if (_stricmp(dataTypeStr.c_str(), "Double") == 0)
        dataType = Double;
    else {
        MessageBoxA(nullptr, format("Invalid dataType at line {} in {}.", lineCount, gameConfigPath).c_str(),
            APP_NAME, MB_OK | MB_ICONERROR);
        return { move(dataType), false };
    }

    return { move(dataType), true };
}

tuple<FloatPoint, bool> ExtractOffset(stringstream& stream, int lineCount, const char* gameConfigPath) {
    string posOffsetStr;
    stream >> posOffsetStr;

    if (posOffsetStr[0] != '(' || posOffsetStr[posOffsetStr.length() - 1] != ')') {
        MessageBoxA(nullptr , format("Invalid offset: expected wrapping '(' and ')' at line {} in {}.",
            lineCount, gameConfigPath).c_str(), APP_NAME, MB_OK | MB_ICONERROR);
        return { FloatPoint(), false };
    }
    auto commaIdx = posOffsetStr.find(',');
    if (commaIdx == string::npos) {
        MessageBoxA(nullptr, format("Invalid offset: expected separating comma ',' at line {} in {}.",
            lineCount, gameConfigPath).c_str(), APP_NAME, MB_OK | MB_ICONERROR);
        return { FloatPoint(), false };
    }

    auto offsetXStr = posOffsetStr.substr(1, commaIdx - 1);
    auto [offsetX, convMessage] = helper::ConvertToFloat(offsetXStr);
    if (convMessage != nullptr) {
        MessageBoxA(nullptr, format("Invalid offset X: {} at line {} in {}.",
            convMessage, lineCount, gameConfigPath).c_str(), APP_NAME, MB_OK | MB_ICONERROR);
        return { FloatPoint(), false };
    }

    auto offsetYStr = posOffsetStr.substr(commaIdx + 1, posOffsetStr.length() - commaIdx - 2);
    auto [offsetY, convMessage2] = helper::ConvertToFloat(offsetYStr);
    if (convMessage2 != nullptr) {
        MessageBoxA(nullptr, format("Invalid offset Y: {} at line {} in {}.",
            convMessage2, lineCount, gameConfigPath).c_str(), APP_NAME, MB_OK | MB_ICONERROR);
        return { FloatPoint(), false };
    }

    return { FloatPoint{offsetX, offsetY}, true };
}

tuple<DWORD, bool> ExtractBaseHeight(stringstream& stream, int lineCount, const char* gameConfigPath) {
    DWORD baseHeight;
    stream >> dec >> baseHeight;

    if (baseHeight == 0) {
        MessageBoxA(nullptr, format("Invalid baseHeight at line {} in {}.", lineCount, gameConfigPath).c_str(),
            APP_NAME, MB_OK | MB_ICONERROR);
        return { baseHeight, false };
    }

    return { baseHeight, true };
}

tuple<FloatPoint, bool> ExtractAspectRatio(stringstream& stream, int lineCount, const char* gameConfigPath) {
    string aspectRatioStr;
    stream >> aspectRatioStr;

    auto colonIdx = aspectRatioStr.find(':');
    if (colonIdx == string::npos) {
        MessageBoxA(nullptr, format("Invalid aspectRatio: expected separating ':' at line {} in {}.",
            lineCount, gameConfigPath).c_str(), APP_NAME, MB_OK | MB_ICONERROR);
        return { FloatPoint(), false };
    }

    auto ratioXStr = aspectRatioStr.substr(0, colonIdx);
    auto [ratioX, convMessage] = helper::ConvertToFloat(ratioXStr);
    if (convMessage != nullptr) {
        MessageBoxA(nullptr, format("Invalid aspectRatio X: {} at line {} in {}.",
            convMessage, lineCount, gameConfigPath).c_str(), APP_NAME, MB_OK | MB_ICONERROR);
        return { FloatPoint(), false };
    }

    auto ratioYStr = aspectRatioStr.substr(colonIdx + 1, aspectRatioStr.length() - colonIdx - 1);
    auto [ratioY, convMessage2] = helper::ConvertToFloat(ratioYStr);
    if (convMessage2 != nullptr) {
        MessageBoxA(nullptr, format("Invalid aspectRatio Y: {} at line {} in {}.",
            convMessage2, lineCount, gameConfigPath).c_str(), APP_NAME, MB_OK | MB_ICONERROR);
        return { FloatPoint(), false };
    }

    return { FloatPoint{ratioX, ratioY}, true };
}

tuple<InputMethod, bool> ExtractInputMethod(stringstream& stream, int lineCount, const char* gameConfigPath) {
    using enum InputMethod;
    string inputMethodStr;
    stream >> inputMethodStr;

    auto inputMethods = None;
    char* nextToken{};
    auto token = strtok_s(inputMethodStr.data(), "/", &nextToken);
    while (token) {
        if (_stricmp(token, "DirectInput") == 0)
            inputMethods |= DirectInput;
        else if (_stricmp(token, "GetKeyboardState") == 0)
            inputMethods |= GetKeyboardState;
        else if (_stricmp(token, "SendInput") == 0)
            inputMethods |= SendInput;
        else if (_stricmp(token, "SendMessage") == 0)
            inputMethods |= SendMsg;
        token = strtok_s(nullptr, "/", &nextToken);
    }

    if (inputMethods == None) {
        MessageBoxA(nullptr, format("Invalid inputMethod at line {} in {}.", lineCount, gameConfigPath).c_str(),
            APP_NAME, MB_OK | MB_ICONERROR);
        return { inputMethods, false };
    }

    return { inputMethods, true };
}

tuple<VkCodes, bool> ReadVkCodes() {
    VkCodes vkCodes;
    int lineCount = 0;
    string line;

    ifstream vkcodeFile(VirtualKeyCodesFile);
    if (!vkcodeFile) {
        MessageBoxA(nullptr, "Missing " VirtualKeyCodesFile " file.", APP_NAME, MB_OK | MB_ICONERROR);
        return { move(vkCodes), false };
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
            MessageBoxA(nullptr, format("Invalid value: {} at line {} in " VirtualKeyCodesFile ".",
                convMessage, lineCount).c_str(), APP_NAME, MB_OK | MB_ICONERROR);
            return { move(vkCodes), false };
        }
        vkCodes[key] = (BYTE)value;
    }

    return { move(vkCodes), true };
}