// https://github.com/noaf8389-cloud/CTD_CHAYA_FLAM.git

#include "server.hpp"

int main(int argc, char** argv) {
    std::string layoutPath = argc > 1 ? argv[1] : "../../ui/CTD26-main/layout_standard.txt";
    Server server(layoutPath, 8080);
    return server.run();
}