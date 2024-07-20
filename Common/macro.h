#pragma once
#include <winerror.h>

#define APP_NAME "ThMouseX"

#define L_(str) L_IMPL(str)
#define L_IMPL(str) L##str

#define _ref

#define null nullptr

#define DLLEXPORT __declspec(dllexport)
#define DLLEXPORT_C extern "C" __declspec(dllexport)

#define QUOTE(x) #x

#define SINGLE_ARG(...) __VA_ARGS__

#define EVAL_DISCARD(expr) EVAL_DISCARD_IMPL(__COUNTER__, expr)
#define EVAL_DISCARD_IMPL(counter, expr) EVAL_DISCARD_IMPL_EXPAND(counter, expr)
#define EVAL_DISCARD_IMPL_EXPAND(counter, expr) auto var_discard_##counter = expr

#define MAKE_UNIQUE_VAR(counter) var_discard_##counter

#define ON_INIT ON_INIT_IMPL(__COUNTER__, __COUNTER__)
#define ON_INIT_IMPL(counter1, counter2) \
void MAKE_UNIQUE_VAR(counter1)(); \
bool MAKE_UNIQUE_VAR(counter2)() { \
    MAKE_UNIQUE_VAR(counter1)(); \
    return true; \
} \
EVAL_DISCARD(MAKE_UNIQUE_VAR(counter2)()); \
void MAKE_UNIQUE_VAR(counter1)()

#define BEGIN_FLAG_ENUM(EnumName, EnumType) \
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

#define SAFE_RELEASE(p)    if ((p)) { (p)->Release(); (p) = 0; }0
#define SAFE_DELETE(a)     if ((a)) { delete (a); (a) = null; }0
#define SAFE_DELETE_ARR(a) if ((a)) { delete[] (a); (a) = null; }0
#define SAFE_FREE_LIB(h)   if ((h)) { FreeLibrary(h); h = null; }0