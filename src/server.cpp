#include <netinet/in.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

#include <jedis/message.h>
#include <jedis/tcp.h>

int main() {
    Jedis::TcpSocket tcp {};

    tcp.SetupPort(8123);
    tcp.SetupAddress("0.0.0.0");
    tcp.Accept(
        [](auto conn_fd) {
            while (true) {
                const int32_t err = Jedis::one_request(conn_fd);
                if (err) {
                    break;
                }
            }
        }
    );
}
