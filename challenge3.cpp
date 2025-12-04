#include "challenge3.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
#include <iterator>
#include <ranges>

namespace {
std::int64_t findLargestBatterySum2(std::string_view bank) {
    const auto first = std::ranges::max_element(bank.substr(0, bank.size() - 1));
    const auto last  = std::ranges::max_element(std::next(first), bank.end());
    return toDigit(*first) * 10 + toDigit(*last);
}

std::int64_t findLargestBatterySum12(std::string_view bank) {
    auto         begin = bank.begin();
    auto         end   = std::prev(bank.end(), 11);
    std::int64_t ret   = 0;

    for ( auto i = 0; i < 12; ++i, ++end, ++begin ) {
        begin  = std::ranges::max_element(begin, end);
        ret   *= 10;
        ret   += toDigit(*begin);
    } //for ( auto i = 0; i < 12; ++i, ++end, ++begin )
    return ret;
}
} //namespace

bool challenge3(const std::vector<std::string_view>& input) {
    const auto sum1 = std::ranges::fold_left(input | std::views::transform(findLargestBatterySum2), 0, std::plus<>{});
    myPrint(" == Result of Part 1: {:d} ==\n", sum1);

    const auto sum2 = std::ranges::fold_left(input | std::views::transform(findLargestBatterySum12), 0, std::plus<>{});
    myPrint(" == Result of Part 2: {:d} ==\n", sum2);

    return sum1 == 16842 && sum2 == 20520794;
}
