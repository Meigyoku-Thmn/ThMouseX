#pragma once
#include <winerror.h>

#ifdef COMMON_EXPORTS
#define DLLEXPORT __declspec(dllexport)
#define DLLEXPORT_C extern "C" __declspec(dllexport)
#else
#define DLLEXPORT __declspec(dllimport)
#define DLLEXPORT_C extern "C" __declspec(dllimport)
#endif

#define QUOTE(x) #x

#define SINGLE_ARG(...) __VA_ARGS__

#define BEGIN_EXPORT_FLAG_ENUM(EnumName, EnumType) \
enum class EnumName: EnumType; \
inline EnumName operator &(EnumName a, const EnumName b) { \
    return (EnumName)((EnumType)a & (EnumType)b); \
} \
inline EnumName operator &=(EnumName& a, const EnumName b) { \
    (EnumType&)a &= (EnumType)b; \
    return b; \
} \
inline EnumName operator |(EnumName a, const EnumName b) { \
    return (EnumName)((EnumType)a | (EnumType)b); \
} \
inline EnumName operator |=(EnumName& a, const EnumName b) { \
    (EnumType&)a |= (EnumType)b; \
    return b; \
} \
enum class EnumName: EnumType {
#define END_FLAG_ENUM() };

// If facility of hresult is win32 then return the lower 16bit, else return unchanged
#define WIN32_FROM_HRESULT(hr)  \
    (SUCCEEDED(hr) ? ERROR_SUCCESS : \
        (HRESULT_FACILITY(hr) == FACILITY_WIN32 ? HRESULT_CODE(hr) : (hr)))