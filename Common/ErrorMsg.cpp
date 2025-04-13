#include <Windows.h>
#include <string>
#include <algorithm>
#include <span>
#include <vector>

#include "ErrorMsg.h"
#include "ErrorMsg.Cor.h"
#include "ErrorMsg.D3D.h"
#include "ErrorMsg.DDraw.h"

using namespace std;

namespace common::errormsg {
    vector<span<const ErrorMessage>> messageGroups;
    void Initialize() {
        cor::EnsureCorrectness();
        d3d::EnsureCorrectness();
        ddraw::EnsureCorrectness();
        messageGroups = { cor::messages, d3d::messages, ddraw::messages };
    }
    string GuessErrorsFromHResult(HRESULT hr) {
        string errorMessage = "";
        for (auto const& messages : messageGroups) {
            auto messageItr = ranges::lower_bound(messages, hr, less<>(), &ErrorMessage::code);
            if (messageItr == messages.end() || messageItr->code != hr)
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