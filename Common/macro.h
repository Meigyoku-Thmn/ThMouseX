#pragma once
#include <winerror.h>

#ifdef COMMON_EXPORTS
#define DLLEXPORT __declspec(dllexport)
#define DLLEXPORT_C extern "C" __declspec(dllexport)
#else
#define DLLEXPORT __declspec(dllimport)
#define DLLEXPORT_C extern "C" __declspec(dllimport)
#endif

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

#ifdef COMMON_EXPORTS

#define EXPORT_VARIABLE____(type, name) \
extern type name; \
DLLEXPORT inline type& get_ ## name() { return ::name; }

#define EXPORT_VARIABLE_ARR(type, name, arrSize) \
extern type name ## arrSize; \
DLLEXPORT inline type (&get_ ## name())arrSize { return ::name; }

#else

#define EXPORT_VARIABLE____(type, name) \
DLLEXPORT type& get_ ## name(); \
inline type& name = get_ ## name();

#define EXPORT_VARIABLE_ARR(type, name, arrSize) \
DLLEXPORT type (&get_ ## name())arrSize; \
inline type (&name) arrSize = get_ ## name();

#endif