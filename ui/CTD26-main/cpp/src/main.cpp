#include "game/controler.hpp"
#include <windows.h>
#include <iostream>

int main() {
    SetProcessDPIAware();
    try {
        std::string username;
        std::cout << "Username: ";
        std::cin >> username;

        Controler("../..", "pieces3", username).run();
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
