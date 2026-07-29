// https://github.com/noaf8389-cloud/CTD_CHAYA_FLAM.git

#include "game/controler.hpp"
#include "game/room_dialog.hpp"
#include <windows.h>
#include "logging/logger.hpp"

int main() {
    SetProcessDPIAware();
    try {
        LoginPrompt login = promptLogin();

        Logger::init("client.log");
        Controler("../..", "pieces3", login.username, login.password, login.matchChoice).run();
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
