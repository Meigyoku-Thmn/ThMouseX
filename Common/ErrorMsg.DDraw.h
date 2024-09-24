#pragma once
#include <Windows.h>
#include <span>
#include "DataTypes.h"

namespace common::errormsg::ddraw {
    void EnsureCorrectness();
    extern const std::span<const ErrorMessage> messages;
}