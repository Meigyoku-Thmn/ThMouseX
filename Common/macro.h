#pragma once
#include <winerror.h>

#define APP_NAME "ThMouseX"

#define L_(str) L_IMPL(str)
#define L_IMPL(str) L##str

#define nil nullptr

#define SYM_NAME(name) #name

#define SCROLL_UP_EVENT       0x97
#define SCROLL_DOWN_EVENT     0x98
#define SCROLL_LEFT_EVENT     0x99
#define SCROLL_RIGHT_EVENT    0x9A

#define UNUSED [[maybe_unused]]

#define DLLEXPORT_C extern "C" __declspec(dllexport)

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

#define defer(...) defer_impl(__COUNTER__, __VA_ARGS__)
#define defer_impl(counter, ...) std::shared_ptr<void> MAKE_UNIQUE_VAR(counter)(nullptr, [&](...) __VA_ARGS__);
