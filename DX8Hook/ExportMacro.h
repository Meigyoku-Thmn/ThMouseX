#pragma once
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the THDXHOOK_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// THDXHOOK_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.

// #ifdef _USRDLL
#ifdef _USRDLL
#ifdef DX8HOOK_EXPORTS
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT __declspec(dllimport)
#endif
#else
#define DLLEXPORT
#endif