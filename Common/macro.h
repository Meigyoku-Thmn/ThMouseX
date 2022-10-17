#pragma once
#define EXTERN_C extern "C"
#define DLLEXPORT __declspec(dllexport)
#define DLLEXPORT_C EXTERN_C __declspec(dllexport)
#define UNBOUND template <typename = void>