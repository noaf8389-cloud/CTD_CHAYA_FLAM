#pragma once

#include <string>

class Server {
public:
    // Stores the board layout path and port to use once run() is called.
    Server(const std::string& layoutPath, int port) : layoutPath_(layoutPath), port_(port) {}

    /// Starts the server and runs the main update loop.
    int run();

private:
    std::string layoutPath_;
    int port_;
};
