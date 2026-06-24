#pragma once
#include "Graph.h"
#include <vector>
#include <string>

// Result for A* single-pair search
struct AStarResult {
    std::vector<int> dist;    // dist[v] = best g-cost from src (INF if not reached)
    std::vector<int> prev;    // predecessor array for path reconstruction
    int  nodesExpanded = 0;   // how many nodes were popped from the open-set
    bool found         = false;
};

// -----------------------------------------------------------------------------
//  AStar - heuristic shortest path search
//    Uses Euclidean distance between node coordinates as the admissible heuristic.
//    When no coordinates are set (all zero), h=0 and the search degrades to
//    Dijkstra's algorithm — still correct, but expands more nodes.
//    Complexity: O((V + E) log V) worst-case, typically much better with good h.
// -----------------------------------------------------------------------------
class AStar {
public:
    static AStarResult search(const Graph& g, int src, int dst);

    // Reconstruct path from prev[] array
    static std::vector<int> reconstructPath(const std::vector<int>& prev,
                                            int src, int dst);

    // Run both A* and Dijkstra on the same query, print side-by-side comparison
    static void compareWithDijkstra(const Graph& g, int src, int dst);

    static void printResult(const AStarResult& res, const Graph& g,
                            int src, int dst);
    static void printComplexity(int V, int E, int expanded, long long ms);
};
