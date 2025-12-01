#include "challenge1.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <algorithm>

namespace {
std::int64_t toRotation(std::string_view line) {
    const auto sign = line.front() == 'L' ? -1 : 1;
    return convert(line.substr(1)) * sign;
}

auto rotations(std::span<const std::string_view> input) {
    auto rotate = [at = 50LL](std::int64_t step) mutable noexcept {
        at += step;
        at %= 100;
        at += 100;
        at %= 100;
        return at;
    };
    return input | std::views::transform(toRotation) | std::views::transform(rotate);
}

std::int64_t countZeroes(std::span<const std::string_view> input) {
    return std::ranges::count(rotations(input), 0);
}

std::int64_t countAllZeroes(std::span<const std::string_view> input) {
    auto rotatations = input | std::views::transform(toRotation);
    auto rotate      = [at = 50LL](std::int64_t step) mutable {
        throwIfInvalid(step != 0);
        std::int64_t ret    = 0;
        bool         invert = step > 0;
        if ( invert ) {
            //Normalize to left turns.
            at   = -at;
            step = -step;
        } //if ( invert )

        throwIfInvalid(at != -100);
        if ( at == 0 ) {
            --at;
            ++step;
        } //if ( at == 0 )

        const auto toZero = (-100 - at) % -100;
        if ( toZero < step ) {
            at += step;
        } //if ( toZero < step )
        else {
            at    = 0;
            step -= toZero;
            ret   = 1;

            ret  += step / -100;
            step %= 100;
            at   += step;
        } //else -> if ( toZero < step )

        if ( invert ) {
            at = -at;
        } //if ( invert )
        return ret;
    };
    return std::ranges::fold_left(rotatations | std::views::transform(rotate), 0, std::plus<>{});
}
} //namespace

bool challenge1(const std::vector<std::string_view>& input) {
    const auto zeroes = countZeroes(input);
    myPrint(" == Result of Part 1: {:d} ==\n", zeroes);

    const auto sum2 = countAllZeroes(input);
    myPrint(" == Result of Part 2: {:d} ==\n", sum2);

    return zeroes == 1152 && sum2 == 6671;
}
