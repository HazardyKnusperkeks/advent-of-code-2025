#include "challenge11.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
#include <flat_map>
#include <ranges>

using namespace std::string_view_literals;

namespace {
struct Graph {
    std::flat_map<std::string_view, std::vector<std::string_view>> Nodes;
};

Graph parse(std::span<const std::string_view> input) {
    auto toNode = [](std::string_view line) {
        auto colon = line.find(':');
        throwIfInvalid(colon != std::string_view::npos);
        auto node = line.substr(0, colon);
        return std::pair{node, splitString(line.substr(colon + 1), ' ') | std::ranges::to<std::vector>()};
    };
    return {input | std::views::transform(toNode) | std::ranges::to<std::flat_map>()};
}

std::int64_t countPaths(const Graph& graph, std::string_view current, std::string_view to) {
    struct CacheEntry {
        std::string_view From;
        std::string_view To;

        constexpr auto operator<=>(const CacheEntry&) const noexcept = default;
    };

    static std::flat_map<CacheEntry, std::int64_t> cache;

    if ( auto iter = cache.find({.From = current, .To = to}); iter != cache.end() ) {
        return iter->second;
    } //if ( auto iter = cache.find({.From = current, .To = to}); iter != cache.end() )

    if ( current == to ) {
        return 1;
    } //if ( current == to )

    auto node = graph.Nodes.find(current);
    if ( node == graph.Nodes.end() ) {
        return 0;
    } //if ( node == graph.Nodes.end() )

    auto ret = std::ranges::fold_left(node->second | std::views::transform([&graph, &to](std::string_view next) {
                                          return countPaths(graph, next, to);
                                      }),
                                      0, std::plus<>{});
    cache.emplace(CacheEntry{.From = current, .To = to}, ret);
    return ret;
}
} //namespace

bool challenge11(const std::vector<std::string_view>& input) {
    const auto graph            = parse(input);
    const auto pathCountFromYou = countPaths(graph, "you", "out");
    myPrint(" == Result of Part 1: {:d} ==\n", pathCountFromYou);

    const auto pathCountFromSvrToDac = countPaths(graph, "svr", "dac");
    const auto pathCountFromSvrToFft = countPaths(graph, "svr", "fft");
    const auto pathCountFromDacToFft = countPaths(graph, "dac", "fft");
    const auto pathCountFromFftToDac = countPaths(graph, "fft", "dac");
    const auto pathCountFromFftToOut = countPaths(graph, "fft", "out");
    const auto pathCountFromDacToOut = countPaths(graph, "dac", "out");
    const auto pathCountFromSvr      = pathCountFromSvrToDac * pathCountFromDacToFft * pathCountFromFftToOut +
                                       pathCountFromSvrToFft * pathCountFromFftToDac * pathCountFromDacToOut;
    myPrint(" == Result of Part 2: {:d} ==\n", pathCountFromSvr);

    return pathCountFromYou == 497 && pathCountFromSvr == 358'564'784'931'864;
}
