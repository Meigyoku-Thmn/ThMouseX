#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include "DataTypes.h"

namespace common::errormsg {
	std::string GuessErrorsFromHResult(HRESULT hr);
}