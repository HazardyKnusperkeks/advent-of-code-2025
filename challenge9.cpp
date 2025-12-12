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
    struct Edge {
        Coordinate From;
        Coordinate To;

        constexpr auto operator<=>(const Edge& that) const noexcept = default;
    };

    struct ColumnLess {
        static bool operator()(const Edge& lhs, const Edge& rhs) noexcept {
            return std::tie(lhs.From.Column, lhs.From.Row, lhs.To.Column, lhs.To.Row) <
                   std::tie(rhs.From.Column, rhs.From.Row, rhs.To.Column, rhs.To.Row);
        }
    };

    std::flat_set<Edge, ColumnLess> verticalEdges;
    std::flat_set<Edge>             horizontalEdges;
    std::flat_set<Coordinate>       set = list | std::ranges::to<std::flat_set>();

    for ( const auto& [a, b] : std::views::concat(list, std::views::single(list.front())) | std::views::adjacent<2> ) {
        const auto [minRow, maxRow]       = std::minmax(a.Row, b.Row);
        const auto [minColumn, maxColumn] = std::minmax(a.Column, b.Column);

        if ( minRow == maxRow ) {
            horizontalEdges.emplace(Coordinate{.Row = minRow, .Column = minColumn}, Coordinate{minRow, maxColumn});
        } //if ( minRow == maxRow )
        else {
            verticalEdges.emplace(Coordinate{.Row = minRow, .Column = minColumn}, Coordinate{maxRow, maxColumn});
        } //else -> if ( minRow == maxRow )
    } //for ( const auto& [a, b] : list | std::views::adjacent<2> )

    auto isInInnerPolygon = [&verticalEdges, &horizontalEdges](const std::int64_t row, const std::int64_t startColumn,
                                                               const std::int64_t endColumn, const bool firstRow) {
        const auto horizontalEdgesOnThisRow =
            std::ranges::equal_range(horizontalEdges, row, {}, [](const Edge& edge) noexcept { return edge.From.Row; });

        std::println("R {}, C {}->{}:", row, startColumn, endColumn);
        for ( auto edge : horizontalEdgesOnThisRow ) {
            std::println("  Row {} => {}", edge.From, edge.To);
        } //for ( auto edge : horizontalEdgesOnThisRow )

        const auto findRow = [&horizontalEdgesOnThisRow](std::int64_t column) noexcept {
            return std::ranges::find_if(horizontalEdgesOnThisRow, [column](const Edge& edge) noexcept {
                return edge.From.Column <= column && column <= edge.To.Column;
            });
        };

        const auto edgeEmbodyingStart = findRow(startColumn);
        const auto edgeEmbodyingEnd   = findRow(endColumn);
        const auto end                = horizontalEdgesOnThisRow.end();
        auto       toColumn           = [](const Edge& edge) noexcept { return edge.To.Column; };

        if ( edgeEmbodyingStart == edgeEmbodyingEnd && edgeEmbodyingStart != end ) {
            //If both match, and they are the same we're done here.
            std::println("Yas");
            return true;
        } //if ( edgeEmbodyingStart == edgeEmbodyingEnd && edgeEmbodyingStart != end )

        if ( (edgeEmbodyingStart == end || edgeEmbodyingEnd == end) && edgeEmbodyingStart != edgeEmbodyingEnd ) {
            const auto matching         = edgeEmbodyingStart == end ? edgeEmbodyingEnd : edgeEmbodyingStart;
            const auto findVerticalEdge = [&verticalEdges, &toColumn](const Coordinate& c) noexcept {
                const auto range = std::ranges::equal_range(verticalEdges, c.Column, {}, toColumn);
                return std::ranges::find_if(range,
                                            [c](const Edge& edge) noexcept { return edge.From == c || edge.To == c; });
            };
            const auto leftEdge  = findVerticalEdge(matching->From);
            const auto rightEdge = findVerticalEdge(matching->To);

            const bool isLeftUp  = leftEdge->From.Row > leftEdge->To.Row;
            const bool isRightUp = rightEdge->From.Row > rightEdge->To.Row;

            if ( firstRow && isLeftUp != isRightUp ) {
                //First Row must have both, if there is only one Edge involved
                std::println("First Row ney");
                return false;
            } //if ( firstRow && isLeftUp != isRightUp )

            if ( isLeftUp == isRightUp && isLeftUp ) {
                std::println("Row ney");
                return false;
            } //if ( isLeftUp == isRightUp && isLeftUp )
        } //if ( (edgeEmbodyingStart == end || edgeEmbodyingEnd == end) && edgeEmbodyingStart != edgeEmbodyingEnd )

        const auto lower_bound = std::ranges::lower_bound(verticalEdges, startColumn, {}, toColumn);
        const auto upper_bound = std::ranges::upper_bound(lower_bound, verticalEdges.end(), endColumn, {}, toColumn);

        for ( auto iter = lower_bound; iter != upper_bound; ++iter ) {
            std::println("  Col {} => {}", iter->From, iter->To);
            if ( iter->From.Row < row && row < iter->To.Row ) {
                std::println("Ney");
                return false;
            } //if ( iter->From.Row < row && row < iter->To.Row )
        } //for ( auto iter = lower_bound; iter != upper_bound; ++iter )

        std::println("Fallback yas");
        return true;
        /*

        while ( c.Column >= 0 ) {
            //auto nextHorizontalEdge = std::ranges::lower_bound(horizontalEdgesOnThisRow, c.Column, {}, toColumn);
            //if ( nextHorizontalEdge == horizontalEdgesOnThisRow.end() || nextHorizontalEdge->From.Column > c.Column )
            //{
            //    //No horizontal edges on our way anymore.
            //    break;
            //} //if ( nextHorizontalEdge == horizontalEdgesOnThisRow.end() || nextHorizontalEdge->From.Column >
            //c.Column)

            const auto horizontalEdgeBound = std::ranges::upper_bound(horizontalEdgesOnThisRow, c.Column, {}, toColumn);
            if ( horizontalEdgeBound == horizontalEdgesOnThisRow.begin() ) {
                //No horizontal edges on our way anymore.
                break;
            } //if ( horizontalEdgeBound == horizontalEdgesOnThisRow.begin() )

            const auto rend = std::reverse_iterator{horizontalEdgesOnThisRow.begin()};
            std::println("Edges: ");
            for ( auto iter = std::reverse_iterator{horizontalEdgeBound}; iter != rend; ++iter ) {
                std::println("  {} -> {}", iter->From, iter->To);
            }
            const auto nextHorizontalEdge =
                std::ranges::find_if(std::reverse_iterator{horizontalEdgeBound}, rend, [&c](const Edge& edge) {
                    std::println("Check Edge {} -> {}", edge.From, edge.To);
                    throwIfInvalid(edge.From.Row == c.Row);
                    return edge.From.Column <= c.Column;
                });

            myFlush();
            if ( nextHorizontalEdge == rend ) {
                //No horizontal edges on our way anymore.
                break;
            } //if ( nextHorizontalEdge == rend )

            const auto& horizontalEdge    = *nextHorizontalEdge;
            bool        skipHorizontal    = false;
            const auto  verticalEdgeBound = std::ranges::upper_bound(verticalEdges, c.Column, {}, toColumn);
            if ( verticalEdgeBound != verticalEdges.begin() ) {
                auto* verticalEdge = &*std::prev(verticalEdgeBound);
                for ( std::reverse_iterator iter{verticalEdgeBound};
                      iter != verticalEdges.rend() && iter->From.Column > nextHorizontalEdge->From.Column;
                      ++iter, --verticalEdge ) {
                    if ( iter->From.Row <= c.Row && c.Row <= iter->To.Row /*&&
                         (iter->From.Row > nextHorizontalEdge->From.Row ||
                          nextHorizontalEdge->From.Row > iter->To.Row)* ) {
                        skipHorizontal = skipHorizontal || false;
                        if ( iter->From == nextHorizontalEdge->From || iter->From == nextHorizontalEdge->To ||
                             iter->To == nextHorizontalEdge->From || iter->To == nextHorizontalEdge->To ) {
                            myFlush();
                            skipHorizontal = true;
                        }
                        ret      = !ret;
                        c.Column = iter->From.Column - 1;
                    } //if ( iter->From.Row <= c.Row && c.Row <= iter->To.Row )
                } //for
            } //if ( verticalEdgeBound != verticalEdges.begin() )
            if ( !skipHorizontal ) {
                ret      = !ret;
                c.Column = nextHorizontalEdge->From.Column - 1;
            } //if ( !skipHorizontal )
        } //while ( c.Column >= 0 )

        if ( c.Column >= 0 ) {
            const auto verticalEdgeBound = std::ranges::upper_bound(verticalEdges, c.Column, {}, toColumn);
            if ( verticalEdgeBound != verticalEdges.begin() ) {
                for ( std::reverse_iterator iter{verticalEdgeBound}; iter != verticalEdges.rend(); ++iter ) {
                    if ( iter->From.Row <= c.Row && c.Row <= iter->To.Row ) {
                        ret      = !ret;
                        c.Column = iter->From.Column - 1;
                    } //if ( iter->From.Row <= c.Row && c.Row <= iter->To.Row )
                } //for ( std::reverse_iterator iter{std::prev(verticalEdge)}; iter != verticalEdges.rend(); ++iter )
            } //if ( verticalEdgeBound != verticalEdges.begin() )
        } //if ( c.Column >= 0 )

        cache.emplace(c, ret);
        return ret;*/
    };
    auto isValidRedAndGreen = [&isInInnerPolygon](const Rectangle& rectangle) {
        const auto [minRow, maxRow]       = std::ranges::minmax(rectangle.C1.Row, rectangle.C2.Row);
        const auto [minColumn, maxColumn] = std::ranges::minmax(rectangle.C1.Column, rectangle.C2.Column);

        std::println("\n\nNext {} -> {} / {} -> {} A: {}", minRow, maxRow, minColumn, maxColumn, rectangle.Area);
        return std::ranges::all_of(
            std::views::iota(minRow, maxRow + 1),
            [minColumn, maxColumn, &isInInnerPolygon, firstRow = true](std::int64_t row) mutable noexcept {
                return isInInnerPolygon(row, minColumn, maxColumn, std::exchange(firstRow, false));
            });
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
