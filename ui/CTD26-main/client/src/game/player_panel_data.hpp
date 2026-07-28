#pragma once

#include <string>
#include <vector>

struct PlayerPanelData {
    std::string username = "?";
    int rating = 1200;
    int score = 0;
    std::vector<std::string> moves;
};
