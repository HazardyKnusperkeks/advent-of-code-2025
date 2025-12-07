#include "challenge7.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
#include <flat_map>
#include <flat_set>

namespace {
using Coordinate = Coordinate<std::int64_t>;

std::int64_t countSplits(MapView map) {
    Coordinate::setMaxFromMap(map);
    const auto startColumn = map.Base.front().find('S');
    throwIfInvalid(startColumn != std::string_view::npos);

    std::flat_set<Coordinate> beams;
    std::flat_set<Coordinate> nextBeams;
    std::int64_t              numberOfSplits = 0;

    auto addIfValid                          = [&nextBeams](Coordinate beam) noexcept {
        if ( beam.isValid() ) {
            nextBeams.insert(beam);
        } //if ( beam.isValid() )
        return;
    };

    beams.emplace(0, static_cast<std::int64_t>(startColumn));

    while ( !beams.empty() ) {
        for ( auto beam : beams ) {
            beam.move(Direction::Down);
            if ( beam.isValid() ) {
                if ( map[beam] == '.' ) {
                    nextBeams.insert(beam);
                } //if ( map[beam] == '.' )
                else {
                    throwIfInvalid(map[beam] == '^');
                    ++numberOfSplits;
                    addIfValid(beam.moved(Direction::Left));
                    addIfValid(beam.moved(Direction::Right));
                } //else -> if ( map[beam] == '.' )
            } //if ( beam.isValid() )
        } //for ( auto beam : beams )
        std::swap(beams, nextBeams);
        nextBeams.clear();
    } //while ( !beams.empty() )

    return numberOfSplits;
}

std::int64_t countTimelines(MapView map, Coordinate beam) noexcept {
    beam.move(Direction::Down);
    if ( !beam.isValid() ) {
        return 1;
    } //if ( !beam.isValid() )

    static std::flat_map<Coordinate, std::int64_t> cache;
    if ( auto iter = cache.find(beam); iter != cache.end() ) {
        return iter->second;
    } //if ( auto iter = cache.find(beam); iter != cache.end() )

    std::int64_t ret = 0;
    if ( map[beam] == '.' ) {
        ret = countTimelines(map, beam);
    } //if ( map[beam] == '.' )
    else {
        ret = countTimelines(map, beam.moved(Direction::Left)) + countTimelines(map, beam.moved(Direction::Right));
    } //else -> if ( map[beam] == '.' )

    cache.emplace(beam, ret);
    return ret;
}

std::int64_t countTimelines(MapView map) noexcept {
    const auto startColumn = map.Base.front().find('S');
    return countTimelines(map, Coordinate{0, static_cast<std::int64_t>(startColumn)});
}
} //namespace

bool challenge7(const std::vector<std::string_view>& input) {
    const auto numberOfSplits = countSplits(input);
    myPrint(" == Result of Part 1: {:d} ==\n", numberOfSplits);

    const auto numberOfTimelines = countTimelines(input);
    myPrint(" == Result of Part 2: {:d} ==\n", numberOfTimelines);

    return numberOfSplits == 1570 && numberOfTimelines == 15'118'009'521'693;
}
