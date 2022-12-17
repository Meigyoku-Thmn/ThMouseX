module;

#include "framework.h"
#include <string>
#include <algorithm>
#include <vector>
#include <functional>

export module common.errormsg;

using namespace std;

import common.datatype;
import common.errormsg.cor;
import common.errormsg.d3d;
import common.errormsg.ddraw;

namespace common::errormsg {
	reference_wrapper<vector<ErrorMessage>> messageGroups[] = { cor::messages, d3d::messages, ddraw::messages };
	export string GuessErrorsFromHResult(HRESULT hr) {
		string errorMessage = "";
		for (auto messages : messageGroups) {
			auto messageItr = lower_bound(messages.get().begin(), messages.get().end(), hr, [](const ErrorMessage& left, HRESULT value) {
				return left.code < DWORD(value);
			});
			if (messageItr == messages.get().end() || messageItr->code != hr)
				continue;
			while (messageItr->code == hr) {
				errorMessage += string(" (") + messageItr->sourceHeader + ") " + messageItr->symbolicName + ": " + messageItr->description + "\n";
				messageItr++;
			}
			errorMessage.pop_back();
		}
		if (errorMessage == "")
			return errorMessage;
		return "Possible error(s):\n" + errorMessage;
	}
}