#pragma once

#include <stdio.h>

// damn microsoft >_<
int (WINAPIV *__vsnprintf)(char*, size_t, const char*, va_list) = _vsnprintf;
int (WINAPIV *__snprintf)(char*, size_t, const char*, ...) = _snprintf;
int (WINAPIV *_sscanf)(const char*, const char*, ...) = sscanf;
int (WINAPIV *_sprintf)(char*, const char*, ...) = sprintf;

#include "d3d8.h"
#pragma comment(lib, "d3d8.lib")
#include "d3dx8core.h"
#pragma comment(lib, "D3dx8.lib")
