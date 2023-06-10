#pragma once

#define _ref
#ifdef DX8HOOK_EXPORTS
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT __declspec(dllimport)
#endif