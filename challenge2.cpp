#include "challenge2.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
#include <unordered_set>

namespace {
struct IdRange {
    std::string_view Lower;
    std::string_view Upper;
};

auto toRanges(std::span<const std::string_view> input) {
    throwIfInvalid(input.size() == 1);
    return splitString(input.front(), ',') | std::views::transform([](std::string_view range) {
               auto dash = range.find('-');
               throwIfInvalid(dash != std::string_view::npos);
               return IdRange{range.substr(0, dash), range.substr(dash + 1)};
           });
}

std::int64_t sumInvalidIds(const IdRange& range) {
    std::int64_t from;
    std::int64_t to;

    if ( auto size = range.Lower.size(); size % 2 == 0 ) {
        from = std::min(convert(range.Lower.substr(0, size / 2)), convert(range.Lower.substr(size / 2)));
    } //if ( range.Lower.size() % 2 == 0 )
    else {
        from = pow(10, size / 2);
    } //else -> if ( range.Lower.size() % 2 == 0 )

    if ( auto size = range.Upper.size(); size % 2 == 0 ) {
        to = std::max(convert(range.Upper.substr(0, size / 2)), convert(range.Upper.substr(size / 2)));
    } //if ( auto size = range.Upper.size(); size % 2 == 0 )
    else {
        to = pow(10, size / 2) - 1;
    } //else -> if ( auto size = range.Upper.size(); size % 2 == 0 )

    const auto lowest  = convert(range.Lower);
    const auto highest = convert(range.Upper);

    auto makeDouble    = [](std::int64_t value) noexcept {
        const auto length = log10(value);
        return value + value * pow(10, length + 1);
    };

    auto withinLimit = [&highest, &lowest](const std::int64_t doubleDigits) noexcept {
        return doubleDigits >= lowest && doubleDigits <= highest;
    };

    return std::ranges::fold_left(std::views::iota(from, to + 1) | std::views::transform(makeDouble) |
                                      std::views::filter(withinLimit),
                                  0, std::plus<>{});
}

std::int64_t sumInvalidIdsPart2(const IdRange& range) {
    std::int64_t from = 1;
    std::int64_t to;

    if ( auto size = range.Upper.size(); size % 2 == 0 ) {
        to = std::max(convert(range.Upper.substr(0, size / 2)), convert(range.Upper.substr(size / 2)));
    } //if ( auto size = range.Upper.size(); size % 2 == 0 )
    else {
        to = pow(10, size / 2) - 1;
    } //else -> if ( auto size = range.Upper.size(); size % 2 == 0 )

    const auto lowest  = convert(range.Lower);
    const auto highest = convert(range.Upper);

    std::unordered_set<std::int64_t> invalidIds;
    for ( auto value = from; value <= to; ++value ) {
        const auto length = log10(value);
        const auto shift  = pow(10, length + 1);

        for ( auto multiple = value * shift + value; multiple <= highest; multiple = multiple * shift + value ) {
            if ( multiple >= lowest && multiple <= highest ) {
                invalidIds.insert(multiple);
            } //if ( multiple >= lowest && multiple <= highest )
        } //for ( auto multiple = value * shift + value; multiple <= highest; multiple = multiple * shift + value )
    } //for ( auto value = from; value <= to; ++value )

    return std::ranges::fold_left(invalidIds, 0, std::plus<>{});
}
} //namespace

bool challenge2(const std::vector<std::string_view>& input) {
    const auto sum1 =
        std::ranges::fold_left(toRanges(input) | std::views::transform(sumInvalidIds), 0, std::plus<>{});
    myPrint(" == Result of Part 1: {:d} ==\n", sum1);

    const auto sum2 =
        std::ranges::fold_left(toRanges(input) | std::views::transform(sumInvalidIdsPart2), 0, std::plus<>{});
    myPrint(" == Result of Part 2: {:d} ==\n", sum2);

    return sum1 == 15'873'079'081 && sum2 == 22'617'871'034;
}
