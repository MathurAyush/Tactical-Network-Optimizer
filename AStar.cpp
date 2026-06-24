#include "AStar.h"
#include "ShortestPath.h"
#include "Colors.h"
#include "RiskUtil.h"
#include <queue>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <algorithm>

// -----------------------------------------------------------------------------
//  A* Search
// -----------------------------------------------------------------------------

AStarResult AStar::search(const Graph& g, int src, int dst) {
    int n = g.size();
    AStarResult res;
    res.dist.assign(n, INF);
    res.prev.assign(n, -1);

    bool useH = g.hasCoords();

    auto heuristic = [&](int v) -> double {
        if (!useH) return 0.0;
        double dx = g.getX(v) - g.getX(dst);
        double dy = g.getY(v) - g.getY(dst);
        return std::sqrt(dx * dx + dy * dy);
    };

    // f[v] = g_cost[v] + h(v); stored as double to match heuristic precision
    std::vector<double> f(n, static_cast<double>(INF));

    using pdi = std::pair<double, int>;
    std::priority_queue<pdi, std::vector<pdi>, std::greater<pdi>> open;

    res.dist[src] = 0;
    f[src] = heuristic(src);
    open.push({f[src], src});

    while (!open.empty()) {
        auto [fval, u] = open.top();
        open.pop();

        if (fval > f[u] + 1e-9) continue; // stale entry

        res.nodesExpanded++;

        if (u == dst) {
            res.found = true;
            break;
        }

        for (const auto& e : g.adj[u]) {
            int v      = e.to;
            int newG   = res.dist[u] + e.weight;
            if (newG < res.dist[v]) {
                res.dist[v] = newG;
                res.prev[v] = u;
                f[v]        = newG + heuristic(v);
                open.push({f[v], v});
            }
        }
    }

    if (!res.found && res.dist[dst] < INF)
        res.found = true;

    return res;
}

// -----------------------------------------------------------------------------
//  Path reconstruction
// -----------------------------------------------------------------------------

std::vector<int> AStar::reconstructPath(const std::vector<int>& prev,
                                        int src, int dst) {
    std::vector<int> path;
    if (prev[dst] == -1 && dst != src) return path;
    for (int v = dst; v != -1; v = prev[v])
        path.push_back(v);
    std::reverse(path.begin(), path.end());
    return path;
}

// -----------------------------------------------------------------------------
//  Print result
// -----------------------------------------------------------------------------

void AStar::printResult(const AStarResult& res, const Graph& g,
                        int src, int dst) {
    std::cout << "\n  " << CLR_RESULT
              << "[A* Search: " << g.getName(src) << " -> " << g.getName(dst) << "]"
              << CLR_RESET << "\n";

    if (!res.found || res.dist[dst] >= INF) {
        std::cout << "  " << CLR_WARN << "  No path found." << CLR_RESET << "\n";
        return;
    }

    auto path = reconstructPath(res.prev, src, dst);

    std::cout << "  " << CLR_BORDER << std::string(60, '-') << CLR_RESET << "\n";
    std::cout << "  " << CLR_RESULT << "  Distance      : " << res.dist[dst] << CLR_RESET << "\n";
    std::cout << "  " << CLR_RESULT << "  Risk score    : " << RiskUtil::pathRisk(g, path) << CLR_RESET << "\n";
    std::cout << "  " << CLR_RESULT << "  Nodes expanded: " << res.nodesExpanded << CLR_RESET << "\n";
    std::cout << "  " << CLR_RESULT << "  Heuristic     : "
              << (g.hasCoords() ? "Euclidean distance (admissible)" : "h=0 (no coords, equivalent to Dijkstra)")
              << CLR_RESET << "\n";
    std::cout << "  " << CLR_RESULT << "  Path          : ";
    RiskUtil::printColoredPath(g, path);
    std::cout << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << std::string(60, '-') << CLR_RESET << "\n";
}

// -----------------------------------------------------------------------------
//  Side-by-side comparison with Dijkstra
// -----------------------------------------------------------------------------

void AStar::compareWithDijkstra(const Graph& g, int src, int dst) {
    std::cout << "\n  " << CLR_RESULT
              << "[A* vs Dijkstra: " << g.getName(src) << " -> " << g.getName(dst) << "]"
              << CLR_RESET << "\n";
    RiskUtil::printLegend();
    std::cout << "  " << CLR_BORDER << std::string(64, '=') << CLR_RESET << "\n";

    // --- Dijkstra (track expansions via a wrapper) ---
    int dijkExpanded = 0;
    {
        int n = g.size();
        std::vector<int> dist(n, INF), prev(n, -1);
        using pii = std::pair<int,int>;
        std::priority_queue<pii, std::vector<pii>, std::greater<pii>> pq;
        dist[src] = 0;
        pq.push({0, src});
        while (!pq.empty()) {
            auto [d, u] = pq.top(); pq.pop();
            if (d > dist[u]) continue;
            dijkExpanded++;
            for (const auto& e : g.adj[u]) {
                if (dist[u] + e.weight < dist[e.to]) {
                    dist[e.to] = dist[u] + e.weight;
                    prev[e.to] = u;
                    pq.push({dist[e.to], e.to});
                }
            }
        }
        auto path = ShortestPath::reconstructPath(prev, src, dst);

        std::cout << "\n  " << CLR_OPTION << "DIJKSTRA" << CLR_RESET << "\n";
        std::cout << "  " << CLR_RESULT  << "  Nodes expanded : " << dijkExpanded << CLR_RESET << "\n";
        if (dist[dst] < INF) {
            std::cout << "  " << CLR_RESULT << "  Distance       : " << dist[dst] << CLR_RESET << "\n";
            std::cout << "  " << CLR_RESULT << "  Path           : ";
            RiskUtil::printColoredPath(g, path);
            std::cout << CLR_RESET << "\n";
        } else {
            std::cout << "  " << CLR_WARN << "  No path found." << CLR_RESET << "\n";
        }
    }

    // --- A* ---
    AStarResult ar = AStar::search(g, src, dst);
    auto path      = AStar::reconstructPath(ar.prev, src, dst);

    std::cout << "\n  " << CLR_OPTION << "A* SEARCH"
              << (g.hasCoords() ? " (Euclidean heuristic)" : " (h=0, no coords)")
              << CLR_RESET << "\n";
    std::cout << "  " << CLR_RESULT << "  Nodes expanded : " << ar.nodesExpanded << CLR_RESET << "\n";
    if (ar.found) {
        std::cout << "  " << CLR_RESULT << "  Distance       : " << ar.dist[dst] << CLR_RESET << "\n";
        std::cout << "  " << CLR_RESULT << "  Path           : ";
        RiskUtil::printColoredPath(g, path);
        std::cout << CLR_RESET << "\n";
    } else {
        std::cout << "  " << CLR_WARN << "  No path found." << CLR_RESET << "\n";
    }

    // --- Summary ---
    std::cout << "\n  " << CLR_BORDER << std::string(64, '-') << CLR_RESET << "\n";
    if (dijkExpanded > 0) {
        int saved = dijkExpanded - ar.nodesExpanded;
        double pct = 100.0 * saved / dijkExpanded;
        std::cout << "  " << CLR_RESULT
                  << "  A* expanded " << ar.nodesExpanded << " nodes vs Dijkstra's "
                  << dijkExpanded << "  =>  "
                  << std::fixed << std::setprecision(1) << pct
                  << "% fewer expansions"
                  << CLR_RESET << "\n";
    }
    std::cout << "  " << CLR_BORDER << std::string(64, '=') << CLR_RESET << "\n";
}

// -----------------------------------------------------------------------------
//  Complexity block
// -----------------------------------------------------------------------------

void AStar::printComplexity(int V, int E, int expanded, long long ms) {
    std::cout << "\n  " << CLR_BORDER << std::string(54, '-') << CLR_RESET << "\n";
    std::cout << "  " << CLR_RESULT << "  Complexity [A*]" << CLR_RESET << "\n";
    std::cout << "  " << CLR_RESULT
              << "    Worst-case : O((V+E) log V)  (same as Dijkstra)" << CLR_RESET << "\n";
    std::cout << "  " << CLR_RESULT
              << "    Typical    : far fewer expansions with good heuristic" << CLR_RESET << "\n";
    std::cout << "  " << CLR_RESULT
              << "    Input      : V=" << V << "  E=" << E
              << "   Expanded: " << expanded << "   Elapsed: " << ms << " ms"
              << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << std::string(54, '-') << CLR_RESET << "\n";
}
