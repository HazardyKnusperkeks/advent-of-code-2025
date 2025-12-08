#include "challenge8.hpp"

#include "coordinate3d.hpp"
#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
#include <iterator>
#include <ranges>
#include <vector>

namespace {
struct JunctionBox {
    Coordinate3D<std::int64_t> Coordinate;
    std::int64_t               Circuit = -1;
};

struct Problem {
    struct Distance {
        double      Distance;
        std::size_t Index1;
        std::size_t Index2;
    };

    std::vector<JunctionBox>  Boxes;
    std::vector<std::int64_t> CircuitSize;
    std::vector<Distance>     Distances;
    std::int64_t              ProductOfLastConnection;

    void connect(void) noexcept {
        auto [_, i1, i2]       = Distances.back();
        auto&      b1          = Boxes[i1];
        auto&      b2          = Boxes[i2];
        const auto hasCircuit1 = b1.Circuit != -1;
        const auto hasCircuit2 = b2.Circuit != -1;
        Distances.pop_back();

        if ( !hasCircuit1 && !hasCircuit2 ) {
            const auto circuit = CircuitSize.size();
            b1.Circuit = b2.Circuit = static_cast<std::int64_t>(circuit);
            CircuitSize.emplace_back(2);
            ProductOfLastConnection = b1.Coordinate.X * b2.Coordinate.X;
        } //if ( !hasCircuit1 && !hasCircuit2 )
        else if ( hasCircuit1 && !hasCircuit2 ) {
            b2.Circuit = b1.Circuit;
            ++CircuitSize[static_cast<std::size_t>(b1.Circuit)];
            ProductOfLastConnection = b1.Coordinate.X * b2.Coordinate.X;
        } //else if ( hasCircuit1 && !hasCircuit2 )
        else if ( !hasCircuit1 && hasCircuit2 ) {
            b1.Circuit = b2.Circuit;
            ++CircuitSize[static_cast<std::size_t>(b1.Circuit)];
            ProductOfLastConnection = b1.Coordinate.X * b2.Coordinate.X;
        } //else if ( !hasCircuit1 && hasCircuit2 )
        else if ( b1.Circuit != b2.Circuit ) {
            CircuitSize[static_cast<std::size_t>(b1.Circuit)] +=
                std::exchange(CircuitSize[static_cast<std::size_t>(b2.Circuit)], 0);
            const auto hasB2Circuit = [b2](const JunctionBox& box) noexcept { return box.Circuit == b2.Circuit; };
            for ( auto& box : Boxes | std::views::filter(hasB2Circuit) ) {
                box.Circuit = b1.Circuit;
            } //for ( auto& box : Boxes | std::views::filter(hasB2Circuit) )
            ProductOfLastConnection = b1.Coordinate.X * b2.Coordinate.X;
        } //else if ( b1.Circuit != b2.Circuit )
        return;
    }

    std::int64_t connectShortest(int connections) noexcept {
        for ( ; connections; --connections ) {
            connect();
        } //for ( ; connections; --connections )

        auto temp = CircuitSize;
        std::ranges::nth_element(temp, std::next(temp.begin(), 3), std::ranges::greater{});
        return std::ranges::fold_left(temp | std::views::take(3), 1, std::multiplies<>{});
    }

    void keepConnecting(void) noexcept {
        while ( !Distances.empty() ) {
            connect();
        } //while ( !Distances.empty() )
        return;
    }
};

Problem parse(std::span<const std::string_view> input) {
    Problem ret;
    auto    toBox = [](std::string_view line) {
        auto numbers = splitString(line, ',');
        auto iter    = numbers.begin();
        throwIfInvalid(iter != numbers.end());
        auto x = convert(*iter);
        throwIfInvalid(iter != numbers.end());
        auto y = convert(*++iter);
        throwIfInvalid(iter != numbers.end());
        auto z = convert(*++iter);
        throwIfInvalid(++iter == numbers.end());
        return JunctionBox{{x, y, z}, -1};
    };
    std::ranges::transform(input, std::back_inserter(ret.Boxes), toBox);
    const auto indices = std::views::iota(0uz, ret.Boxes.size());
    ret.Distances      = std::views::cartesian_product(indices, indices) |
                    std::views::filter([](const std::tuple<std::size_t, std::size_t>& tuple) noexcept {
                        return std::get<0>(tuple) > std::get<1>(tuple);
                    }) |
                    std::views::transform([ret](const std::tuple<std::size_t, std::size_t>& tuple) noexcept {
                        const auto i1 = std::get<0>(tuple);
                        const auto i2 = std::get<1>(tuple);
                        const auto c1 = ret.Boxes[i1].Coordinate;
                        const auto c2 = ret.Boxes[i2].Coordinate;
                        return Problem::Distance{(c1 - c2).length(), i1, i2};
                    }) |
                    std::ranges::to<std::vector>();

    std::ranges::sort(ret.Distances, std::ranges::greater{}, &Problem::Distance::Distance);

    return ret;
}
} //namespace

bool challenge8(const std::vector<std::string_view>& input) {
    auto       problem  = parse(input);
    const auto product1 = problem.connectShortest(1000);
    myPrint(" == Result of Part 1: {:d} ==\n", product1);

    problem.keepConnecting();
    const auto product2 = problem.ProductOfLastConnection;
    myPrint(" == Result of Part 2: {:d} ==\n", product2);

    return product1 == 81536 && product2 == 7'017'750'530;
}
