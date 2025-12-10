#include "challenge9.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
#include <flat_map>
#include <ranges>
#include <vector>

namespace {
using Coordinate = Coordinate<std::int64_t>;
using List       = std::vector<Coordinate>;

struct Rectangle {
    Coordinate   C1;
    Coordinate   C2;
    std::int64_t Area;
};

List parse(std::span<const std::string_view> input) {
    return input | std::views::transform([](std::string_view line) {
               const auto comma = line.find(',');
               throwIfInvalid(comma != std::string_view::npos);
               return Coordinate{convert(line.substr(comma + 1)), convert(line.substr(0, comma))};
           }) |
           std::ranges::to<std::vector>();
}

std::vector<Rectangle> calculateRectangles(const List& list) noexcept {
    auto calcArea = [](const std::tuple<Coordinate, Coordinate>& tuple) noexcept {
        auto [c1, c2] = tuple;
        return Rectangle{c1, c2, (std::abs(c1.Row - c2.Row) + 1) * (std::abs(c1.Column - c2.Column) + 1)};
    };
    auto ret = symmetricCartesianProduct(list) | std::views::transform(calcArea) | std::ranges::to<std::vector>();
    std::ranges::sort(ret, std::ranges::greater{}, &Rectangle::Area);
    return ret;
}

std::int64_t areaOfLargestRedAndGreenRectangle(std::span<const Rectangle> rectangles, const List& list) noexcept {
    auto isInPolygon = [&list](Coordinate c) {
        static std::flat_map<Coordinate, std::uint8_t> cache;
        if ( auto iter = cache.find(c); iter != cache.end() ) {
            return static_cast<bool>(iter->second);
        } //if ( auto iter = cache.find(c); iter != cache.end() )

        bool ret = false;

        for ( const auto& [a, b] :
              std::views::concat(list, std::views::single(list.front())) | std::views::adjacent<2> ) {
            if ( a.Row == b.Row ) {
                const auto left  = std::min(a.Column, b.Column);
                const auto right = std::max(a.Column, b.Column);
                if ( c.Column >= left && c.Column <= right ) {
                    if ( c.Row == b.Row ) {
                        ret = true;
                        break;
                    } //if ( c.Row == b.Row )
                    // ret = !ret;
                } //if ( c.Column >= left && c.Column <= right )
            } //if ( a.Row == b.Row )
            else if ( a.Column == b.Column ) {
                const auto top    = std::min(a.Row, b.Row);
                const auto bottom = std::max(a.Row, b.Row);
                if ( c.Row >= top && c.Row <= bottom ) {
                    if ( c.Column == b.Column ) {
                        ret = true;
                        break;
                    } //if ( c.Column == b.Column )
                    ret = !ret;
                } //if ( c.Row >= left && c.Row <= right )
            } //else if ( a.Column == b.Column )
            else {
                const auto slope     = static_cast<double>(b.Row - a.Row) / static_cast<double>(b.Column - a.Column);
                const auto offset    = static_cast<double>(a.Row) - slope * static_cast<double>(a.Column);

                const auto onLineRow = slope * static_cast<double>(c.Column) + offset;
                const auto top       = std::min(a.Row, b.Row);
                const auto bottom    = std::max(a.Row, b.Row);
                if ( onLineRow >= top && onLineRow <= bottom ) {
                    ret = !ret;
                } //if ( onLineRow >= top && onLineRow <= bottom )
            } //else
            //if ( (a.Row > c.Row) != (b.Row > c.Row) ) {
            //    const auto slope = (c.Column - a.Column) * (b.Row - a.Row) - (b.Column - a.Column) * (c.Row - a.Row);
            //    if ( slope == 0 ) {
            //        ret = true;
            //        break;
            //    } //if ( slope == 0 )
            //    if ( (slope < 0) != (b.Row < a.Row) ) {
            //        ret = !ret;
            //    } //if ( (slope < 0) != (b.Row < a.Row) )
            //} //if ( (a.Row > c.Row) != (b.Row > c.Row) )
        } //for ( const auto& [a, b] : list | std::views::adjacent<2> )

        cache.emplace(c, ret);
        return ret;
    };
    auto isValidRedAndGreen = [&isInPolygon](const Rectangle& rectangle) {
        return isInPolygon({rectangle.C1.Row, rectangle.C2.Column}) &&
               isInPolygon({rectangle.C2.Row, rectangle.C1.Column});
    };
    return std::ranges::find_if(rectangles, isValidRedAndGreen)->Area;
}
} //namespace

bool challenge9(const std::vector<std::string_view>& input) {
    const auto list       = parse(input);
    const auto rectangles = calculateRectangles(list);
    const auto area1      = rectangles.front().Area;
    myPrint(" == Result of Part 1: {:d} ==\n", area1);

    const auto area2 = areaOfLargestRedAndGreenRectangle(rectangles, list);
    myPrint(" == Result of Part 2: {:d} ==\n", area2);

    return area1 == 4'769'758'290 && area2 == 20520794;
}
