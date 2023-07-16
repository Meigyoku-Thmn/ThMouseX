#include "framework.h"
#include <string>
#include <algorithm>
#include <vector>
#include <functional>

#include "ErrorMsg.h"
#include "ErrorMsg.Cor.h"
#include "ErrorMsg.D3D.h"
#include "ErrorMsg.DDraw.h"



using namespace std;

namespace common::errormsg {
    reference_wrapper<vector<ErrorMessage>> messageGroups[] = { cor::messages, d3d::messages, ddraw::messages };
    string GuessErrorsFromHResult(HRESULT hr) {
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