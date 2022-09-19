module;

#include "framework.h"
#include <fstream>
#include <sstream>

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
    ifstream gamesFile("games.txt");
    auto& pConfig = gs_gameConfigArray;
    pConfig = {};
    if (!gamesFile) {
        MessageBox(NULL, "Can not find games file.", "Launcher", MB_OK | MB_ICONERROR);
        return false;
    }
    int configIdx;
    // read each line of config file
    for (configIdx = 0; configIdx < GAME_CONFIG_MAX_LEN && !gamesFile.eof(); configIdx++) {
        string _line;
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
        stringstream lineStream{string(lineView)};
        stringstream converter;
        auto& currentConfig = pConfig.Configs[configIdx];

        // read process name
        string processName;
        lineStream >> currentConfig.ProcessName;

        // read pointer chain
        string pointerChainStr;
        lineStream >> pointerChainStr;
        string::size_type leftBoundIdx = 0, rightBoundIdx = -1;
        for (size_t addressLevelIdx = 0; addressLevelIdx < ADDRESS_CHAIN_MAX_LEN; addressLevelIdx++) {
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
            currentConfig.Address.Level[addressLevelIdx] = address;
            currentConfig.Address.Length++;
        }
        if (currentConfig.Address.Level[0] == 0) {
            configIdx--;
            continue;
        }

        // read data type
        string dataType;
        lineStream >> dataType;
        if (dataType.compare("int") == 0)
            currentConfig.PosDataType = Int_DataType;
        else if (dataType.compare("float") == 0)
            currentConfig.PosDataType = Float_DataType;
        else if (dataType.compare("short") == 0)
            currentConfig.PosDataType = Short_DataType;
        else {
            configIdx--;
            continue;
        }

        // read offset (X,Y)
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

        // read game-internal base resolution
        lineStream >> dec >> currentConfig.BaseResolutionX;
    }

    if (configIdx == 0) {
        MessageBox(NULL, "No valid data in config file.", "Launcher", MB_OK | MB_ICONERROR);
        return false;
    }
    pConfig.Length = configIdx;

    return true;
}

export bool ReadIniFile() {
    ifstream iniFile("ThMouse.ini");
    if (!iniFile) {
        MessageBox(NULL, "Can not find ThMouse.ini file.", "Launcher", MB_OK | MB_ICONERROR);
        return false;
    }
    string line;
    getline(iniFile, line);
    if (line.compare("[ThMouse]") != 0) {
        MessageBox(NULL, "ThMouse.ini file error.", "Launcher", MB_OK | MB_ICONERROR);
        return false;
    }
    while (!iniFile.eof()) {
        getline(iniFile, line);
        if (line.find("LeftButton") != string::npos) {
            auto eqIndex = line.find('=');
            auto numStr = line.substr(eqIndex + 1);
            stringstream ss;
            ss << numStr;
            ss >> gs_boomButton;
        } else if (line.find("CursorTexture") != string::npos) {
            auto eqIndex = line.find('=');
            auto pathStr = line.substr(eqIndex + 1);
            stringstream ss;
            ss << pathStr;
            string texturePath;
            ss >> texturePath;
            GetFullPathName(texturePath.c_str(), MAX_PATH, gs_textureFilePath, NULL);
        } else if (line.find("MiddleButton") != string::npos) {
            auto eqIndex = line.find('=');
            auto numStr = line.substr(eqIndex + 1);
            stringstream ss;
            ss << numStr;
            ss >> gs_extraButton;
        } else if (line.find("CursorOnResolutionX") != string::npos) {
            auto eqIndex = line.find('=');
            auto numStr = line.substr(eqIndex + 1);
            stringstream ss;
            ss << numStr;
            ss >> gs_textureBaseResolutionX;
        }
    }
    return true;
}
