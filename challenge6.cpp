#include "challenge6.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
#include <iterator>
#include <ranges>
#include <vector>

namespace {
enum Operation : char { Addition = '+', Multiplication = '*' };

struct Problem {
    std::vector<std::int64_t> Numbers;
    Operation                 Op;
};

struct Math {
    std::vector<Problem> Problems;
};

Math parse(std::span<const std::string_view> input) {
    Math ret;

    auto addToNumbers = [&ret](std::string_view line) {
        std::ranges::for_each(std::views::zip(splitString(line, ' '), ret.Problems),
                              [](std::tuple<std::string_view, Problem&> numberAndProblem) {
                                  auto&& [number, problem] = numberAndProblem;
                                  problem.Numbers.push_back(convert(number));
                                  return;
                              });
        return;
    };

    std::ranges::transform(splitString(input.back(), ' '), std::back_inserter(ret.Problems), [](std::string_view op) {
        throwIfInvalid(op.size() == 1);
        return Problem{{}, static_cast<Operation>(op.front())};
    });
    std::ranges::for_each(input.subspan(0, input.size() - 1), addToNumbers);
    return ret;
}

void reparseFancy(Math& math, MapView map) {
    using Coordinate = ::Coordinate<std::int64_t>;
    Coordinate::setMaxFromMap(map);
    Coordinate lastSpacer{Coordinate::MaxRow - 1, -1};

    auto reparse = [&lastSpacer, &map](Problem& problem) {
        problem.Numbers.clear();
        auto nextSpacer = lastSpacer.right();
        for ( nextSpacer.move(Direction::Right); nextSpacer.isValid() && map[nextSpacer] == ' ';
              nextSpacer.move(Direction::Right) ) {
        } //for ( nextSpacer.move(Direction::Right); map[nextSpacer] == ' '; nextSpacer.move(Direction::Right) )

        if ( nextSpacer.isValid() ) {
            nextSpacer.move(Direction::Left);
        } //if ( nextSpacer.isValid() )
        for ( Coordinate numberStart{0, nextSpacer.Column - 1}; numberStart.Column != lastSpacer.Column;
              numberStart.move(Direction::Left) ) {
            std::int64_t& number = problem.Numbers.emplace_back(0);
            for ( auto digitPos = numberStart; digitPos.Row < nextSpacer.Row; digitPos.move(Direction::Down) ) {
                auto digit = map[digitPos];
                if ( digit == ' ' ) {
                    continue;
                } //if ( digit == ' ' )
                number *= 10;
                number += toDigit(digit);
            } //for ( auto digitPos = numberStart; digitPos.Row < nextSpacer.Row; digitPos.move(Direction::Down) )
        }

        lastSpacer = nextSpacer;
        return;
    };
    std::ranges::for_each(math.Problems, reparse);
    return;
}

template<typename F>
std::int64_t solveProblemImpl(const Problem& problem, std::int64_t init, F&& function) noexcept {
    return std::ranges::fold_left(problem.Numbers, init, std::forward<F>(function));
}

std::int64_t solveProblem(const Problem& problem) noexcept {
    if ( problem.Op == Operation::Addition ) {
        return solveProblemImpl(problem, 0, std::plus<>{});
    } //if ( problem.Op == Operation::Addition )
    return solveProblemImpl(problem, 1, std::multiplies<>{});
}
} //namespace

bool challenge6(const std::vector<std::string_view>& input) {
    auto       math = parse(input);
    const auto sum1 = std::ranges::fold_left(math.Problems | std::views::transform(solveProblem), 0, std::plus<>{});
    myPrint(" == Result of Part 1: {:d} ==\n", sum1);

    reparseFancy(math, input);
    const auto sum2 = std::ranges::fold_left(math.Problems | std::views::transform(solveProblem), 0, std::plus<>{});
    myPrint(" == Result of Part 2: {:d} ==\n", sum2);

    return sum1 == 6'605'396'225'322 && sum2 == 11'052'310'600'986;
}
