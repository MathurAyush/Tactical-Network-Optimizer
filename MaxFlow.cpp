#include "MaxFlow.h"
#include "Colors.h"
#include "RiskUtil.h"
#include <queue>
#include <algorithm>
#include <iostream>
#include <iomanip>

// -----------------------------------------------------------------------------
//  BFS on the residual graph to find an augmenting path
// -----------------------------------------------------------------------------

bool MaxFlow::bfsAugment(const std::vector<std::vector<int>>& cap,
                          int src, int sink, int n,
                          std::vector<int>& parent) {
    std::vector<bool> visited(n, false);
    std::queue<int> q;
    visited[src] = true;
    parent[src]  = -1;
    q.push(src);

    while (!q.empty()) {
        int u = q.front(); q.pop();
        for (int v = 0; v < n; v++) {
            if (!visited[v] && cap[u][v] > 0) {
                visited[v] = true;
                parent[v]  = u;
                if (v == sink) return true;
                q.push(v);
            }
        }
    }
    return false;
}

// -----------------------------------------------------------------------------
//  Edmonds-Karp (Ford-Fulkerson with BFS augmenting paths)
// -----------------------------------------------------------------------------

FlowResult MaxFlow::edmondsKarp(const Graph& g, int source, int sink) {
    int n = g.size();
    FlowResult res;
    res.maxFlow = 0;
    res.flow.assign(n, std::vector<int>(n, 0));

    std::vector<std::vector<int>> cap(n, std::vector<int>(n, 0));
    for (const Edge& e : g.edgeList) {
        cap[e.from][e.to] += e.weight;
        if (!g.directed)
            cap[e.to][e.from] += e.weight;
    }

    std::vector<int> parent(n);
    while (bfsAugment(cap, source, sink, n, parent)) {
        int pathFlow = INT_MAX;
        for (int v = sink; v != source; v = parent[v]) {
            int u = parent[v];
            pathFlow = std::min(pathFlow, cap[u][v]);
        }
        for (int v = sink; v != source; v = parent[v]) {
            int u = parent[v];
            cap[u][v] -= pathFlow;
            cap[v][u] += pathFlow;
            res.flow[u][v] += pathFlow;
            res.flow[v][u] -= pathFlow;
        }
        res.maxFlow += pathFlow;
    }
    return res;
}

// -----------------------------------------------------------------------------
//  Output
// -----------------------------------------------------------------------------

void MaxFlow::printResult(const FlowResult& res, const Graph& g,
                           int source, int sink) {
    int n = g.size();
    std::cout << "\n  " << CLR_RESULT << "[Edmonds-Karp Max Flow Result]" << CLR_RESET << "\n";
    RiskUtil::printLegend();
    std::cout << "  " << CLR_BORDER << std::string(62, '-') << CLR_RESET << "\n";
    std::cout << "  " << CLR_RESULT
              << "  Source : [" << source << "] " << g.getName(source) << CLR_RESET << "\n";
    std::cout << "  " << CLR_RESULT
              << "  Sink   : [" << sink   << "] " << g.getName(sink)   << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << std::string(62, '-') << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << std::left
              << "  " << std::setw(6) << "From"
              << std::setw(22) << "Base"
              << "     "
              << std::setw(6) << "To"
              << std::setw(22) << "Base"
              << "Flow" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << std::string(62, '-') << CLR_RESET << "\n";

    bool anyFlow = false;
    for (int u = 0; u < n; u++) {
        for (int v = 0; v < n; v++) {
            if (res.flow[u][v] > 0) {
                anyFlow = true;
                // Risk colour on the arc
                int risk = g.getRisk(u, v);
                if (risk < 0) risk = 0;
                const char* rc = RiskUtil::color(risk);
                std::cout << "  " << CLR_RESULT
                          << "  [" << std::right << std::setw(2) << u << "] "
                          << std::left << std::setw(20) << g.getName(u) << std::right
                          << rc << "  -->  " << CLR_RESET
                          << "[" << std::right << std::setw(2) << v << "] "
                          << std::left << std::setw(20) << g.getName(v) << std::right
                          << CLR_RESULT << "  flow: " << std::setw(4) << res.flow[u][v]
                          << CLR_RESET << "\n";
            }
        }
    }

    if (!anyFlow)
        std::cout << "  " << CLR_WARN << "  No flow between source and sink." << CLR_RESET << "\n";

    std::cout << "  " << CLR_BORDER << std::string(62, '-') << CLR_RESET << "\n";
    std::cout << "  " << CLR_RESULT
              << "  Maximum flow (supply throughput): " << res.maxFlow << CLR_RESET << "\n";
}

// -----------------------------------------------------------------------------
//  Complexity stats block
// -----------------------------------------------------------------------------

void MaxFlow::printComplexity(int V, int E, long long ms) {
    std::cout << "\n  " << CLR_BORDER << std::string(54, '-') << CLR_RESET << "\n";
    std::cout << "  " << CLR_RESULT << "  Complexity [Edmonds-Karp Max Flow]" << CLR_RESET << "\n";
    std::cout << "  " << CLR_RESULT
              << "    Time  : O(V * E^2)     Space : O(V^2)" << CLR_RESET << "\n";
    std::cout << "  " << CLR_RESULT
              << "    Input : V=" << V << " nodes, E=" << E << " edges"
              << "   Elapsed: " << ms << " ms" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << std::string(54, '-') << CLR_RESET << "\n";
}
