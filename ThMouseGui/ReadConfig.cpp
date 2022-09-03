#include "stdafx.h"
#include "ReadConfig.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

using namespace std;

/// <summary>
/// Đọc Config
/// </summary>
/// <returns> true hoặc false </returns>
/// <param name="pConfig"> con trỏ</param>
BOOL readGamesFile(GameConfigArray *pConfig) {
    ifstream gamesFile("games.txt");
    if (!gamesFile) {
        MessageBox(NULL, "Can not find games file.", "Launcher", MB_OK);
        return FALSE;
    }
    int configIdx;
    // read each line of config file
    for (configIdx = 0; configIdx < GAME_CONFIG_MAX_LEN && !gamesFile.eof(); configIdx++) {
        string line;
        getline(gamesFile, line);

        // ignore empty line
        if (line.empty()) {
            configIdx--;
            continue;
        }

        // ignore comment line
        if (line[0] == ';') {
            configIdx--;
            continue;
        }

        // use a "tokenizer"
        stringstream lineStream(line);
        stringstream converter;
        auto &currentConfig = pConfig->Configs[configIdx];

        // read process name
        string processName;
        lineStream >> currentConfig.ProcessName;

        // read pointer chain
        string pointerChainStr;
        lineStream >> pointerChainStr;
        string::size_type leftBoundIdx = 0, rightBoundIdx = -1;
        for (size_t addressIdx = 0; addressIdx < ADDRESS_CHAIN_MAX_LEN; addressIdx++) {
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
            currentConfig.Posistion.Chain[addressIdx] = address;
            currentConfig.Posistion.Length++;
        }
        if (currentConfig.Posistion.Chain[0] == 0) {
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

        // read pixel rate
        lineStream >> currentConfig.PixelRate;

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
        currentConfig.PixelOffset.Y = 1;
        currentConfig.PixelOffset.X = 1;
        if (lineStream.eof() == true) {
            configIdx--;
            continue;
        }

        // read game-internal base resolution
        lineStream >> dec >> currentConfig.BaseResolutionX;

        // the rest is for pointer chain starts on thread stack, useless for now
        UINT offsetIsRelative;
        lineStream >> offsetIsRelative;
        currentConfig.OffsetIsRelative = (bool)offsetIsRelative;
        if (offsetIsRelative == 0)
            continue;
        lineStream >> currentConfig.BaseName;
    }

    if (configIdx == 0) {
        MessageBox(NULL, "No valid data in config file.", "Launcher", MB_OK);
        return FALSE;
    }
    pConfig->Length = configIdx;

    return TRUE;
}

BOOL readIniFile(int *pLeftButton, int *pMidButton, char *pTextureFilePath) {
    ifstream iniFile("ThMouse.ini");
    if (!iniFile) {
        MessageBox(NULL, "Can not find ThMouse.ini file.", "Launcher", MB_OK);
        return 0;
    }
    string line;
    getline(iniFile, line);
    if (line.compare("[ThMouse]") != 0) {
        MessageBox(NULL, "ThMouse.ini file error.", "Launcher", MB_OK);
        return 0;
    }
    while (!iniFile.eof()) {
        getline(iniFile, line);
        if (line.find("LeftButton") != string::npos) {
            string::size_type eqIndex = line.find('=');
            string numStr = line.substr(eqIndex + 1);
            stringstream ss;
            ss << numStr;
            ss >> *pLeftButton;
        } else if (line.find("CursorTexture") != string::npos) {
            string::size_type eqIndex = line.find('=');
            string pathStr = line.substr(eqIndex + 1);
            stringstream ss;
            ss << pathStr;
            ss >> pTextureFilePath;
        } else if (line.find("MiddleButton") != string::npos) {
            string::size_type eqIndex = line.find('=');
            string numStr = line.substr(eqIndex + 1);
            stringstream ss;
            ss << numStr;
            ss >> *pMidButton;
        }
    }
    return 1;
}
