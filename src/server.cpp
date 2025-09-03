#include <netinet/in.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

#include <jedis/tcp.h>

int main() {
    Jedis::TcpSocket tcp {};

    tcp.SetupPort(8123);
    tcp.SetupAddress("0.0.0.0");
    tcp.Accept(
        [](auto conn_fd_) {
            char r_buffer[64] = {};
            if (const ssize_t n = read(conn_fd_, r_buffer, sizeof(r_buffer) - 1); n < 0) {
                std::cout << "read() error" << std::endl;
                return;
            }

            std::cout << "client says : " << r_buffer << std::endl;

            constexpr char w_buff[] = "world";
            write(conn_fd_, w_buff, strlen(w_buff));
        }
    );
}
