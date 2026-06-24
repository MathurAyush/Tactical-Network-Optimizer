#pragma once
#include "Graph.h"
#include <vector>
#include <string>

struct CentralityResult {
    std::vector<double> degree;       // normalised degree centrality  [0,1]
    std::vector<double> betweenness;  // normalised betweenness centrality [0,1]
    std::vector<double> closeness;    // closeness centrality [0,1]
    std::vector<double> composite;    // equal-weight average of the three
};

// -----------------------------------------------------------------------------
//  Centrality - identifies the most strategically critical bases
//
//    Degree centrality    : fraction of other bases directly connected
//    Betweenness centrality: fraction of all inter-base shortest paths
//                            that pass through this node — a high score
//                            means the base is a key relay / chokepoint
//    Closeness centrality : inverse of average distance to all other bases —
//                           high score means rapid reinforcement capability
//    Composite score      : equal-weight average, used for final ranking
//
//  Algorithm: Dijkstra from each source for all-pairs shortest paths O(V*(V+E)logV)
// -----------------------------------------------------------------------------
class Centrality {
public:
    static CentralityResult analyze(const Graph& g);
    static void printResult(const CentralityResult& res, const Graph& g);
    static void printComplexity(int V, int E, long long ms);
};
