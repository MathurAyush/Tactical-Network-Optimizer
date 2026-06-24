#include "ShortestPath.h"
#include "Colors.h"
#include "RiskUtil.h"
#include <queue>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <sstream>

// -----------------------------------------------------------------------------
//  Dijkstra - O((V + E) log V)
// -----------------------------------------------------------------------------

SPResult ShortestPath::dijkstra(const Graph& g, int src) {
    int n = g.size();
    SPResult res;
    res.dist.assign(n, INF);
    res.prev.assign(n, -1);

    using pii = std::pair<int,int>;
    std::priority_queue<pii, std::vector<pii>, std::greater<pii>> pq;

    res.dist[src] = 0;
    pq.push({0, src});

    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (d > res.dist[u]) continue;

        for (const auto& e : g.adj[u]) {
            int v = e.to, w = e.weight;
            if (res.dist[u] + w < res.dist[v]) {
                res.dist[v] = res.dist[u] + w;
                res.prev[v] = u;
                pq.push({res.dist[v], v});
            }
        }
    }
    return res;
}

// -----------------------------------------------------------------------------
//  Bellman-Ford - O(V * E)
// -----------------------------------------------------------------------------

SPResult ShortestPath::bellmanFord(const Graph& g, int src) {
    int n = g.size();
    SPResult res;
    res.dist.assign(n, INF);
    res.prev.assign(n, -1);
    res.dist[src] = 0;

    for (int iter = 0; iter < n - 1; iter++) {
        bool updated = false;
        for (const Edge& e : g.edgeList) {
            if (res.dist[e.from] == INF) continue;
            if (res.dist[e.from] + e.weight < res.dist[e.to]) {
                res.dist[e.to] = res.dist[e.from] + e.weight;
                res.prev[e.to] = e.from;
                updated = true;
            }
            if (!g.directed) {
                if (res.dist[e.to] == INF) continue;
                if (res.dist[e.to] + e.weight < res.dist[e.from]) {
                    res.dist[e.from] = res.dist[e.to] + e.weight;
                    res.prev[e.from] = e.to;
                    updated = true;
                }
            }
        }
        if (!updated) break;
    }

    for (const Edge& e : g.edgeList) {
        if (res.dist[e.from] != INF &&
            res.dist[e.from] + e.weight < res.dist[e.to]) {
            res.hasNegCycle = true;
            break;
        }
    }
    return res;
}

// -----------------------------------------------------------------------------
//  Floyd-Warshall - all-pairs O(V^3)
// -----------------------------------------------------------------------------

APSPResult ShortestPath::floydWarshall(const Graph& g) {
    int n = g.size();
    APSPResult res;
    res.dist = g.getAdjMatrix();
    res.next.assign(n, std::vector<int>(n, -1));

    for (int u = 0; u < n; u++)
        for (int v = 0; v < n; v++)
            if (u != v && res.dist[u][v] < INF)
                res.next[u][v] = v;

    for (int k = 0; k < n; k++)
        for (int u = 0; u < n; u++)
            for (int v = 0; v < n; v++) {
                if (res.dist[u][k] == INF || res.dist[k][v] == INF) continue;
                if (res.dist[u][k] + res.dist[k][v] < res.dist[u][v]) {
                    res.dist[u][v] = res.dist[u][k] + res.dist[k][v];
                    res.next[u][v] = res.next[u][k];
                }
            }

    for (int u = 0; u < n; u++)
        if (res.dist[u][u] < 0) { res.hasNegCycle = true; break; }

    return res;
}

// -----------------------------------------------------------------------------
//  Path reconstruction
// -----------------------------------------------------------------------------

std::vector<int> ShortestPath::reconstructPath(const std::vector<int>& prev,
                                                int src, int dst) {
    std::vector<int> path;
    if (prev[dst] == -1 && dst != src) return path;

    for (int v = dst; v != -1; v = prev[v])
        path.push_back(v);
    std::reverse(path.begin(), path.end());
    return path;
}

// -----------------------------------------------------------------------------
//  Output helpers
// -----------------------------------------------------------------------------

void ShortestPath::printSP(const SPResult& res, const Graph& g,
                            int src, int dst) {
    int n = g.size();
    std::cout << "\n  " << CLR_RESULT
              << "  Source base : [" << src << "] " << g.getName(src) << CLR_RESET << "\n";
    RiskUtil::printLegend();
    std::cout << "  " << CLR_BORDER << std::string(62, '-') << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << std::left
              << "  " << std::setw(26) << "Destination"
              << std::setw(8) << "Dist"
              << "Path  (risk score per hop)" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << std::string(62, '-') << CLR_RESET << "\n";

    if (res.hasNegCycle) {
        std::cout << "  " << CLR_WARN << "  Negative cycle detected - distances unreliable." << CLR_RESET << "\n";
        std::cout << "  " << CLR_BORDER << std::string(62, '-') << CLR_RESET << "\n";
        return;
    }

    auto printOne = [&](int v) {
        std::cout << "  " << CLR_RESULT << "  [" << std::right << std::setw(2) << v << "] "
                  << std::left << std::setw(22) << g.getName(v) << std::right;
        if (res.dist[v] >= INF) {
            std::cout << "  UNREACHABLE" << CLR_RESET << "\n";
        } else {
            std::cout << std::setw(6) << res.dist[v] << "  ";
            auto path = reconstructPath(res.prev, src, v);
            RiskUtil::printColoredPath(g, path);
            std::cout << CLR_RESULT << "  (risk: " << RiskUtil::pathRisk(g, path) << ")"
                      << CLR_RESET << "\n";
        }
    };

    if (dst >= 0 && dst < n) {
        printOne(dst);
    } else {
        for (int v = 0; v < n; v++)
            if (v != src) printOne(v);
    }
    std::cout << "  " << CLR_BORDER << std::string(62, '-') << CLR_RESET << "\n";
}

void ShortestPath::printAPSP(const APSPResult& res, const Graph& g) {
    int n = g.size();
    std::cout << "\n  " << CLR_RESULT << "[Floyd-Warshall All-Pairs Shortest Path Matrix]" << CLR_RESET << "\n";
    std::cout << "  Rows = source, Columns = destination  (INF = no path)\n";
    std::cout << "  " << CLR_BORDER << std::string(6 + 7 * n, '-') << CLR_RESET << "\n";

    if (res.hasNegCycle) {
        std::cout << "  " << CLR_WARN << "  Negative cycle detected." << CLR_RESET << "\n";
        return;
    }

    std::cout << "  " << std::setw(6) << " ";
    for (int v = 0; v < n; v++)
        std::cout << std::setw(7) << ("[" + std::to_string(v) + "]");
    std::cout << "\n  " << CLR_BORDER << std::string(6 + 7 * n, '-') << CLR_RESET << "\n";

    for (int u = 0; u < n; u++) {
        std::cout << "  " << CLR_RESULT << "[" << std::setw(2) << u << "] ";
        for (int v = 0; v < n; v++) {
            if (res.dist[u][v] >= INF)
                std::cout << std::setw(7) << "INF";
            else
                std::cout << std::setw(7) << res.dist[u][v];
        }
        std::cout << "   " << g.getName(u) << CLR_RESET << "\n";
    }
    std::cout << "  " << CLR_BORDER << std::string(6 + 7 * n, '-') << CLR_RESET << "\n";
}

// -----------------------------------------------------------------------------
//  Complexity stats block
// -----------------------------------------------------------------------------

void ShortestPath::printComplexity(const std::string& algo, int V, int E, long long ms) {
    std::string timeO, spaceO;
    if      (algo == "Dijkstra")       { timeO = "O((V+E) log V)"; spaceO = "O(V)  "; }
    else if (algo == "Bellman-Ford")   { timeO = "O(V * E)      "; spaceO = "O(V)  "; }
    else                               { timeO = "O(V^3)        "; spaceO = "O(V^2)"; }

    std::cout << "\n  " << CLR_BORDER << std::string(54, '-') << CLR_RESET << "\n";
    std::cout << "  " << CLR_RESULT << "  Complexity [" << algo << "]" << CLR_RESET << "\n";
    std::cout << "  " << CLR_RESULT
              << "    Time  : " << timeO
              << "   Space : " << spaceO << CLR_RESET << "\n";
    std::cout << "  " << CLR_RESULT
              << "    Input : V=" << V << " nodes, E=" << E << " edges"
              << "   Elapsed: " << ms << " ms" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << std::string(54, '-') << CLR_RESET << "\n";
}
