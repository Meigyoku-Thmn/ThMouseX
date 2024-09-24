#pragma once
#include <Windows.h>
#include <span>
#include "DataTypes.h"

namespace common::errormsg::cor {
    void EnsureCorrectness();
    extern const std::span<const ErrorMessage> messages;
}