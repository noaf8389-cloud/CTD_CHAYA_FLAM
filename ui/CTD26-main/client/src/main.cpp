// https://github.com/noaf8389-cloud/CTD_CHAYA_FLAM.git

#include "game/controler.hpp"
#include <windows.h>
#include <iostream>
#include "logging/logger.hpp"

int main() {
    SetProcessDPIAware();
    try {
        std::string username;
        std::cout << "Username: ";
        std::cin >> username;

        std::string password;
        std::cout << "Password: ";
        std::cin >> password;

        Logger::init("client.log");
        Controler("../..", "pieces3", username, password).run();
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
