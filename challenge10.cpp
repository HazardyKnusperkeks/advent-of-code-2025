#include "challenge10.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
#include <deque>
#include <flat_set>
#include <future>
#include <iterator>
#include <numeric>
#include <queue>
#include <ranges>
#include <thread>
#include <vector>

namespace {
using Button = std::vector<std::int64_t>;

struct Machine {
    std::vector<bool>         TargetLights;
    std::vector<Button>       Buttons;
    std::vector<std::int64_t> Joltage;
};

std::vector<Machine> parse(std::span<const std::string_view> input) {
    auto toMachine = [](const std::string_view line) {
        Machine ret;
        for ( auto section : splitString(line, ' ') ) {
            section.remove_suffix(1);
            switch ( section.front() ) {
                case '[' : {
                    ret.TargetLights = section | std::views::drop(1) |
                                       std::views::transform([](char c) noexcept { return c == '#'; }) |
                                       std::ranges::to<std::vector>();
                    break;
                } //case '['

                case '(' : {
                    auto& button = ret.Buttons.emplace_back();
                    std::ranges::transform(splitString(section.substr(1), ','), std::back_inserter(button),
                                           convert<10>);
                    break;
                } //case '('

                case '{' : {
                    std::ranges::transform(splitString(section.substr(1), ','), std::back_inserter(ret.Joltage),
                                           convert<10>);
                    break;
                } //case '{'
                default : throwIfInvalid(false);
            } //switch ( section.front() )
        } //for ( auto section : splitString(line, ' ') )
        return ret;
    };
    return input | std::views::transform(toMachine) | std::ranges::to<std::vector>();
}

std::int64_t fewestLightPresses(const Machine& machine) {
    using NodeT = std::vector<bool>;

    auto flip   = [](NodeT& node, std::int64_t index) noexcept {
        node[static_cast<std::size_t>(index)] = !node[static_cast<std::size_t>(index)];
        return;
    };

    struct NodeAndCost {
        NodeT        Node;
        std::int64_t Cost;
    };

    std::deque<NodeAndCost> queue;
    std::flat_set<NodeT>    visited;
    queue.emplace_back(NodeT(machine.TargetLights.size(), false), 0);

    while ( !queue.empty() ) {
        const auto current = queue.front();
        queue.pop_front();

        if ( visited.contains(current.Node) ) {
            continue;
        } //if ( visited.contains(current.Node) )

        if ( current.Node == machine.TargetLights ) {
            return current.Cost;
        } //if ( current.Node == machine.TargetLights )

        visited.insert(current.Node);

        for ( const auto& button : machine.Buttons ) {
            auto nextNode = current.Node;
            for ( auto light : button ) {
                flip(nextNode, light);
            } //for ( auto light : button )
            if ( !visited.contains(nextNode) ) {
                queue.emplace_back(std::move(nextNode), current.Cost + 1);
            } //if ( !visited.contains(nextNode) )
        } //for ( const auto& button : machine.Buttons )
    } //while ( !queue.empty() )
    throwIfInvalid(false);
    return -1;
}

std::atomic<int>  nextid   = 0;
thread_local auto threadId = []() { return ++nextid; }();

std::int64_t fewestJoltagePresses2(const Machine& machine) {
    using NodeT    = std::vector<std::int64_t>;

    const auto add = [](NodeT& node, const Button& button) noexcept {
        for ( auto light : button ) {
            ++node[static_cast<std::size_t>(light)];
        } //for ( auto light : button )
        return;
    };

    const auto heuristic = [&machine](const NodeT& node) noexcept {
        return std::ranges::max(std::views::zip_transform(std::minus<>{}, machine.Joltage, node));
    };

    struct NodeAndCost {
        NodeT        Node;
        std::int64_t Cost;
        std::int64_t CostAndHeuristic;

        bool operator<(const NodeAndCost& that) const noexcept {
            return std::tie(CostAndHeuristic, Cost) > std::tie(that.CostAndHeuristic, that.Cost);
        }
    };

    std::priority_queue<NodeAndCost> queue;
    std::flat_set<NodeT>             visited;
    queue.emplace(NodeT(machine.TargetLights.size(), false), 0, 0);

    while ( !queue.empty() ) {
        const auto current = queue.top();
        queue.pop();

        if ( visited.contains(current.Node) ) {
            continue;
        } //if ( visited.contains(current.Node) )

        if ( current.Node == machine.Joltage ) {
            return current.Cost;
        } //if ( current.Node == machine.Joltage )

        visited.insert(current.Node);

        if ( visited.size() % 10000 == 0 ) {
            std::println("{:8d} - {:3d} - {:3d} @ {:2d} todo {:8d}", visited.size(), current.Cost,
                         current.CostAndHeuristic, threadId, queue.size());
        } //if ( visited.size() % 10000 == 0 )

        for ( const auto& button : machine.Buttons ) {
            auto nextNode = current.Node;
            add(nextNode, button);
            if ( !visited.contains(nextNode) &&
                 std::ranges::all_of(std::views::zip_transform(std::ranges::less_equal{}, nextNode, machine.Joltage),
                                     [](bool b) noexcept { return b; }) ) {
                const auto heuristicCost = current.Cost + 1 + heuristic(nextNode);
                queue.emplace(std::move(nextNode), current.Cost + 1, heuristicCost);
            } //if ( std::ranges::all_of(std::views::zip_transform(std::ranges::less_equal{}, nextNode,
              //machine.Joltage),[](bool b)noexcept{return b;}) )
        } //for ( const auto& button : machine.Buttons )
    } //while ( !queue.empty() )
    throwIfInvalid(false);
    return -1;
}

template<typename T>
struct Fraction {
    public:
    T Numerator;
    T Denominator;

    constexpr Fraction(T value = {}) noexcept : Fraction{value, 1} {
        return;
    }

    constexpr Fraction(T numerator, T denominator) noexcept : Numerator{numerator}, Denominator{denominator} {
        return;
    }

    [[nodiscard]] T toBase(void) const {
        Fraction copy{*this};
        copy.normalize();
        throwIfInvalid(copy.Denominator == 1);
        return copy.Numerator;
    }

    [[nodiscard]] bool operator==(const Fraction& that) const noexcept {
        return std::abs(asDouble() - that.asDouble()) <= 1e-15;
    }

    [[nodiscard]] auto operator<=>(const Fraction& that) const noexcept {
        return asDouble() <=> that.asDouble();
    }

    Fraction& operator+=(const Fraction& that) noexcept {
        const auto lcm  = std::lcm(Denominator, that.Denominator);
        Numerator      *= lcm / Denominator;
        Numerator      += that.Numerator * (lcm / that.Denominator);
        Denominator     = lcm;
        normalize();
        return *this;
    }

    [[nodiscard]] Fraction operator-(void) const noexcept {
        Fraction ret{-Numerator, Denominator};
        return ret;
    }

    Fraction& operator-=(const Fraction& that) noexcept {
        return operator+=(-that);
    }

    [[nodiscard]] Fraction operator-(Fraction that) const noexcept {
        Fraction ret{*this};
        auto     lcm     = std::lcm(Denominator, that.Denominator);
        ret.Numerator   *= lcm / ret.Denominator;
        ret.Numerator   -= that.Numerator * (lcm / that.Denominator);
        ret.Denominator  = lcm;
        ret.normalize();
        return ret;
    }

    [[nodiscard]] Fraction operator*(Fraction that) const noexcept {
        Fraction ret{*this};
        ret.Numerator   *= that.Numerator;
        ret.Denominator *= that.Denominator;
        ret.normalize();
        return ret;
    }

    Fraction& operator/=(const Fraction& that) noexcept {
        Numerator   *= that.Denominator;
        Denominator *= that.Numerator;
        normalize();
        return *this;
    }

    [[nodiscard]] Fraction operator/(const Fraction& that) const noexcept {
        Fraction ret{*this};
        return ret /= that;
    }

    private:
    void normalize(void) noexcept {
        auto gcd     = std::gcd(Numerator, Denominator);
        Numerator   /= gcd;
        Denominator /= gcd;

        if ( Denominator < 0 ) {
            Numerator   = -Numerator;
            Denominator = -Denominator;
        } //if ( Denominator < 0 )
        return;
    }

    [[nodiscard]] double asDouble(void) const noexcept {
        return static_cast<double>(Numerator) / static_cast<double>(Denominator);
    }
};

std::int64_t fewestJoltagePresses(const Machine& machine) {
    std::vector<std::int64_t> buttonCounter;
    buttonCounter.resize(machine.Joltage.size());

    for ( const auto& button : machine.Buttons ) {
        for ( const auto jolt : button ) {
            ++buttonCounter[static_cast<std::size_t>(jolt)];
        } //for ( const auto jolt : button )
    } //for ( const auto& button : machine.Buttons )

    throwIfInvalid(std::ranges::count(buttonCounter, 0) == 0);

    //auto print = [&buttonCounter](std::int64_t x) {
    //std::print("{:d}: {:d} - ", x, std::ranges::count(buttonCounter, x));
    //};
    // for ( auto i = 0; i < 5; ++i ) {
    //print(i);
    //}
    //std::println("");

    const auto numberOfArtificalVariables = machine.Joltage.size();
    const auto numberOfVariables          = machine.Buttons.size();
    const auto numberOfRows               = machine.Joltage.size() + 2;
    const auto numberOfColumns            = numberOfVariables + numberOfArtificalVariables + 1;
    using Fraction                        = Fraction<std::int64_t>;

    struct Tableau {
        std::size_t           Rows;
        std::size_t           Columns;
        std::size_t           ArtificalVariables;
        std::vector<Fraction> Data;
        std::size_t           KilledColumns = 0;
        bool                  InPhaseOne    = true;
        std::size_t           StartRow      = 0;

        Tableau(std::size_t rows, std::size_t columns, std::size_t artificalVariables) noexcept :
                Rows{rows}, Columns{columns}, ArtificalVariables{artificalVariables} {
            Data.resize(rows * columns);
        }

        Fraction& operator[](std::size_t row, std::size_t column) noexcept {
            return Data[row * Columns + column];
        }

        Fraction& at(std::size_t row, std::size_t column) noexcept {
            return operator[](row, column);
        }

        void print(void) const noexcept {
            auto printLine = [take = Columns - KilledColumns](const auto& line) noexcept {
                std::print("| ");
                for ( const Fraction& f : line | std::views::take(take) ) {
                    std::print("{:3d}/{:3d} | ", f.Numerator, f.Denominator);
                } //for ( const Fraction& f : line | std::views::take(take) )
                std::println();
                return;
            };
            std::ranges::for_each(Data | std::views::chunk(Columns) | std::views::drop(StartRow), printLine);
            return;
        }

        void addTo(std::size_t sourceRow, std::size_t destinationRow) noexcept {
            for ( auto column = 0zu; column < Columns - KilledColumns; ++column ) {
                at(destinationRow, column) += at(sourceRow, column);
            } //for ( auto column = 0zu; column < Columns - KilledColumns; ++column )
            return;
        }

        void subtractMultiple(std::size_t destinationRow, std::size_t sourceRow, const Fraction& factor) noexcept {
            for ( auto column = 0zu; column < Columns - KilledColumns; ++column ) {
                at(destinationRow, column) -= at(sourceRow, column) * factor;
            } //for ( auto column = 0zu; column < Columns - KilledColumns; ++column )
            return;
        }

        void divide(std::size_t row, const Fraction& value) noexcept {
            for ( auto column = 0zu; column < Columns - KilledColumns; ++column ) {
                at(row, column) /= value;
            } //for ( auto column = 0zu; column < Columns - KilledColumns; ++column )
            return;
        }

        std::int64_t solve(void) {
            while ( true ) {
                const auto pivotColumn =
                    std::ranges::max(
                        std::views::concat(
                            std::views::iota(0zu, Columns - 1 - KilledColumns) |
                                std::views::transform(
                                    [this](std::size_t column) noexcept { return std::pair{at(0, column), column}; }) |
                                std::views::filter([](const auto& pair) noexcept { return pair.first > 0; }),
                            std::views::single(std::pair{Fraction{}, static_cast<std::size_t>(-1)})),
                        {}, &std::pair<Fraction, std::size_t>::first)
                        .second;

                //NOLINTNEXTLINE
                if ( pivotColumn == static_cast<std::size_t>(-1) ) {
                    throwIfInvalid(false);
                } //if ( pivotColumn == static_cast<std::size_t>(-1) )

                const auto pivotRow =
                    std::ranges::min(std::views::iota(StartRow + 1uz, Rows) |
                                     std::views::transform([this, &pivotColumn](std::size_t row) noexcept {
                                         return std::pair{at(row, pivotColumn), row};
                                     }) |
                                     std::views::filter([](const auto& pair) noexcept { return pair.first > 0; }) |
                                     std::views::transform([this](auto pair) noexcept {
                                         pair.first = at(pair.second, Columns - KilledColumns - 1) / pair.first;
                                         return pair;
                                     }))
                        .second;

                divide(pivotRow, at(pivotRow, pivotColumn));

                for ( auto row = StartRow; row < Rows; ++row ) {
                    if ( row == pivotRow ) {
                        continue;
                    } //if ( row == pivotRow )

                    const auto factor = at(row, pivotColumn);
                    subtractMultiple(row, pivotRow, factor);
                } //for ( auto row = StartRow; row < Rows; ++row )

                if ( InPhaseOne ) {
                    for ( auto artificalVariable = 0zu; artificalVariable < ArtificalVariables; ++artificalVariable ) {
                        if ( auto column = Columns - 1 - KilledColumns - artificalVariable - 1; at(0, column) < 0 ) {
                            for ( auto nextColumn = column + 1; nextColumn < Columns - KilledColumns;
                                  ++column, ++nextColumn ) {
                                for ( auto row = 0zu; row < Rows; ++row ) {
                                    at(row, column) = at(row, nextColumn);
                                } //for ( auto row = 0zu; row < Rows; ++row )
                            } //for ( auto nextColumn = column + 1; nextColumn < Columns - KilledColumns; ++nextColumn )
                            ++KilledColumns;
                            --ArtificalVariables;
                            break;
                        } //if ( at(0, Columns - 1 - KilledColumns - artificalVariable - 1) < 0 )
                    } //for ( auto artificalVariable = 0zu; artificalVariable < ArtificalVariables; ++artificalVariable)

                    if ( at(0, Columns - KilledColumns - 1) == 0 ) {
                        auto ret = at(1, Columns - KilledColumns - 1);
                        throwIfInvalid(ret.Denominator == 1);
                        return ret.Numerator;
                        // throwIfInvalid(at(0, Columns - KilledColumns - 1) == 0);
                        // InPhaseOne = false;
                        // StartRow   = 1;
                    } //if ( ArtificalVariables == 0 )
                } //if ( InPhaseOne )

                // print();
                // std::println("===");
                // myFlush();
            } //while ( true )
            return -1;
        }
    };

    constexpr std::size_t artificalTargetRow              = 0;
    constexpr std::size_t officialTargetRow               = 1;
    constexpr std::size_t matrixOffset                    = 2;
    const auto            startColumnOfArtificalVariables = numberOfVariables;
    const auto            bColumn                         = numberOfColumns - 1;

    Tableau tableau{numberOfRows, numberOfColumns, numberOfArtificalVariables};

    //Setting the artifical target function and variable coefficient
    for ( auto column = 0zu; column < numberOfArtificalVariables; ++column ) {
        tableau[artificalTargetRow, startColumnOfArtificalVariables + column]    = -1;
        tableau[column + matrixOffset, startColumnOfArtificalVariables + column] = 1;
    } //for ( auto column = 0zu; column < numberOfArtificalVariables; ++column )

    //Setting the target function
    for ( auto column = 0zu; column < machine.Buttons.size(); ++column ) {
        tableau[officialTargetRow, column] = -1;
    } //for ( auto column = 0zu; column < machine.Buttons.size(); ++column )

    //Setting the b vector
    for ( auto row = 0zu; row < machine.Joltage.size(); ++row ) {
        tableau[row + matrixOffset, bColumn]                               = machine.Joltage[row];
        tableau[row + matrixOffset, startColumnOfArtificalVariables + row] = 1;
    } //for ( auto row = 2zu; row <= machine.Joltage.size() + 1; ++row )

    //Setting the A matrix
    for ( const auto& [id, button] : std::views::enumerate(machine.Buttons) ) {
        for ( const auto jolt : button ) {
            tableau[static_cast<std::size_t>(jolt) + matrixOffset, static_cast<std::size_t>(id)] = 1;
        } //for ( const auto jolt : button )
    } //for ( const auto& button : machine.Buttons )

    //Build the first valid solution with the artifical variables
    for ( auto i = 0zu; i < numberOfArtificalVariables; ++i ) {
        tableau.addTo(matrixOffset + i, artificalTargetRow);
    } //for ( auto i = 0zu; i < numberOfArtificalVariables; ++i )

    // tableau.print();
    // std::println("===");
    // myFlush();
    auto ret = tableau.solve();
    // tableau.print();

    return ret;
}
} //namespace

bool challenge10(const std::vector<std::string_view>& input) {
    const auto machines = parse(input);
    const auto sum1 = std::ranges::fold_left(machines | std::views::transform(fewestLightPresses), 0, std::plus<>{});
    myPrint(" == Result of Part 1: {:d} ==\n", sum1);

    auto       start     = now();
    auto       completed = 0;
    const auto total     = std::ssize(machines);
    std::mutex mutex;
    auto       update = [&](const Machine& machine) noexcept {
        const auto             ret      = fewestJoltagePresses(machine);
        const auto             now      = ::now();
        const auto             duration = std::chrono::duration_cast<std::chrono::duration<double>>(now - start);
        const std::scoped_lock lock{mutex};
        ++completed;
        const auto perSecond     = completed / duration.count();
        const auto remaining     = total - completed;
        const auto remainingTime = static_cast<double>(remaining) / perSecond;
        std::println("{:3d} ({:6.2f}%) {:8.2f} / min => {:6.2f} min @ {:2d}", completed,
                     static_cast<double>(completed) / static_cast<double>(total) * 100., perSecond * 60.,
                     remainingTime / 60., threadId);
        myFlush();
        return ret;
    };
    //auto futures =
    //    machines | std::views::chunk(machines.size() / std::thread::hardware_concurrency()) |
    //    std::views::transform([&update](const auto& machineRange) noexcept {
    //        return std::async(std::launch::async, [&machineRange, &update](void) noexcept {
    //            return std::ranges::fold_left(machineRange | std::views::transform(update), 0, std::plus<>{});
    //        });
    //    }) |
    //    std::ranges::to<std::vector>();
    //const auto sum2 =
    //    std::ranges::fold_left(futures | std::views::transform(&std::future<std::int64_t>::get), 0, std::plus<>{});
    const auto sum2 =
        std::ranges::fold_left(machines /*| std::views::take(1)*/ | std::views::transform(update), 0, std::plus<>{});
    myPrint(" == Result of Part 2: {:d} ==\n", sum2);

    return sum1 == 520 && sum2 == 20520794;
}
