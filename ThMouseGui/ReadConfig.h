#pragma once

#include "..\ThDxHook\ThDxHook.h"

BOOL readGamesFile(GameConfigArray * pConfig);
BOOL readIniFile(int* pLeftButton, int* pMidButton, char* pTextureFilePath);
