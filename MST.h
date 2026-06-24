#pragma once
#include "Graph.h"
#include <vector>
#include <string>

struct MSTResult {
    std::vector<Edge> edges;   // edges selected for the MST
    int totalWeight = 0;
    bool valid = false;        // false if graph is disconnected
};

// -----------------------------------------------------------------------------
//  MST - Minimum Spanning Tree algorithms
//    * Kruskal  (sort + Union-Find)
//    * Prim     (greedy with priority queue)
// -----------------------------------------------------------------------------
class MST {
public:
    static MSTResult kruskal(const Graph& g);
    static MSTResult prim(const Graph& g, int start = 0);
    static void printResult(const MSTResult& result, const Graph& g,
                            const std::string& algorithm);
    static void printASCIITree(const MSTResult& result, const Graph& g, int root = 0);
    static void printComplexity(const std::string& algo, int V, int E, long long ms);

private:
    // Union-Find helpers for Kruskal
    static int  find(std::vector<int>& parent, int x);
    static void unite(std::vector<int>& parent, std::vector<int>& rank,
                      int x, int y);
};
