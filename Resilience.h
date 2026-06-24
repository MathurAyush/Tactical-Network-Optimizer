#pragma once
#include "Graph.h"
#include <vector>

// -----------------------------------------------------------------------------
//  Resilience - Network Resilience Analyzer
//    * simulateAttack - removes one route and recomputes connectivity (BFS)
//    * findBridges    - critical edges (bridges) whose removal disconnects
//                       part of the network, plus which bases get cut off
//    * globalMinCut   - the network's weakest point (minimum edge cut), found
//                       by reusing Edmonds-Karp max-flow from base 0 to every
//                       other base (the classic trick for undirected min-cut)
// -----------------------------------------------------------------------------

// One bridge plus the set of bases it strands when removed
struct BridgeImpact {
    Edge edge;
    std::vector<int> strandedBases; // bases unreachable from base 0 once cut
};

struct ResilienceReport {
    std::vector<BridgeImpact> bridges;
    int                minCutValue = 0;
    std::vector<Edge>  minCutEdges;
};

class Resilience {
public:
    // BFS reachable set from 'src', optionally pretending the edge (u,v) is gone
    static std::vector<int> reachableFrom(const Graph& g, int src, int u = -1, int v = -1);

    // All critical edges (bridges) in an undirected network
    static std::vector<Edge> findBridges(const Graph& g);

    // Minimum edge cut of the whole network (weakest point)
    static std::pair<int, std::vector<Edge>> globalMinCut(const Graph& g);

    // Full resilience report: bridges + their impact + min cut
    static ResilienceReport analyze(const Graph& g);

    // -- Output ---------------------------------------------------------------
    static void printAttackResult(const Graph& g, int edgeIdx, int referenceBase);
    static void printReport(const ResilienceReport& rep, const Graph& g, int referenceBase);

private:
    static void bridgeDFS(const Graph& g, int u, int parent,
                          std::vector<int>& disc, std::vector<int>& low,
                          int& timer, std::vector<Edge>& bridges);
};
