#pragma once

#define _ref
#ifdef DX8HOOK_EXPORTS
#define DLLEXPORT __declspec(dllexport)
#define DLLEXPORT_C extern "C" __declspec(dllexport)
#else
#define DLLEXPORT __declspec(dllimport)
#define DLLEXPORT_C extern "C" __declspec(dllimport)
#endif

#define QUOTE(x) #x