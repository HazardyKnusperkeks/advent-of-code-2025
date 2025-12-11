#ifndef ASTAR_HPP
#define ASTAR_HPP

#include "helper.hpp"

#include <algorithm>
#include <concepts>
#include <cstdint>
#include <queue>
#include <ranges>
#include <unordered_map>

namespace AStar {
template<typename Function, typename Position>
concept CostFunctionFor = requires(const Function& f, Position a, Position b) {
                              { f(a, b) } noexcept -> std::same_as<std::int64_t>;
                          };

template<typename ContainerType, typename Position>
concept x = std::same_as<typename ContainerType::value_type, Position>;

template<typename Function, typename Position>
concept NeighborFunctionFor = requires(const Function& f, Position a) {
                                  { f(a) } noexcept -> std::ranges::viewable_range;
                                  { f(a) } -> x<Position>;
                              };

enum class Mode : std::uint8_t {
    Default,
    ReturnPath,
    AllPaths,
};

template<Mode TheMode, typename Position>
struct Path {
    void push_back(const Position&&) const noexcept {}

    [[nodiscard]] bool empty() const noexcept {
        return true;
    }
};

template<Mode TheMode, typename Position>
requires (TheMode == Mode::ReturnPath || TheMode == Mode::AllPaths)
struct Path<TheMode, Position> : std::vector<Position> {};

template<Mode TheMode, typename Position>
struct AStarResult {
    std::vector<Path<TheMode, Position>> Paths;
    std::int64_t                         Cost;
};

template<typename Position>
struct PairHash {
    static std::size_t operator()(const std::pair<Position, Position>& pair) noexcept {
        constexpr bool isEnum = std::is_enum_v<Position>;
        using Projection =
            std::conditional_t<isEnum, decltype([](Position p) { return std::to_underlying(p); }), std::identity>;
        using Hash = std::conditional_t<isEnum, std::hash<std::underlying_type_t<Position>>, std::hash<Position>>;
        constexpr Hash       hash;
        constexpr Projection projection;
        return hash(projection(pair.first)) ^ hash(projection(pair.second));
    }
};

template<Mode TheMode, typename Position, CostFunctionFor<Position> CostFunction,
         NeighborFunctionFor<Position> NeighborFunction, CostFunctionFor<Position> HeuristicFunction>
AStarResult<TheMode, Position>
findShortestPath(const Position& start, const Position& end, const CostFunction& costFunction,
                 const NeighborFunction& neighborFunction, const HeuristicFunction& heuristicFunction) {
    static std::unordered_map<std::pair<Position, Position>, AStarResult<TheMode, Position>, PairHash<Position>> cache;

    if ( start == end ) {
        return {{{}}, costFunction(end, end)};
    } //if ( start == end )

    if ( const auto iter = cache.find({start, end}); iter != cache.end() ) {
        return iter->second;
    } //if ( const auto iter = cache.find({start, end}); iter != cache.end() )

    AStarResult<TheMode, Position> result;
    result.Cost = -1;

    struct Node {
        Position                Pos;
        Path<TheMode, Position> PathSoFar;
        std::int64_t            Cost;
        std::int64_t            Heuristic;

        bool operator<(const Node& that) const noexcept {
            return Heuristic > that.Heuristic;
        }
    };

    auto buildHeuristic = [end, &heuristicFunction](const Position& pos) noexcept {
        return heuristicFunction(pos, end);
    };

    std::priority_queue<Node>                  queue;
    std::unordered_map<Position, std::int64_t> visited;

    queue.emplace(start, Path<TheMode, Position>{}, 0, buildHeuristic(start));

    while ( !queue.empty() ) {
        Node current = queue.top();
        queue.pop();

        if ( current.Pos == end ) {
            if ( result.Cost == -1 ) {
                throwIfInvalid(result.Paths.empty());
                result.Cost = current.Cost;
            } //if ( result.Cost == -1 )
            else if ( current.Cost > result.Cost ) {
                cache.emplace(std::pair{start, end}, result);
                return result;
            } //else if ( current.Cost > result.Cost )

            throwIfInvalid(current.PathSoFar.back() == end);
            result.Paths.emplace_back(std::move(current.PathSoFar));

            if constexpr ( TheMode != Mode::AllPaths ) {
                cache.emplace(std::pair{start, end}, result);
                return result;
            } //if constexpr ( Mode != Mode::AllPaths )
            continue;
        } //if ( current.Pos == end )

        if constexpr ( TheMode == Mode::AllPaths ) {
            if ( result.Cost != -1 && current.Cost >= result.Cost ) {
                cache.emplace(std::pair{start, end}, result);
                return result;
            } //if ( result.Cost != -1 && current.Cost >= result.Cost )
        } //if constexpr ( TheMode == Mode::AllPaths )

        if ( const auto iter = visited.find(current.Pos); iter != visited.end() ) {
            if ( iter->second < current.Cost ) {
                continue;
            } //if ( iter->second < current.Cost )

            if constexpr ( TheMode != Mode::AllPaths ) {
                if ( iter->second <= current.Cost ) {
                    continue;
                } //if ( iter->second <= current.Cost )
            } //if constexpr ( TheMode != Mode::AllPaths )

            iter->second = current.Cost;
        } //if ( const auto iter = visited.find(current.Pos); iter != visited.end() )
        else {
            visited.emplace(current.Pos, current.Cost);
        } //else -> if ( const auto iter = visited.find(current.Pos); iter != visited.end() )

        std::ranges::for_each(
            neighborFunction(current.Pos) |
                std::views::transform([&current, &costFunction, &buildHeuristic](const Position& pos) noexcept {
                    const auto cost = current.Cost + costFunction(current.Pos, pos);
                    return Node{pos,
                                std::views::concat(current.PathSoFar, std::views::single(pos)) |
                                    std::ranges::to<std::vector>(),
                                cost, cost + buildHeuristic(pos)};
                }),
            [&queue](Node node) noexcept { queue.push(std::move(node)); });
    } //while ( !queue.empty() )

    return result;
}
} //namespace AStar

#endif //ASTAR_HPP
