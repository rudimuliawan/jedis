//
// Created by Rudi Muliawan on 27/08/25.
//

#pragma once

#include <iostream>
#include <string>

inline void DIE(const std::string &message) {
    std::cout << "DIE : " << message << std::endl;
    exit(-1);
}
