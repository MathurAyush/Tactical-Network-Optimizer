#pragma once
#include "Graph.h"
#include "ShortestPath.h"
#include <vector>

struct TSPResult {
    std::vector<int> path;   // ordered visit sequence including return to start
    int  cost  = INF;
    bool found = false;      // false if graph is not fully connected
};

// -----------------------------------------------------------------------------
//  TSP - Travelling Salesman Problem for patrol / resupply route planning
//    * Brute Force      - exact, O(n!); practical only for n <= 11
//    * Nearest Neighbor - O(n^2) greedy heuristic; fast approximation
// -----------------------------------------------------------------------------
class TSP {
public:
    // Exact optimal tour (warns if n > 11)
    static TSPResult bruteForce(const Graph& g, int start = 0);
    // Fast heuristic
    static TSPResult nearestNeighbor(const Graph& g, int start = 0);

    static void printResult(const TSPResult& res, const Graph& g,
                            const std::string& algorithm);
    static void printComplexity(const std::string& algo, int n, long long ms);

private:
    static void permute(const std::vector<std::vector<int>>& mat,
                        std::vector<int>& nodes, int l, int r,
                        int start, TSPResult& best);
};
