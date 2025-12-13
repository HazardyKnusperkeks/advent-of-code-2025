#include "challenge12.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
#include <flat_set>
#include <iterator>
#include <ranges>

namespace {
constexpr auto NumberOfPresents = 6;

struct Present {
    std::array<bool, 9> Cells;
    std::int64_t        Area;

    constexpr auto operator<=>(const Present&) const noexcept = default;

    template<typename Self>
    decltype(auto) operator[](this Self&& self, std::int64_t row, std::int64_t column) noexcept {
        return *std::next(std::forward<Self>(self).Cells.begin(), row * 3 + column);
    }

    [[nodiscard]] Present flip(void) const noexcept {
        Present ret{};
        for ( auto row = 0; row < 3; ++row ) {
            ret[row, 0] = (*this)[row, 2];
            ret[row, 1] = (*this)[row, 1];
            ret[row, 2] = (*this)[row, 0];
        } //for ( auto row = 0; row < 3; ++row )
        ret.Area = Area;
        return ret;
    }

    [[nodiscard]] Present rotate(void) const noexcept {
        Present ret{};
        ret[0, 0] = (*this)[0, 2];
        ret[0, 1] = (*this)[1, 2];
        ret[0, 2] = (*this)[2, 2];
        ret[1, 0] = (*this)[0, 1];
        ret[1, 1] = (*this)[1, 1];
        ret[1, 2] = (*this)[2, 1];
        ret[2, 0] = (*this)[0, 0];
        ret[2, 1] = (*this)[1, 0];
        ret[2, 2] = (*this)[2, 0];
        ret.Area  = Area;
        return ret;
    }
};

std::flat_set<Present> toPresents(std::ranges::subrange<std::span<const std::string_view>::iterator> range) {
    std::flat_set<Present> ret;
    auto                   rangeToConvert = range | std::views::drop(1) | std::views::take(3);
    Present                present{};
    std::ranges::for_each(std::views::enumerate(rangeToConvert), [&present](const auto& idAndLine) {
        auto [row, line] = idAndLine;
        std::ranges::for_each(std::views::enumerate(line), [&row, &present](const auto& idAndCell) {
            auto [column, cell] = idAndCell;
            switch ( cell ) {
                case '.' : present[row, column] = false; break;
                case '#' : {
                    present[row, column] = true;
                    ++present.Area;
                    break;
                } //case '#'
                default : throwIfInvalid(false);
            } //switch ( cell )
            return;
        });
        return;
    });

    ret.insert(present);
    ret.insert(present.flip());
    ret.insert(present.rotate());
    ret.insert(present.rotate().flip());
    return ret;
}

struct Tree {
    std::int64_t                               Width;
    std::int64_t                               Length;
    std::int64_t                               Area;
    std::array<std::int64_t, NumberOfPresents> PresentCount;
};

Tree toTree(std::string_view line) {
    auto x = line.find('x');
    throwIfInvalid(x != std::string_view::npos);
    auto colon = line.find(':', x + 1);
    throwIfInvalid(colon != std::string_view::npos);
    Tree ret{};
    ret.Width         = convert(line.substr(0, x));
    ret.Length        = convert(line.substr(x + 1, colon - x));
    ret.Area          = ret.Width * ret.Length;
    auto presentCount = splitString(line.substr(colon + 1), ' ');
    throwIfInvalid(std::ranges::distance(presentCount) == NumberOfPresents);
    std::ranges::transform(presentCount, ret.PresentCount.begin(), convert<10>);
    return ret;
}

struct Problem {
    std::array<std::flat_set<Present>, NumberOfPresents> Presents;
    std::vector<Tree>                                    Trees;
};

Problem parse(std::span<const std::string_view> input) {
    constexpr auto linesPerPresent = 5;
    constexpr auto offset          = static_cast<std::size_t>(linesPerPresent) * NumberOfPresents;
    throwIfInvalid(input.size() > offset);
    auto presentRange = input.subspan(0, offset);
    auto treeRange    = input.subspan(offset);

    Problem ret;
    std::ranges::transform(presentRange | std::views::chunk(linesPerPresent), ret.Presents.begin(), toPresents);
    ret.Trees = treeRange | std::views::transform(toTree) | std::ranges::to<std::vector>();
    return ret;
}

struct FitTree {
    const Tree&                                Base; //NOLINT
    std::vector<bool>                          Occupied;
    std::array<std::int64_t, NumberOfPresents> RemainingCount;

    explicit FitTree(const Tree& base) noexcept : Base{base}, RemainingCount{Base.PresentCount} {
        Occupied.resize(static_cast<std::size_t>(Base.Area));
        return;
    }

    bool set(const Present& present, std::int64_t top, std::int64_t left) {
        const auto toIndex = [this, &left, &top](std::tuple<int, int> rowAndColumn) noexcept {
            auto [row, column] = rowAndColumn;
            return static_cast<std::size_t>((row + top) * Base.Width + (column + left));
        };
        auto range = std::views::cartesian_product(std::views::iota(0, 3), std::views::iota(0, 3)) |
                     std::views::filter([&present](std::tuple<int, int> rowAndColumn) noexcept {
                         auto [row, column] = rowAndColumn;
                         return present[row, column];
                     }) |
                     std::views::transform(toIndex);

        const auto isBlocked = [this](std::size_t index) noexcept { return static_cast<bool>(Occupied[index]); };
        if ( std::ranges::any_of(range, isBlocked) ) {
            return false;
        } //if ( std::ranges::any_of(range, isBlocked) ) )

        std::ranges::for_each(range, [this](std::size_t index) noexcept {
            Occupied[index] = true;
            return;
        });
        return true;
    }

    void unset(const Present& present, std::int64_t top, std::int64_t left) noexcept {
        const auto toIndex = [this, &left, &top](std::tuple<int, int> rowAndColumn) noexcept {
            auto [row, column] = rowAndColumn;
            return static_cast<std::size_t>((row + top) * Base.Width + (column + left));
        };
        auto range = std::views::cartesian_product(std::views::iota(0, 3), std::views::iota(0, 3)) |
                     std::views::filter([&present](std::tuple<int, int> rowAndColumn) noexcept {
                         auto [row, column] = rowAndColumn;
                         return present[row, column];
                     }) |
                     std::views::transform(toIndex);
        std::ranges::for_each(range, [this](std::size_t index) noexcept {
            Occupied[index] = false;
            return;
        });
        return;
    }
};

bool fits(FitTree tree, const std::array<std::flat_set<Present>, NumberOfPresents>& allPresents) noexcept {
    const auto presentIter = std::ranges::find_if(tree.RemainingCount, [](std::int64_t x) noexcept { return x > 0; });
    if ( presentIter == tree.RemainingCount.end() ) {
        return true;
    } //if ( presentIter == tree.RemainingCount.end() )
    const auto& presentsToCheck =
        *std::next(allPresents.begin(), std::ranges::distance(tree.RemainingCount.begin(), presentIter));
    --*presentIter;

    for ( const auto& present : presentsToCheck ) {
        for ( auto row : std::views::iota(0, tree.Base.Length - 2) ) {
            for ( auto column : std::views::iota(0, tree.Base.Width - 2) ) {
                if ( tree.set(present, row, column) ) {
                    if ( fits(tree, allPresents) ) {
                        return true;
                    } //if ( fits(tree, allPresents) )

                    tree.unset(present, row, column);
                } //if ( tree.set(present, row, column) )
            } //for ( auto column : std::views::iota(0, tree.Base.Width - 2) )
        } //for ( auto row : std::views::iota(0, tree.Base.Length - 2) )
    } //for ( const auto& present : presentsToCheck )
    return false;
}

bool fits(const Tree& tree, const std::array<std::flat_set<Present>, NumberOfPresents>& allPresents) noexcept {
    const auto minimumAreaForPresents =
        std::ranges::fold_left(std::views::zip_transform(
                                   [](std::int64_t count, const std::flat_set<Present>& presents) noexcept {
                                       return count * presents.begin()->Area;
                                   },
                                   tree.PresentCount, allPresents),
                               0, std::plus<>{});
    if ( minimumAreaForPresents > tree.Area ) {
        //Easy, cut the problem in half!
        return false;
    } //if ( minimumAreaForPresents > tree.Area )
    return fits(FitTree{tree}, allPresents);
}
} //namespace

bool challenge12(const std::vector<std::string_view>& input) {
    const auto problem = parse(input);
    const auto count   = std::ranges::count_if(
        problem.Trees, [&problem](const Tree& tree) noexcept { return fits(tree, problem.Presents); });
    myPrint(" == Result of Part 1: {:d} ==\n", count);

    return count == 479;
}
