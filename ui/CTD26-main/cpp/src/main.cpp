#include "game/controler.hpp"
#include <windows.h>
#include <iostream>

int main() {
    SetProcessDPIAware();
    try {
        Controler("../..", "pieces3").run();
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
