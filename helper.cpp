#include "helper.hpp"

#include "print.hpp"

#include <stdexcept>

void throwIfInvalid(bool valid, const char* msg) {
    if ( !valid ) {
        myFlush();
        throw std::runtime_error{msg};
    } //if ( !valid )
    return;
}

void fail(void) {
    throwIfInvalid(false, "Fail");
    std::unreachable();
}
