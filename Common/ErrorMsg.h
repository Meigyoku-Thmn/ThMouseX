#pragma once
#include "framework.h"
#include <string>
#include <vector>
#include "DataTypes.h"

namespace common::errormsg {
	std::string GuessErrorsFromHResult(HRESULT hr);
}