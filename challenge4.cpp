#include "challenge4.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
#include <iterator>
#include <unordered_set>

namespace {
auto countFreeRolls(MapView map) {
    auto isRoll     = [map](Coordinate<std::int64_t> c) noexcept { return map[c] == '@'; };
    auto isFreeRoll = [&isRoll](Coordinate<std::int64_t> c) noexcept {
        return isRoll(c) && std::ranges::count_if(c.validNeighborsWithDiagnonal(), isRoll) < 4;
    };
    return std::ranges::count_if(Coordinate<std::int64_t>::allPositions(), isFreeRoll);
}

auto countRepeadetelyFreeRolls(MapView map) {
    std::unordered_set<Coordinate<std::int64_t>> freedPositions;
    auto                                         oldSize = freedPositions.size();
    auto                                         isRoll  = [map, &freedPositions](Coordinate<std::int64_t> c) noexcept {
        return map[c] == '@' && !freedPositions.contains(c);
    };
    auto isFreeRoll = [&isRoll](Coordinate<std::int64_t> c) noexcept {
        return isRoll(c) && std::ranges::count_if(c.validNeighborsWithDiagnonal(), isRoll) < 4;
    };
    do {
        std::ranges::copy(Coordinate<std::int64_t>::allPositions() | std::views::filter(isFreeRoll),
                          std::inserter(freedPositions, freedPositions.end()));
    } while ( std::exchange(oldSize, freedPositions.size()) != freedPositions.size() );
    return oldSize;
}
} //namespace

bool challenge4(const std::vector<std::string_view>& input) {
    MapView map{input};
    Coordinate<std::int64_t>::setMaxFromMap(map);
    const auto sum1 = countFreeRolls(map);
    myPrint(" == Result of Part 1: {:d} ==\n", sum1);

    const auto sum2 = countRepeadetelyFreeRolls(map);
    myPrint(" == Result of Part 2: {:d} ==\n", sum2);

    return sum1 == 1411 && sum2 == 20520794;
}
