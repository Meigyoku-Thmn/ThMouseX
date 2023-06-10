#pragma once

namespace main {
    class config {
    public:
        static bool PopulateMethodRVAs();
        static bool ReadGamesFile();
        static bool ReadGeneralConfigFile();
    };
}