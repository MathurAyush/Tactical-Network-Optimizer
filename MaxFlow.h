#pragma once
#include "Graph.h"
#include <vector>

struct FlowResult {
    int  maxFlow;
    std::vector<std::vector<int>> flow; // flow[u][v] on each arc
};

// -----------------------------------------------------------------------------
//  MaxFlow - Edmonds-Karp algorithm (BFS-based Ford-Fulkerson)
//  Models maximum supply throughput from a source base to a sink base.
//  Time complexity: O(V * E^2)
// -----------------------------------------------------------------------------
class MaxFlow {
public:
    static FlowResult edmondsKarp(const Graph& g, int source, int sink);
    static void printResult(const FlowResult& res, const Graph& g,
                            int source, int sink);
    static void printComplexity(int V, int E, long long ms);

private:
    // BFS to find an augmenting path in the residual graph
    static bool bfsAugment(const std::vector<std::vector<int>>& cap,
                           int src, int sink, int n,
                           std::vector<int>& parent);
};
