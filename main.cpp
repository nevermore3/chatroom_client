#include "client.h"

int main() {
    const char* serverIp = "192.168.0.139";
    int port = 6666;
    Client client(serverIp, port);
    client.Start();
    return 0;
}