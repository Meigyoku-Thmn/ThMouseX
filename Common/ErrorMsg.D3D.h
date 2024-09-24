#pragma once
#include <Windows.h>
#include <span>
#include "DataTypes.h"

namespace common::errormsg::d3d {
    void EnsureCorrectness();
    extern const std::span<const ErrorMessage> messages;
}