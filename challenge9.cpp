#include "challenge9.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
#include <flat_map>
#include <flat_set>
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
    //auto isValidRedAndGreen = [&containsMap](const Rectangle& rectangle) noexcept {
    //    auto top    = rectangle.C1;
    //    auto bottom = rectangle.C2;

    //if ( top.Row > bottom.Row ) {
    //    std::swap(top, bottom);
    //} //if ( top.Row > bottom.Row )

    //const auto left  = std::min(rectangle.C1.Column, rectangle.C2.Column);
    //const auto right = std::max(rectangle.C1.Column, rectangle.C2.Column);

    //// std::println("\nChecking Area {:d} ({:}) x ({:})", rectangle.Area, rectangle.C1, rectangle.C2);

    //for ( auto row = top.Row; row <= bottom.Row; ++row ) {
    //    const auto& rowMap = containsMap.Map[row];
    //    throwIfInvalid(rowMap.size() == 2);
    //    if ( left < *rowMap.begin() ) {
    //        return false;
    //    } //if ( left < *rowMap.begin() )
    //    if ( right > *rowMap.rbegin() ) {
    //        return false;
    //    } //if ( right > *rowMap.rbegin() )
    //} //for ( auto row = top.Row; row <= bottom.Row; ++row )

    //// std::println("Is Valid");
    //return true;
    //};
    //// std::ranges::for_each(rectangles | std::views::take(250), isValidRedAndGreen);
    //return 0;
    //return std::ranges::find_if(rectangles, isValidRedAndGreen)->Area;
    return 0;
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
