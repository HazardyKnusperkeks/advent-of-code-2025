#include "challenge9.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
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
               return Coordinate{.Row = convert(line.substr(comma + 1)), .Column = convert(line.substr(0, comma))};
           }) |
           std::ranges::to<std::vector>();
}

std::vector<Rectangle> calculateRectangles(const List& list) noexcept {
    auto calcArea = [](const std::tuple<Coordinate, Coordinate>& tuple) noexcept {
        auto [c1, c2] = tuple;
        if ( c2.Row < c1.Row ) {
            std::swap(c1, c2);
        } //if ( c2.Row < c1.Row )
        return Rectangle{.C1   = c1,
                         .C2   = c2,
                         .Area = (std::abs(c1.Row - c2.Row) + 1) * (std::abs(c1.Column - c2.Column) + 1)};
    };
    auto ret = symmetricCartesianProduct(list) | std::views::transform(calcArea) | std::ranges::to<std::vector>();
    std::ranges::sort(ret, std::ranges::greater{}, &Rectangle::Area);
    return ret;
}

std::int64_t areaOfLargestRedAndGreenRectangle(std::span<const Rectangle> rectangles, const List& list) {
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

    for ( const auto& [a, b] : std::views::concat(list, std::views::single(list.front())) | std::views::adjacent<2> ) {
        const auto [minRow, maxRow]       = std::minmax(a.Row, b.Row);
        const auto [minColumn, maxColumn] = std::minmax(a.Column, b.Column);
        const Edge edge{.From = Coordinate{.Row = minRow, .Column = minColumn},
                        .To   = Coordinate{.Row = maxRow, .Column = maxColumn}};

        if ( minRow == maxRow ) {
            horizontalEdges.emplace(edge);
        } //if ( minRow == maxRow )
        else {
            verticalEdges.emplace(edge);
        } //else -> if ( minRow == maxRow )
    } //for ( const auto& [a, b] : list | std::views::adjacent<2> )

    const auto toFromRow    = [](const Edge& edge) noexcept { return edge.From.Row; };
    const auto toFromColumn = [](const Edge& edge) noexcept { return edge.From.Column; };
    const auto toToColumn   = [](const Edge& edge) noexcept { return edge.To.Column; };
    const auto toToRow      = [](const Edge& edge) noexcept { return edge.To.Row; };

    const auto hits         = [](const Coordinate& c) noexcept {
        return [c](const Edge& edge) noexcept { return edge.From == c || edge.To == c; };
    };

    auto checkLeftToRight = [&horizontalEdges, &verticalEdges, &toFromRow, &toFromColumn,
                             &hits](const std::int64_t row, std::int64_t left, const std::int64_t right,
                                    const bool topAllowed, const std::int64_t rowLimit) {
        const auto edgesOnThisRow         = std::ranges::equal_range(horizontalEdges, row, {}, toFromRow);
        const auto lowerBoundCuttingEdges = std::ranges::upper_bound(verticalEdges, left, {}, toFromColumn);
        const auto upperBoundCuttingEdges =
            std::ranges::lower_bound(lowerBoundCuttingEdges, verticalEdges.end(), right, {}, toFromColumn);

        auto       horizontalEdge     = std::ranges::lower_bound(edgesOnThisRow, left, {}, toFromColumn);
        const auto horizontalEnd      = edgesOnThisRow.end();
        auto       verticalEdge       = lowerBoundCuttingEdges;
        const auto verticalEnd        = upperBoundCuttingEdges;
        bool       firstEdge          = true;
        bool       checkForTopAllowed = false;

        if ( horizontalEdge->From.Row != row ) {
            //Shouldn't have looked for toFromColumn, but toToColumn, adapt manually, this is O(1).
            --horizontalEdge;
            checkForTopAllowed = true;
            throwIfInvalid(horizontalEdge->From.Row == row);
        } //if ( horizontalEdge->From.Row != row )

        const auto advanceVertical = [&verticalEdge, &verticalEnd, row](std::int64_t column) noexcept {
            verticalEdge = std::ranges::find_if(verticalEdge, verticalEnd, [row, column](const Edge& edge) noexcept {
                return edge.From.Row <= row && row <= edge.To.Row && edge.From.Column >= column;
            });
            return;
        };

        advanceVertical(left);

        while ( left < right ) {
            if ( verticalEdge != verticalEnd && verticalEdge->From.Column < horizontalEdge->From.Column ) {
                //Crossing a vertical Edge -> Instant disquallify.
                return false;
            } //if ( verticalEdge != verticalEnd && verticalEdge->From.Column < horizontalEdge->From.Column )

            if ( horizontalEdge->From.Column <= right && right <= horizontalEdge->To.Column ) {
                //The end is on an edge, this is in the loop.
                return true;
            } //if ( horizontalEdge->From.Column <= right && right <= horizontalEdge->To.Column )

            if ( firstEdge ) {
                firstEdge                = false;

                const auto leftVertical  = std::ranges::find_if(verticalEdges, hits(horizontalEdge->From));
                const auto rightVertical = std::ranges::find_if(verticalEdges, hits(horizontalEdge->To));

                const auto leftGoesUp    = leftVertical->From.Row < horizontalEdge->From.Row;
                const auto rightGoesUp   = rightVertical->From.Row < horizontalEdge->To.Row;

                if ( checkForTopAllowed ) {
                    if ( rightGoesUp != topAllowed ) {
                        const auto otherSide = (rightGoesUp ? rightVertical->From : rightVertical->To).Row;
                        if ( (!topAllowed && otherSide > rowLimit) || (topAllowed && otherSide < rowLimit) ) {
                            return false;
                        } //if ( (!topAllowed && otherSide > rowLimit) || (topAllowed && otherSide < rowLimit) )
                    } //if ( bottomGoesLeft == topAllowed )
                } //if ( checkForTopAllowed )
                else {
                    if ( leftGoesUp == rightGoesUp ) {
                        //The Target isn't on the starting Edge and it's not a step, thus we leave the loop.
                        return false;
                    } //if ( leftGoesUp == rightGoesUp )

                } //else -> if ( checkForTopAllowed )
            } //if ( firstEdge )

            const auto leftVertical  = std::ranges::find_if(verticalEdge, verticalEnd, hits(horizontalEdge->From));
            const auto rightVertical = std::ranges::find_if(leftVertical, verticalEnd, hits(horizontalEdge->To));

            if ( leftVertical != verticalEnd ) {
                throwIfInvalid(rightVertical != verticalEnd);
                const auto leftGoesUp  = leftVertical->From.Row < horizontalEdge->From.Row;
                const auto rightGoesUp = rightVertical->From.Row < horizontalEdge->To.Row;

                if ( leftGoesUp != rightGoesUp ) {
                    //Hit an edge which is just a step, one side of that is outside the loop.
                    return false;
                } //if ( leftGoesUp != rightGoesUp )
            } //if ( leftVertical != verticalEnd )

            left = horizontalEdge->To.Column + 1;
            ++horizontalEdge;
            advanceVertical(left);

            if ( horizontalEdge == horizontalEnd || horizontalEdge->From.Row != row ) {
                //No edges on this available anymore, only check the vertical ones.
                return verticalEdge == verticalEnd;
            } //if ( horizontalEdge == horizontalEnd || horizontalEdge->From.Row != row )
        } //while ( left < right )

        return true;
    };

    auto checkRightToLeft = [&horizontalEdges, &verticalEdges, &toFromRow, &toFromColumn, &toToColumn,
                             &hits](const std::int64_t row, std::int64_t right, const std::int64_t left,
                                    const bool bottomAllowed, const std::int64_t rowLimit) {
        const auto edgesOnThisRow = std::ranges::equal_range(horizontalEdges, row, {}, toFromRow) | std::views::reverse;
        const auto lowerBoundCuttingEdges = std::ranges::upper_bound(verticalEdges, left, {}, toFromColumn);
        const auto upperBoundCuttingEdges =
            std::ranges::lower_bound(lowerBoundCuttingEdges, verticalEdges.end(), right, {}, toFromColumn);

        auto       horizontalEdge        = std::ranges::lower_bound(edgesOnThisRow, right, {}, toToColumn);
        const auto horizontalEnd         = edgesOnThisRow.end();
        auto       verticalEdge          = std::reverse_iterator{upperBoundCuttingEdges};
        const auto verticalEnd           = std::reverse_iterator{lowerBoundCuttingEdges};
        bool       firstEdge             = true;
        bool       checkForBottomAllowed = false;

        if ( horizontalEdge->From.Row != row ) {
            //Shouldn't have looked for toToColumn, but toFromColumn, adapt manually, this is O(1).
            --horizontalEdge;
            checkForBottomAllowed = true;
            throwIfInvalid(horizontalEdge->From.Row == row);
        } //if ( horizontalEdge->From.Row != row )

        const auto advanceVertical = [&verticalEdge, &verticalEnd, row](std::int64_t column) noexcept {
            verticalEdge = std::ranges::find_if(verticalEdge, verticalEnd, [row, column](const Edge& edge) noexcept {
                return edge.From.Row <= row && row <= edge.To.Row && edge.From.Column <= column;
            });
            return;
        };

        advanceVertical(right);

        while ( right > left ) {
            if ( verticalEdge != verticalEnd && verticalEdge->From.Column > horizontalEdge->From.Column ) {
                //Crossing a vertical Edge -> Instant disquallify.
                return false;
            } //if ( verticalEdge != verticalEnd && verticalEdge->From.Column > horizontalEdge->From.Column )

            if ( horizontalEdge->From.Column <= left && left <= horizontalEdge->To.Column ) {
                //The end is on an edge, this is in the loop.
                return true;
            } //if ( horizontalEdge->From.Column <= left && left <= horizontalEdge->To.Column )

            if ( firstEdge ) {
                firstEdge                = false;

                const auto leftVertical  = std::ranges::find_if(verticalEdges, hits(horizontalEdge->From));
                const auto rightVertical = std::ranges::find_if(verticalEdges, hits(horizontalEdge->To));

                const auto leftGoesUp    = leftVertical->From.Row < horizontalEdge->From.Row;
                const auto rightGoesUp   = rightVertical->From.Row < horizontalEdge->To.Row;

                if ( checkForBottomAllowed ) {
                    if ( rightGoesUp == bottomAllowed ) {
                        const auto otherSide = (rightGoesUp ? rightVertical->From : rightVertical->To).Row;
                        if ( (!bottomAllowed && otherSide > rowLimit) || (bottomAllowed && otherSide < rowLimit) ) {
                            return false;
                        } //if ( (!bottomAllowed && otherSide > rowLimit) || (bottomAllowed && otherSide < rowLimit) )
                    } //if ( rightGoesUp == bottomAllowed )
                } //if ( checkForBottomAllowed )
                else {
                    if ( leftGoesUp == rightGoesUp ) {
                        //The Target isn't on the starting Edge and it's not a step, thus we leave the loop.
                        return false;
                    } //if ( leftGoesUp == rightGoesUp )
                } //else -> if ( checkForBottomAllowed )
            } //if ( firstEdge )

            const auto rightVertical = std::ranges::find_if(verticalEdge, verticalEnd, hits(horizontalEdge->To));
            const auto leftVertical  = std::ranges::find_if(rightVertical, verticalEnd, hits(horizontalEdge->From));

            if ( leftVertical != verticalEnd ) {
                throwIfInvalid(rightVertical != verticalEnd);
                const auto leftGoesUp  = leftVertical->From.Row < horizontalEdge->From.Row;
                const auto rightGoesUp = rightVertical->From.Row < horizontalEdge->To.Row;

                if ( leftGoesUp != rightGoesUp ) {
                    //Hit an edge which is just a step, one side of that is outside the loop.
                    return false;
                } //if ( leftGoesUp != rightGoesUp )
            } //if ( leftVertical != verticalEnd )

            right = horizontalEdge->From.Column - 1;
            ++horizontalEdge;
            advanceVertical(right);

            if ( horizontalEdge == horizontalEnd || horizontalEdge->From.Row != row ) {
                //No edges on this available anymore, only check the vertical ones.
                return verticalEdge == verticalEnd;
            } //if ( horizontalEdge == horizontalEnd || horizontalEdge->From.Row != row )
        } //while ( right > left )

        return true;
    };

    auto checkTopToBottom = [&horizontalEdges, &verticalEdges, &toFromRow, &toFromColumn,
                             &hits](const std::int64_t column, std::int64_t top, const std::int64_t bottom,
                                    const bool leftAllowed, const std::int64_t columnLimit) {
        const auto edgesOnThisColumn      = std::ranges::equal_range(verticalEdges, column, {}, toFromColumn);
        const auto lowerBoundCuttingEdges = std::ranges::upper_bound(horizontalEdges, top, {}, toFromRow);
        const auto upperBoundCuttingEdges =
            std::ranges::lower_bound(lowerBoundCuttingEdges, horizontalEdges.end(), bottom, {}, toFromRow);

        auto       verticalEdge        = std::ranges::lower_bound(edgesOnThisColumn, top, {}, toFromRow);
        const auto verticalEnd         = edgesOnThisColumn.end();
        auto       horizontalEdge      = lowerBoundCuttingEdges;
        const auto horizontalEnd       = upperBoundCuttingEdges;
        bool       firstEdge           = true;
        bool       checkForLeftAllowed = false;

        if ( verticalEdge == verticalEnd || verticalEdge->From.Column != column ) {
            //Shouldn't have looked for toFromRow, but toToRow, adapt manually, this is O(1).
            --verticalEdge;
            checkForLeftAllowed = true;
            throwIfInvalid(verticalEdge->From.Column == column);
        } //if ( verticalEdge == verticalEnd || verticalEdge->From.Column != column )

        const auto advanceHorizontal = [&horizontalEdge, &horizontalEnd, column](std::int64_t row) noexcept {
            horizontalEdge =
                std::ranges::find_if(horizontalEdge, horizontalEnd, [row, column](const Edge& edge) noexcept {
                    return edge.From.Column < column && column < edge.To.Column && edge.From.Row >= row;
                });
            return;
        };

        advanceHorizontal(top);

        while ( top < bottom ) {
            if ( horizontalEdge != horizontalEnd && horizontalEdge->From.Row < verticalEdge->From.Row ) {
                //Crossing a vertical Edge -> Instant disquallify.
                return false;
            } //if ( horizontalEdge != horizontalEnd && horizontalEdge->From.Row < verticalEdge->From.Row )

            if ( verticalEdge->From.Row <= bottom && bottom <= verticalEdge->To.Row ) {
                //The end is on an edge, this is in the loop.
                return true;
            } //if ( verticalEdge->From.Row <= bottom && bottom <= verticalEdge->To.Row )

            if ( firstEdge ) {
                firstEdge                   = false;

                const auto topHorizontal    = std::ranges::find_if(horizontalEdges, hits(verticalEdge->From));
                const auto bottomHorizontal = std::ranges::find_if(horizontalEdges, hits(verticalEdge->To));

                const auto topGoesLeft      = topHorizontal->From.Column < verticalEdge->From.Column;
                const auto bottomGoesLeft   = bottomHorizontal->From.Column < verticalEdge->To.Column;

                if ( checkForLeftAllowed ) {
                    if ( bottomGoesLeft != leftAllowed ) {
                        const auto otherSide = (bottomGoesLeft ? bottomHorizontal->From : bottomHorizontal->To).Column;
                        if ( (!leftAllowed && otherSide > columnLimit) || (leftAllowed && otherSide < columnLimit) ) {
                            return false;
                        } //if ( (!leftAllowed && otherSide > columnLimit) || (leftAllowed && otherSide < columnLimit) )
                    } //if ( bottomGoesLeft != leftAllowed )
                } //if ( checkForLeftAllowed )
                else {
                    if ( topGoesLeft == bottomGoesLeft ) {
                        //The Target isn't on the starting Edge and it's not a step, thus we leave the loop.
                        return false;
                    } //if ( topGoesLeft == bottomGoesLeft )
                } //else -> if ( checkForLeftAllowed )
            } //if ( firstEdge )

            const auto topHorizontal    = std::ranges::find_if(horizontalEdge, horizontalEnd, hits(verticalEdge->From));
            const auto bottomHorizontal = std::ranges::find_if(topHorizontal, horizontalEnd, hits(verticalEdge->To));

            if ( topHorizontal != bottomHorizontal ) {
                throwIfInvalid(topHorizontal != verticalEnd);
                const auto topGoesLeft    = topHorizontal->From.Column < horizontalEdge->From.Column;
                const auto bottomGoesLeft = bottomHorizontal->From.Column < horizontalEdge->To.Column;

                if ( topGoesLeft != bottomGoesLeft ) {
                    //Hit an edge which is just a step, one side of that is outside the loop.
                    return false;
                } //if ( topGoesLeft != bottomGoesLeft )
            } //if ( topHorizontal != bottomHorizontal )

            top = verticalEdge->To.Row + 1;
            ++verticalEdge;
            advanceHorizontal(top);

            if ( verticalEdge == verticalEnd || verticalEdge->From.Column != column ) {
                //No edges on this available anymore, only check the vertical ones.
                return horizontalEdge == horizontalEnd;
            } //if ( verticalEdge == verticalEnd || verticalEdge->From.Column != column )
        } //while ( top < bottom )

        return true;
    };

    auto checkBottomToTop = [&horizontalEdges, &verticalEdges, &toFromRow, &toFromColumn, &toToRow,
                             &hits](const std::int64_t column, std::int64_t bottom, const std::int64_t top,
                                    const bool rightAllowed, const std::int64_t columnLimit) {
        const auto edgesOnThisRow =
            std::ranges::equal_range(verticalEdges, column, {}, toFromColumn) | std::views::reverse;
        const auto lowerBoundCuttingEdges = std::ranges::upper_bound(horizontalEdges, top, {}, toFromRow);
        const auto upperBoundCuttingEdges =
            std::ranges::lower_bound(lowerBoundCuttingEdges, horizontalEdges.end(), bottom, {}, toFromRow);

        auto       verticalEdge         = std::ranges::lower_bound(edgesOnThisRow, top, {}, toToRow);
        const auto verticalEnd          = edgesOnThisRow.end();
        auto       horizontalEdge       = std::reverse_iterator{upperBoundCuttingEdges};
        const auto horizontalEnd        = std::reverse_iterator{lowerBoundCuttingEdges};
        bool       firstEdge            = true;
        bool       checkForRightAllowed = false;

        if ( verticalEdge->From.Column != column ) {
            //Shouldn't have looked for toToRow, but toFromRow, adapt manually, this is O(1).
            --verticalEdge;
            checkForRightAllowed = true;
            throwIfInvalid(verticalEdge->From.Column == column);
        } //if ( verticalEdge->From.Column != column )

        const auto advanceHorizontal = [&horizontalEdge, &horizontalEnd, column](std::int64_t row) noexcept {
            horizontalEdge =
                std::ranges::find_if(horizontalEdge, horizontalEnd, [row, column](const Edge& edge) noexcept {
                    return edge.From.Column <= column && column <= edge.To.Column && edge.From.Row <= row;
                });
            return;
        };

        advanceHorizontal(bottom);

        while ( bottom > top ) {
            if ( horizontalEdge != horizontalEnd && horizontalEdge->From.Row > verticalEdge->To.Row ) {
                //Crossing a vertical Edge -> Instant disquallify.
                return false;
            } //if ( horizontalEdge != horizontalEnd && horizontalEdge->From.Row > verticalEdge->To.Row )

            if ( verticalEdge->From.Row <= top && top <= verticalEdge->To.Row ) {
                //The end is on an edge, this is in the loop.
                return true;
            } //if ( verticalEdge->From.Row <= top && top <= verticalEdge->To.Row )

            if ( firstEdge ) {
                firstEdge                   = false;

                const auto topHorizontal    = std::ranges::find_if(horizontalEdges, hits(verticalEdge->From));
                const auto bottomHorizontal = std::ranges::find_if(horizontalEdges, hits(verticalEdge->To));

                const auto topGoesLeft      = topHorizontal->From.Column < verticalEdge->From.Column;
                const auto bottomGoesLeft   = bottomHorizontal->From.Column < verticalEdge->To.Column;

                if ( checkForRightAllowed ) {
                    if ( bottomGoesLeft == rightAllowed ) {
                        const auto otherSide = (bottomGoesLeft ? bottomHorizontal->From : bottomHorizontal->To).Column;
                        if ( (!rightAllowed && otherSide > columnLimit) || (rightAllowed && otherSide < columnLimit) ) {
                            return false;
                        } //if ( (!rightAllowed && otherSide > columnLimit) || (rightAllowed && otherSide < columnLimit)
                          //)
                    } //if ( bottomGoesLeft != leftAllowed )
                } //if ( checkForRightAllowed )
                else {
                    if ( topGoesLeft == bottomGoesLeft ) {
                        //The Target isn't on the starting Edge and it's not a step, thus we leave the loop.
                        return false;
                    } //if ( topGoesLeft == bottomGoesLeft )
                } //else -> if ( checkForRightAllowed )
            } //if ( firstEdge )

            const auto bottomHorizontal = std::ranges::find_if(horizontalEdge, horizontalEnd, hits(verticalEdge->To));
            const auto topHorizontal = std::ranges::find_if(bottomHorizontal, horizontalEnd, hits(verticalEdge->From));

            if ( topHorizontal != horizontalEnd ) {
                throwIfInvalid(bottomHorizontal != horizontalEnd);
                const auto topGoesLeft    = topHorizontal->From.Column < verticalEdge->From.Column;
                const auto bottomGoesLeft = bottomHorizontal->From.Column < verticalEdge->To.Column;

                if ( topGoesLeft != bottomGoesLeft ) {
                    //Hit an edge which is just a step, one side of that is outside the loop.
                    return false;
                } //if ( topGoesLeft != bottomGoesLeft )
            } //if ( topHorizontal != horizontalEnd )

            bottom = verticalEdge->From.Row - 1;
            ++verticalEdge;
            advanceHorizontal(bottom);

            if ( verticalEdge == verticalEnd || verticalEdge->From.Column != column ) {
                //No edges on this available anymore, only check the vertical ones.
                return horizontalEdge == horizontalEnd;
            } //if ( verticalEdge == verticalEnd || verticalEdge->From.Column != column )
        } //while ( right > left )

        return true;
    };

    auto isValidRedAndGreen = [&checkLeftToRight, &checkRightToLeft, &checkTopToBottom,
                               &checkBottomToTop](const Rectangle& rectangle) {
        if ( rectangle.C1.Column < rectangle.C2.Column ) {
            return checkLeftToRight(rectangle.C1.Row, rectangle.C1.Column, rectangle.C2.Column, /*topAllowed=*/true,
                                    /*rowLimit=*/rectangle.C2.Row) &&
                   checkTopToBottom(rectangle.C1.Column, rectangle.C1.Row, rectangle.C2.Row, /*leftAllowed=*/true,
                                    /*columnLimit=*/rectangle.C2.Column) &&
                   checkRightToLeft(rectangle.C2.Row, rectangle.C2.Column, rectangle.C1.Column, /*bottomAllowed=*/true,
                                    /*rowLimit=*/rectangle.C1.Row) &&
                   checkBottomToTop(rectangle.C2.Column, rectangle.C2.Row, rectangle.C1.Row, /*rightAllowed=*/true,
                                    /*columnLimit=*/rectangle.C1.Column);
        } //if ( rectangle.C1.Column < rectangle.C2.Column )

        return checkRightToLeft(rectangle.C1.Row, rectangle.C1.Column, rectangle.C2.Column, /*bottomAllowed=*/false,
                                /*rowLimit=*/rectangle.C2.Row) &&
               checkTopToBottom(rectangle.C1.Column, rectangle.C1.Row, rectangle.C2.Row, /*leftAllowed=*/false,
                                /*columnLimit=*/rectangle.C2.Column) &&
               checkLeftToRight(rectangle.C2.Row, rectangle.C2.Column, rectangle.C1.Column, /*topAllowed=*/false,
                                /*rowLimit=*/rectangle.C1.Row) &&
               checkBottomToTop(rectangle.C2.Column, rectangle.C2.Row, rectangle.C1.Row, /*rightAllowed=*/false,
                                /*columnLimit=*/rectangle.C1.Column);
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

    return area1 == 4'769'758'290 && area2 == 1'588'990'708;
}
