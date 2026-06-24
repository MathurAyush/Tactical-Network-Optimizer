#include "Resilience.h"
#include "Colors.h"
#include "RiskUtil.h"
#include "MaxFlow.h"
#include <queue>
#include <iostream>
#include <iomanip>
#include <algorithm>

// -----------------------------------------------------------------------------
//  BFS reachability, optionally treating the edge (u <-> v) as removed
// -----------------------------------------------------------------------------

std::vector<int> Resilience::reachableFrom(const Graph& g, int src, int u, int v) {
    int n = g.size();
    std::vector<bool> visited(n, false);
    std::vector<int>  order;
    if (n == 0) return order;

    std::queue<int> q;
    visited[src] = true;
    q.push(src);
    while (!q.empty()) {
        int x = q.front(); q.pop();
        order.push_back(x);
        for (const auto& e : g.adj[x]) {
            int y = e.to;
            if ((x == u && y == v) || (x == v && y == u)) continue; // pretend cut
            if (!visited[y]) { visited[y] = true; q.push(y); }
        }
    }
    return order;
}

// -----------------------------------------------------------------------------
//  Bridge finding - classic DFS low-link algorithm (Tarjan)
// -----------------------------------------------------------------------------

void Resilience::bridgeDFS(const Graph& g, int u, int parent,
                           std::vector<int>& disc, std::vector<int>& low,
                           int& timer, std::vector<Edge>& bridges) {
    disc[u] = low[u] = ++timer;
    bool skippedParentEdge = false;

    for (const auto& e : g.adj[u]) {
        int v = e.to;
        if (v == parent && !skippedParentEdge) {
            skippedParentEdge = true; // skip exactly one copy of the parent edge
            continue;
        }
        if (disc[v] == 0) {
            bridgeDFS(g, v, u, disc, low, timer, bridges);
            low[u] = std::min(low[u], low[v]);
            if (low[v] > disc[u])
                bridges.push_back({u, v, e.weight, e.risk});
        } else {
            low[u] = std::min(low[u], disc[v]);
        }
    }
}

std::vector<Edge> Resilience::findBridges(const Graph& g) {
    int n = g.size();
    std::vector<int> disc(n, 0), low(n, 0);
    std::vector<Edge> bridges;
    int timer = 0;

    for (int i = 0; i < n; i++)
        if (disc[i] == 0)
            bridgeDFS(g, i, -1, disc, low, timer, bridges);

    return bridges;
}

// -----------------------------------------------------------------------------
//  Global minimum cut - for an undirected network, the global min cut always
//  separates base 0 from some other base, so min over t of maxflow(0, t)
//  yields the network's weakest point. The cut edge-set is recovered from the
//  residual capacities (origCap - flow) via a BFS reachability pass from 0.
// -----------------------------------------------------------------------------

std::pair<int, std::vector<Edge>> Resilience::globalMinCut(const Graph& g) {
    int n = g.size();
    if (n < 2) return {0, {}};

    // Original capacity matrix (mirrors MaxFlow::edmondsKarp's construction)
    std::vector<std::vector<int>> origCap(n, std::vector<int>(n, 0));
    for (const Edge& e : g.edgeList) {
        origCap[e.from][e.to] += e.weight;
        if (!g.directed)
            origCap[e.to][e.from] += e.weight;
    }

    int bestVal = INF;
    std::vector<Edge> bestCut;

    for (int t = 1; t < n; t++) {
        FlowResult fr = MaxFlow::edmondsKarp(g, 0, t);
        if (fr.maxFlow >= bestVal) continue;

        // Residual capacity = original capacity - net flow
        std::vector<std::vector<int>> resid(n, std::vector<int>(n, 0));
        for (int u = 0; u < n; u++)
            for (int v = 0; v < n; v++)
                resid[u][v] = origCap[u][v] - fr.flow[u][v];

        // Reachable set S from base 0 in the residual graph
        std::vector<bool> inS(n, false);
        std::queue<int> q;
        inS[0] = true;
        q.push(0);
        while (!q.empty()) {
            int u = q.front(); q.pop();
            for (int v = 0; v < n; v++)
                if (!inS[v] && resid[u][v] > 0) { inS[v] = true; q.push(v); }
        }

        std::vector<Edge> cutEdges;
        for (const Edge& e : g.edgeList)
            if (inS[e.from] != inS[e.to])
                cutEdges.push_back(e);

        bestVal = fr.maxFlow;
        bestCut = cutEdges;
    }

    if (bestVal == INF) return {0, {}};
    return {bestVal, bestCut};
}

// -----------------------------------------------------------------------------
//  Full report
// -----------------------------------------------------------------------------

ResilienceReport Resilience::analyze(const Graph& g) {
    ResilienceReport rep;
    auto bridges = findBridges(g);

    for (const Edge& b : bridges) {
        BridgeImpact bi;
        bi.edge = b;
        auto reach = reachableFrom(g, 0, b.from, b.to);
        std::vector<bool> ok(g.size(), false);
        for (int v : reach) ok[v] = true;
        for (int v = 0; v < g.size(); v++)
            if (!ok[v]) bi.strandedBases.push_back(v);
        rep.bridges.push_back(bi);
    }

    auto cut = globalMinCut(g);
    rep.minCutValue = cut.first;
    rep.minCutEdges = cut.second;
    return rep;
}

// -----------------------------------------------------------------------------
//  Output
// -----------------------------------------------------------------------------

static void printColoredEdge(const Graph& g, const Edge& e) {
    const char* rc = RiskUtil::color(e.risk);
    std::cout << "[" << CLR_BASE << g.getName(e.from) << CLR_RESET << "] "
              << rc << "---(d:" << e.weight << " r:" << e.risk << " "
              << RiskUtil::label(e.risk) << ")--->" << CLR_RESET
              << " [" << CLR_BASE << g.getName(e.to) << CLR_RESET << "]";
}

void Resilience::printAttackResult(const Graph& g, int edgeIdx, int referenceBase) {
    if (edgeIdx < 0 || edgeIdx >= static_cast<int>(g.edgeList.size())) {
        std::cout << "  " << CLR_WARN << "Invalid route index." << CLR_RESET << "\n";
        return;
    }
    const Edge& e = g.edgeList[edgeIdx];

    std::cout << "\n  " << CLR_RESULT << "[Simulated Attack - Route Severed]" << CLR_RESET << "\n";
    std::cout << "  Target route: ";
    printColoredEdge(g, e);
    std::cout << "\n  " << std::string(60, '-') << "\n";

    auto reach = reachableFrom(g, referenceBase, e.from, e.to);
    std::vector<bool> ok(g.size(), false);
    for (int v : reach) ok[v] = true;

    std::cout << "  Connectivity recheck (BFS from " << CLR_BASE << g.getName(referenceBase)
              << CLR_RESET << "):\n";

    bool anyLost = false;
    for (int v = 0; v < g.size(); v++) {
        if (!ok[v]) {
            anyLost = true;
            std::cout << "    " << CLR_WARN << "[UNREACHABLE] [" << v << "] "
                      << g.getName(v) << CLR_RESET << "\n";
        }
    }
    if (!anyLost)
        std::cout << "    " << CLR_BORDER << "Network remains fully connected - route was redundant."
                  << CLR_RESET << "\n";
}

void Resilience::printReport(const ResilienceReport& rep, const Graph& g, int referenceBase) {
    std::cout << "\n  " << CLR_RESULT << "[Network Resilience Report]" << CLR_RESET << "\n";
    RiskUtil::printLegend();
    std::cout << "  " << std::string(60, '-') << "\n";

    std::cout << "  " << CLR_RESULT << "Critical Edges (Bridges) - "
              << rep.bridges.size() << " found" << CLR_RESET << "\n";
    if (rep.bridges.empty()) {
        std::cout << "    " << CLR_BORDER << "None - the network has no single point of failure."
                  << CLR_RESET << "\n";
    } else {
        for (const auto& bi : rep.bridges) {
            std::cout << "    ";
            printColoredEdge(g, bi.edge);
            std::cout << "\n      -> if cut, unreachable from " << g.getName(referenceBase) << ": ";
            if (bi.strandedBases.empty()) {
                std::cout << CLR_BORDER << "(none, alternate routes exist)" << CLR_RESET;
            } else {
                for (size_t i = 0; i < bi.strandedBases.size(); i++) {
                    if (i) std::cout << ", ";
                    std::cout << CLR_WARN << g.getName(bi.strandedBases[i]) << CLR_RESET;
                }
            }
            std::cout << "\n";
        }
    }

    std::cout << "  " << std::string(60, '-') << "\n";
    std::cout << "  " << CLR_RESULT << "Min Cut (weakest point in the network)" << CLR_RESET << "\n";
    std::cout << "    Cut value (min total capacity to sever): "
              << CLR_WARN << rep.minCutValue << CLR_RESET << "\n";
    std::cout << "    Edges forming the cut:\n";
    for (const Edge& e : rep.minCutEdges) {
        std::cout << "      " << CLR_WARN << "[CUT] " << CLR_RESET;
        printColoredEdge(g, e);
        std::cout << "\n";
    }
}
