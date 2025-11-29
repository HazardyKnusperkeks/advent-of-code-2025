#include "print.hpp"

#include <iostream>

std::ostream_iterator<char> outIterator{std::cout};
std::ostream_iterator<char> errIterator{std::cerr};

void myFlush() {
    std::cout.flush();
    return;
}
