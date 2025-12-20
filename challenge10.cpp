#include "challenge10.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
#include <flat_map>
#include <iterator>
#include <ranges>
#include <vector>

namespace {
enum Bool : std::int8_t { False, True }; //NOLINT

using Button                   = std::vector<std::int64_t>;
using Bools                    = std::vector<Bool>;
using Joltages                 = std::vector<std::int64_t>;
using Presses                  = std::uint64_t;

constexpr std::int64_t Invalid = 1000000;

Presses setBit(Presses presses, std::uint64_t bit) noexcept {
    presses |= 1 << bit;
    return presses;
}

Joltages& add(Joltages& joltages, const Button& button) noexcept { //NOLINT
    for ( const auto& joltage : button ) {
        ++joltages[static_cast<std::size_t>(joltage)];
    } //for ( const auto& joltage : button )
    return joltages;
}

Joltages& sub(Joltages& joltages, const Button& button) noexcept { //NOLINT
    for ( const auto& joltage : button ) {
        --joltages[static_cast<std::size_t>(joltage)];
    } //for ( const auto& joltage : button )
    return joltages;
}

Joltages pressesToJoltage(Presses presses, std::span<const Button> buttons, std::size_t joltageCount) noexcept {
    Joltages ret(joltageCount);
    for ( auto bit = 0u; bit < buttons.size(); ++bit ) {
        if ( presses & (1 << bit) ) {
            add(ret, buttons[bit]);
        } //if ( presses & (1 << bit) )
    } //for ( auto bit = 0u; bit < buttons.size(); ++bit )
    return ret;
}

Bool mod2(std::int64_t x) noexcept {
    return x % 2 ? True : False;
}

bool isNegative(std::int64_t x) noexcept {
    return x < 0;
}

struct Machine {
    Bools               TargetLights;
    std::vector<Button> Buttons;
    Joltages            Joltage;
};

std::vector<Machine> parse(std::span<const std::string_view> input) {
    auto toMachine = [](const std::string_view line) {
        Machine ret;
        for ( auto section : splitString(line, ' ') ) {
            section.remove_suffix(1);
            switch ( section.front() ) {
                case '[' : {
                    ret.TargetLights = section | std::views::drop(1) |
                                       std::views::transform([](char c) noexcept { return c == '#' ? True : False; }) |
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

std::vector<Presses> calculateAllPossibilities(const Bools& targetLights, std::span<const Button> buttons) noexcept {
    const auto& buttonCombos = [buttons](void) noexcept {
        static std::flat_map<std::int64_t, std::vector<Presses>> cache;
        const std::int64_t                                       limit = 1 << buttons.size();

        if ( const auto iter = cache.find(limit); iter != cache.end() ) {
            return iter->second;
        } //if ( const auto iter = cache.find(limit); iter != cache.end() )

        auto result =
            std::views::iota(0, limit) | std::views::transform([buttons](const std::int64_t buttonCombo) noexcept {
                return std::ranges::fold_left(std::views::iota(0uz, buttons.size()) |
                                                  std::views::filter([buttonCombo](std::int64_t bit) noexcept {
                                                      return buttonCombo & (1 << bit);
                                                  }),
                                              0, setBit);
            }) |
            std::ranges::to<std::vector>();
        cache.emplace(limit, result);
        return result;
    }();

    return buttonCombos | std::views::filter([&targetLights, buttons](Presses presses) noexcept {
               return std::ranges::equal(pressesToJoltage(presses, buttons, targetLights.size()), targetLights, {},
                                         mod2);
           }) |
           std::ranges::to<std::vector>();
}

std::int64_t fewestLightPresses(const Machine& machine) noexcept {
    return std::ranges::min(calculateAllPossibilities(machine.TargetLights, machine.Buttons) |
                            std::views::transform(std::popcount<std::uint64_t>));
}

std::int64_t fewestJoltagePressesImpl(const Joltages& joltages, std::span<const Button> buttons,
                                      std::flat_map<Bools, std::vector<Presses>>& cache) {
    //Not my idea, but:
    //https://www.reddit.com/r/adventofcode/comments/1pk87hl/2025_day_10_part_2_bifurcate_your_way_to_victory/
    if ( std::ranges::all_of(joltages, [](std::int64_t x) noexcept { return x == 0; }) ) {
        return 0;
    } //if ( std::ranges::all_of(joltages, [](std::int64_t x) noexcept { return x == 0; }) )

    const auto& possiblePresses = [&joltages, &buttons, &cache]() noexcept {
        const Bools targetLights = joltages | std::views::transform(mod2) | std::ranges::to<std::vector>();

        if ( auto iter = cache.find(targetLights); iter != cache.end() ) {
            return iter->second;
        } //if ( auto iter = cache.find(targetLights); iter != cache.end() )

        auto result = calculateAllPossibilities(targetLights, buttons);
        cache.emplace(targetLights, result);
        return result;
    }();

    auto applyPress = [&joltages, &buttons, &cache](const Presses presses) noexcept {
        const auto pressCount = std::popcount(presses);
        const auto half       = [](std::int64_t& x) noexcept {
            x /= 2;
            return;
        };
        auto remainingJoltage = joltages;

        for ( auto bit = 0u; bit < buttons.size(); ++bit ) {
            if ( presses & (1 << bit) ) {
                sub(remainingJoltage, buttons[bit]);
            } //if ( presses & (1 << bit) )
        } //for ( auto bit = 0u; bit < buttons.size(); ++bit )

        if ( std::ranges::any_of(remainingJoltage, isNegative) ) {
            return Invalid;
        } //if ( std::ranges::any_of(remainingJoltage, isNegative) )

        std::ranges::for_each(remainingJoltage, half);

        return pressCount + 2 * fewestJoltagePressesImpl(remainingJoltage, buttons, cache);
    };

    if ( possiblePresses.empty() ) {
        return Invalid;
    } //if ( possiblePresses.empty() )

    return std::ranges::min(possiblePresses | std::views::transform(applyPress));
}

std::int64_t fewestJoltagePresses(const Machine& machine) noexcept {
    std::flat_map<Bools, std::vector<Presses>> cache;
    return fewestJoltagePressesImpl(machine.Joltage, machine.Buttons, cache);
}
} //namespace

bool challenge10(const std::vector<std::string_view>& input) {
    const auto machines = parse(input);
    const auto sum1 = std::ranges::fold_left(machines | std::views::transform(fewestLightPresses), 0, std::plus<>{});
    myPrint(" == Result of Part 1: {:d} ==\n", sum1);

    const auto sum2 = std::ranges::fold_left(machines | std::views::transform(fewestJoltagePresses), 0, std::plus<>{});
    myPrint(" == Result of Part 2: {:d} ==\n", sum2);

    return sum1 == 520 && sum2 == 20626;
}
