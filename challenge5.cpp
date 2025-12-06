#include "challenge5.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
#include <ranges>
#include <vector>

namespace {
struct Range {
    std::int64_t From;
    std::int64_t To;

    std::int64_t size(void) const noexcept {
        return To - From + 1;
    }
};

Range toRange(std::string_view line) {
    auto dash = line.find('-');
    throwIfInvalid(dash != std::string_view::npos);
    return {convert(line.substr(0, dash)), convert(line.substr(dash + 1))};
}

struct Database {
    std::vector<Range>        FreshIngredients;
    std::vector<std::int64_t> IngredientsToCheck;
};

Database parse(std::span<const std::string_view> input) {
    Database ret;

    auto filtered  = input | std::views::take_while([](std::string_view line) noexcept { return !line.empty(); });
    auto readUntil = std::ranges::transform(filtered, std::back_inserter(ret.FreshIngredients), toRange).in;
    std::ranges::transform(std::next(readUntil), input.end(), std::back_inserter(ret.IngredientsToCheck), convert<10>);

    std::ranges::sort(ret.FreshIngredients, std::ranges::less{}, &Range::From);

    //Merge
    auto iter = ret.FreshIngredients.begin();
    auto next = std::next(iter);
    while ( next != ret.FreshIngredients.end() ) {
        if ( iter->To >= next->From ) {
            if ( next->To > iter->To ) {
                iter->To = next->To;
            } //if ( next->To > iter->To )
            next = ret.FreshIngredients.erase(next);
        } //if ( iter->To >= next->From )
        else {
            ++iter;
            ++next;
        } //else -> if ( iter->To >= next->From )
    }

    return ret;
}

std::int64_t countFresh(const Database& database) noexcept {
    auto isFresh = [&database](std::int64_t ingredient) noexcept {
        auto contains = [ingredient](const Range& range) noexcept {
            return ingredient >= range.From && ingredient <= range.To;
        };
        return std::ranges::any_of(database.FreshIngredients, contains);
    };
    return std::ranges::count_if(database.IngredientsToCheck, isFresh);
}
} //namespace

bool challenge5(const std::vector<std::string_view>& input) {
    const auto database = parse(input);
    const auto sum1     = countFresh(database);
    myPrint(" == Result of Part 1: {:d} ==\n", sum1);

    const auto sum2 =
        std::ranges::fold_left(database.FreshIngredients | std::views::transform(&Range::size), 0, std::plus<>{});
    myPrint(" == Result of Part 2: {:d} ==\n", sum2);

    return sum1 == 775 && sum2 == 350'684'792'662'845;
}
