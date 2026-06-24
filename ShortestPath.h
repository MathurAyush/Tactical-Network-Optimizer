#pragma once
#include "Graph.h"
#include <vector>

// Result for single-source algorithms (Dijkstra / Bellman-Ford)
struct SPResult {
    std::vector<int> dist;       // dist[v] = shortest distance from source
    std::vector<int> prev;       // prev[v] = predecessor on shortest path (-1 if none)
    bool hasNegCycle = false;    // Bellman-Ford negative-cycle detection
};

// Result for all-pairs Floyd-Warshall
struct APSPResult {
    std::vector<std::vector<int>> dist; // dist[u][v]
    std::vector<std::vector<int>> next; // next[u][v] = first hop from u toward v
    bool hasNegCycle = false;
};

// -----------------------------------------------------------------------------
//  ShortestPath - three algorithms for supply route optimisation
//    * Dijkstra      - non-negative weights, O((V+E) log V)
//    * Bellman-Ford  - handles negative weights, detects negative cycles
//    * Floyd-Warshall - all-pairs O(V^3)
// -----------------------------------------------------------------------------
class ShortestPath {
public:
    static SPResult   dijkstra(const Graph& g, int src);
    static SPResult   bellmanFord(const Graph& g, int src);
    static APSPResult floydWarshall(const Graph& g);

    // Reconstruct path from prev[] array
    static std::vector<int> reconstructPath(const std::vector<int>& prev,
                                            int src, int dst);
    // Pretty-print single-source results
    static void printSP(const SPResult& res, const Graph& g,
                        int src, int dst = -1);
    // Pretty-print all-pairs table
    static void printAPSP(const APSPResult& res, const Graph& g);
    // Complexity stats block
    static void printComplexity(const std::string& algo, int V, int E, long long ms);
};
