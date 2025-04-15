//
// Created by rudi on 4/15/25.
//

#pragma once

#include <cstdint>
#include <vector>

struct Conn {
    int fd = -1;
    // application's intention, for the event loop
    bool want_read = false;
    bool want_write = false;
    bool want_error = false;

    // buffered input and output
    std::vector<uint8_t> incoming;
    std::vector<uint8_t> outgoing;
};
