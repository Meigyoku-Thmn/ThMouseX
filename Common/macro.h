#pragma once
#include <winerror.h>

#define EXTERN_C extern "C"
#define DLLEXPORT __declspec(dllexport)
#define DLLEXPORT_C EXTERN_C __declspec(dllexport)
#define UNBOUND template <typename = void>

#define BEGIN_EXPORT_FLAG_ENUM(EnumName, EnumType) \
export enum class EnumName: EnumType; \
export UNBOUND EnumName operator &(EnumName a, const EnumName b) { \
    return (EnumName)((EnumType)a & (EnumType)b); \
} \
export UNBOUND EnumName operator &=(EnumName& a, const EnumName b) { \
    (EnumType&)a &= (EnumType)b; \
    return b; \
} \
export UNBOUND EnumName operator |(EnumName a, const EnumName b) { \
    return (EnumName)((EnumType)a | (EnumType)b); \
} \
export UNBOUND EnumName operator |=(EnumName& a, const EnumName b) { \
    (EnumType&)a |= (EnumType)b; \
    return b; \
} \
enum class EnumName: EnumType {
#define END_FLAG_ENUM() };

// If facility of hresult is win32 then return the lower 16bit, else return unchanged
#define WIN32_FROM_HRESULT(hr)  \
    (SUCCEEDED(hr) ? ERROR_SUCCESS : \
        (HRESULT_FACILITY(hr) == FACILITY_WIN32 ? HRESULT_CODE(hr) : (hr)))