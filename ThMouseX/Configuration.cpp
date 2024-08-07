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
#include "../Common/Log.h"
#include "../Common/Helper.h"
#include "../Common/Helper.Encoding.h"
#include "Direct3D8.h"
#include "Direct3D9.h"
#include "Direct3D11.h"
#include "DirectInput.h"
#include "Configuration.h"

namespace note = common::log;
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

using namespace std;

using VkCodes = unordered_map<string, BYTE, string_hash, equal_to<>>;

template<CompileTimeString ini_key, typename OutputType>
static bool IniTryGetButton(const inipp::Ini<char>::Section& section, const VkCodes& buttonNames, OutputType output) {
    string input;
    if (!inipp::get_value(section, ini_key.data, input)) {
        constexpr auto msg = ThMouseXFile ": Missing " + ini_key + " value.";
        MessageBoxA(nil, msg.data, APP_NAME, MB_OK | MB_ICONERROR);
        return false;
    }
    else {
        auto vkCode = buttonNames.find(input);
        if (vkCode == buttonNames.end()) {
            constexpr auto msg = ThMouseXFile ": Invalid " + ini_key + " value.";
            MessageBoxA(nil, msg.data, APP_NAME, MB_OK | MB_ICONERROR);
            return false;
        }
        output = vkCode->second;
    }
    return true;
}

template<CompileTimeString ini_key, std::size_t output_len>
static bool IniTryGetWstrPath(const inipp::Ini<char>::Section& section, WCHAR(&output)[output_len]) {
    string input;
    if (!inipp::get_value(section, ini_key.data, input)) {
        constexpr auto msg = ThMouseXFile ": Missing " + ini_key + " value.";
        MessageBoxA(nil, msg.data, APP_NAME, MB_OK | MB_ICONERROR);
        return false;
    }
    else {
        if (input.size() == 0) {
            constexpr auto msg = ThMouseXFile ": Invalid " + ini_key + " value.";
            MessageBoxA(nil, msg.data, APP_NAME, MB_OK | MB_ICONERROR);
            return false;
        }
        auto wStr = encoding::ConvertToUtf16(input.c_str());
        GetFullPathNameW(wStr.c_str(), output_len, output, nil);
    }
    return true;
}

template<CompileTimeString ini_key>
static bool IniTryGetULong(const inipp::Ini<char>::Section& section, DWORD& output) {
    string input;
    if (!inipp::get_value(section, ini_key.data, input)) {
        constexpr auto msg = ThMouseXFile ": Missing " + ini_key + " value.";
        MessageBoxA(nil, msg.data, APP_NAME, MB_OK | MB_ICONERROR);
        return false;
    }
    else {
        auto [value, convMessage] = helper::ConvertToULong(input, 10);
        if (convMessage != nil) {
            constexpr auto fmt = ThMouseXFile ": Invalid " + ini_key + " value: {}.";
            auto msg = vformat(fmt.data, make_format_args(convMessage));
            MessageBoxA(nil, msg.c_str(), APP_NAME, MB_OK | MB_ICONERROR);
            return false;
        }
        output = value;
    }
    return true;
}

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
    bool MarkThMouseXProcess() {
        auto rs = _putenv(APP_NAME "=" APP_NAME);
        if (rs != 0)
            note::ToFile("[Configuration] MarkThMouseXProcess failed.");
        return true;
    }
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
                MessageBoxA(nil, format("Missing {} file.", gameConfigPath).c_str(), APP_NAME, MB_OK | MB_ICONERROR);
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
            MessageBoxA(nil, "Missing " ThMouseXFile " file.", APP_NAME, MB_OK | MB_ICONERROR);
            return false;
        }

        inipp::Ini<char> ini;
        ini.parse(iniFile);
        if (!ini.errors.empty()) {
            auto const& invalidLine = *ini.errors.begin();
            MessageBoxA(nil, (ThMouseXFile ": Invalid syntax: \"" + invalidLine + "\"").c_str(), APP_NAME, MB_OK | MB_ICONERROR);
            return false;
        }

        ini.strip_trailing_comments();
        auto const& defaultSection = ini.sections[""];

        if (!IniTryGetButton<"LeftClick">(defaultSection, vkCodes, gs_vkCodeForLeftClick)) return false;
        if (!IniTryGetButton<"MiddleClick">(defaultSection, vkCodes, gs_vkCodeForMiddleClick)) return false;
        if (!IniTryGetButton<"RightClick">(defaultSection, vkCodes, gs_vkCodeForRightClick)) return false;
        if (!IniTryGetButton<"ForwardClick">(defaultSection, vkCodes, gs_vkCodeForForwardClick)) return false;
        if (!IniTryGetButton<"BackwardClick">(defaultSection, vkCodes, gs_vkCodeForBackwardClick)) return false;
        if (!IniTryGetButton<"ScrollUp">(defaultSection, vkCodes, gs_vkCodeForScrollUp)) return false;
        if (!IniTryGetButton<"ScrollDown">(defaultSection, vkCodes, gs_vkCodeForScrollDown)) return false;
        if (!IniTryGetButton<"ScrollLeft">(defaultSection, vkCodes, gs_vkCodeForScrollLeft)) return false;
        if (!IniTryGetButton<"ScrollRight">(defaultSection, vkCodes, gs_vkCodeForScrollRight)) return false;

        if (!IniTryGetButton<"ToggleMouseControl">(defaultSection, vkCodes, gs_toggleMouseControl)) return false;
        if (!IniTryGetButton<"ToggleOsCursorButton">(defaultSection, vkCodes, gs_toggleOsCursorButton)) return false;
        if (!IniTryGetButton<"ToggleImGuiButton">(defaultSection, vkCodes, gs_toggleImGuiButton)) return false;

        if (!IniTryGetWstrPath<"ImGuiFontPath">(defaultSection, gs_imGuiFontPath)) return false;
        if (!IniTryGetULong<"ImGuiBaseFontSize">(defaultSection, gs_imGuiBaseFontSize)) return false;
        if (!IniTryGetULong<"ImGuiBaseVerticalResolution">(defaultSection, gs_imGuiBaseVerticalResolution)) return false;

        if (!IniTryGetWstrPath<"CursorTexture">(defaultSection, gs_textureFilePath)) return false;
        if (!IniTryGetULong<"CursorBaseHeight">(defaultSection, gs_textureBaseHeight)) return false;

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
    auto wProcessName = encoding::ConvertToUtf16(processName);

    auto maxSize = ARRAYSIZE(gs_gameConfigs[0].ProcessName) - 1;
    if (wProcessName.size() > maxSize) {
        MessageBoxA(nil, format("processName longer than {} characters at line {} in {}.",
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
            if (convMessage != nil) {
                MessageBoxA(nil, format("Invalid positionRVA: {} at line {} in {}.",
                    convMessage, lineCount, gameConfigPath).c_str(), APP_NAME, MB_OK | MB_ICONERROR);
                return { move(addressOffsets), scriptType, false };
            }

            addressOffsets.push_back(offset);
        }

        if (addressOffsets.empty()) {
            MessageBoxA(nil, format("Found no address offset for positionRVA at line {} in {}.",
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
        MessageBoxA(nil, format("Invalid dataType at line {} in {}.", lineCount, gameConfigPath).c_str(),
            APP_NAME, MB_OK | MB_ICONERROR);
        return { move(dataType), false };
    }

    return { move(dataType), true };
}

tuple<FloatPoint, bool> ExtractOffset(stringstream& stream, int lineCount, const char* gameConfigPath) {
    string posOffsetStr;
    stream >> posOffsetStr;

    if (posOffsetStr[0] != '(' || posOffsetStr[posOffsetStr.length() - 1] != ')') {
        MessageBoxA(nil, format("Invalid offset: expected wrapping '(' and ')' at line {} in {}.",
            lineCount, gameConfigPath).c_str(), APP_NAME, MB_OK | MB_ICONERROR);
        return { FloatPoint(), false };
    }
    auto commaIdx = posOffsetStr.find(',');
    if (commaIdx == string::npos) {
        MessageBoxA(nil, format("Invalid offset: expected separating comma ',' at line {} in {}.",
            lineCount, gameConfigPath).c_str(), APP_NAME, MB_OK | MB_ICONERROR);
        return { FloatPoint(), false };
    }

    auto offsetXStr = posOffsetStr.substr(1, commaIdx - 1);
    auto [offsetX, convMessage] = helper::ConvertToFloat(offsetXStr);
    if (convMessage != nil) {
        MessageBoxA(nil, format("Invalid offset X: {} at line {} in {}.",
            convMessage, lineCount, gameConfigPath).c_str(), APP_NAME, MB_OK | MB_ICONERROR);
        return { FloatPoint(), false };
    }

    auto offsetYStr = posOffsetStr.substr(commaIdx + 1, posOffsetStr.length() - commaIdx - 2);
    auto [offsetY, convMessage2] = helper::ConvertToFloat(offsetYStr);
    if (convMessage2 != nil) {
        MessageBoxA(nil, format("Invalid offset Y: {} at line {} in {}.",
            convMessage2, lineCount, gameConfigPath).c_str(), APP_NAME, MB_OK | MB_ICONERROR);
        return { FloatPoint(), false };
    }

    return { FloatPoint{offsetX, offsetY}, true };
}

tuple<DWORD, bool> ExtractBaseHeight(stringstream& stream, int lineCount, const char* gameConfigPath) {
    DWORD baseHeight;
    stream >> dec >> baseHeight;

    if (baseHeight == 0) {
        MessageBoxA(nil, format("Invalid baseHeight at line {} in {}.", lineCount, gameConfigPath).c_str(),
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
        MessageBoxA(nil, format("Invalid aspectRatio: expected separating ':' at line {} in {}.",
            lineCount, gameConfigPath).c_str(), APP_NAME, MB_OK | MB_ICONERROR);
        return { FloatPoint(), false };
    }

    auto ratioXStr = aspectRatioStr.substr(0, colonIdx);
    auto [ratioX, convMessage] = helper::ConvertToFloat(ratioXStr);
    if (convMessage != nil) {
        MessageBoxA(nil, format("Invalid aspectRatio X: {} at line {} in {}.",
            convMessage, lineCount, gameConfigPath).c_str(), APP_NAME, MB_OK | MB_ICONERROR);
        return { FloatPoint(), false };
    }

    auto ratioYStr = aspectRatioStr.substr(colonIdx + 1, aspectRatioStr.length() - colonIdx - 1);
    auto [ratioY, convMessage2] = helper::ConvertToFloat(ratioYStr);
    if (convMessage2 != nil) {
        MessageBoxA(nil, format("Invalid aspectRatio Y: {} at line {} in {}.",
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
        token = strtok_s(nil, "/", &nextToken);
    }

    if (inputMethods == None) {
        MessageBoxA(nil, format("Invalid inputMethod at line {} in {}.", lineCount, gameConfigPath).c_str(),
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
        MessageBoxA(nil, "Missing " VirtualKeyCodesFile " file.", APP_NAME, MB_OK | MB_ICONERROR);
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
        if (convMessage != nil) {
            MessageBoxA(nil, format("Invalid value: {} at line {} in " VirtualKeyCodesFile ".",
                convMessage, lineCount).c_str(), APP_NAME, MB_OK | MB_ICONERROR);
            return { move(vkCodes), false };
        }
        vkCodes[key] = (BYTE)value;
    }

    return { move(vkCodes), true };
}