#pragma once

class ServerConnection;

class CommandSender {
public:
    // Wraps an existing server connection for sending click/jump commands.
    explicit CommandSender(ServerConnection& connection);

    void sendClick(int row, int col){
        send("click", row, col);
    }

    void sendJump(int row, int col){
        send("jump", row, col);
    }

private:
      void send(const char* command, int row, int col);

    ServerConnection& connection_;
};
