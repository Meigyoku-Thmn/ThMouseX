#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include "DataTypes.h"

namespace common::errormsg {
	void Initialize();
	std::string GuessErrorsFromHResult(HRESULT hr);
}