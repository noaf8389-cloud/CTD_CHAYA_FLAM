#pragma once
#include "../model/Position.hpp"

struct Motion {
    Position from;
    Position to;
    long long completionTime;
};

struct Jump {
    Position position;
    long long endTime;
};

struct Rest {
    Position position;
    long long endTime;
};
