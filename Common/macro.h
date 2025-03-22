#pragma once
#include <winerror.h>
#include <bit>

#define APP_NAME "ThMouseX"
#define HOOK_ENGINE_STATE_NAME "ThMouseX_MinHook_State"

#define L_(str) L_IMPL(str)
#define L_IMPL(str) L##str

#define LS_(sym) LS_IMPL(sym)
#define LS_IMPL(str) LS_IMPL2(#str)
#define LS_IMPL2(str) L##str

#define nil nullptr

#define rcast reinterpret_cast
#define scast static_cast
#define dcast dynamic_cast
#define bcast std::bit_cast

#define SYM_NAME(name) #name

#define SCROLL_UP_EVENT       0x97
#define SCROLL_DOWN_EVENT     0x98
#define SCROLL_LEFT_EVENT     0x99
#define SCROLL_RIGHT_EVENT    0x9A

#define UNUSED [[maybe_unused]]

#define DLLEXPORT_C extern "C" __declspec(dllexport)

#define MAKE_UNIQUE_VAR(counter) var_discard_##counter

#define defer(...) defer_impl(__COUNTER__, __VA_ARGS__)
#define defer_impl(counter, ...) std::shared_ptr<void> MAKE_UNIQUE_VAR(counter)(nil, [&](...) __VA_ARGS__)

#define FixedStringMember(type, name, value) type name[ARRAYSIZE(value)] = value
#define ImportWinAPI(hModule, API) decltype(&API) API = hModule ? bcast<decltype(API)>(GetProcAddress(hModule, SYM_NAME(API))) : nil

#define SHELLCODE_SECTION_NAME ".shlcode"
#define SHELLCODE  __declspec(safebuffers) __declspec(code_seg(SHELLCODE_SECTION_NAME))
