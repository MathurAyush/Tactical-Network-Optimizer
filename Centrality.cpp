#include "Centrality.h"
#include "ShortestPath.h"
#include "Colors.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <cmath>

// -----------------------------------------------------------------------------
//  Centrality::analyze
//  All-pairs via Dijkstra from each source — O(V * (V+E) log V)
// -----------------------------------------------------------------------------

CentralityResult Centrality::analyze(const Graph& g) {
    int n = g.size();
    CentralityResult res;
    res.degree.assign(n, 0.0);
    res.betweenness.assign(n, 0.0);
    res.closeness.assign(n, 0.0);
    res.composite.assign(n, 0.0);

    if (n <= 1) return res;

    // -- Degree centrality (normalised) --
    for (int u = 0; u < n; u++) {
        // Count unique neighbours (multi-edges collapsed)
        int deg = static_cast<int>(g.adj[u].size());
        res.degree[u] = static_cast<double>(deg) / (n - 1);
        if (res.degree[u] > 1.0) res.degree[u] = 1.0; // cap for multigraphs
    }

    // -- All-pairs shortest paths using Dijkstra from every source --
    // sp[s][t] = shortest distance from s to t (INF if unreachable)
    std::vector<std::vector<int>> sp(n);
    // prev[s][t] = predecessor of t on shortest path from s
    std::vector<std::vector<int>> prev(n);

    for (int s = 0; s < n; s++) {
        SPResult r = ShortestPath::dijkstra(g, s);
        sp[s]   = r.dist;
        prev[s] = r.prev;
    }

    // -- Betweenness centrality --
    // For each ordered pair (s,t), find the shortest path and give credit
    // to all intermediate nodes. We use one shortest path per pair (the one
    // Dijkstra finds). Normalisation: divide by (n-1)*(n-2) for directed,
    // (n-1)*(n-2)/2 for undirected.
    std::vector<double> bet(n, 0.0);
    for (int s = 0; s < n; s++) {
        for (int t = 0; t < n; t++) {
            if (s == t) continue;
            if (sp[s][t] >= INF) continue;
            // Walk the path and credit intermediates
            auto path = ShortestPath::reconstructPath(prev[s], s, t);
            for (size_t i = 1; i + 1 < path.size(); i++)
                bet[path[i]] += 1.0;
        }
    }
    double betNorm = g.directed
                   ? static_cast<double>((n - 1) * (n - 2))
                   : static_cast<double>((n - 1) * (n - 2)) / 2.0;
    for (int u = 0; u < n; u++)
        res.betweenness[u] = (betNorm > 0) ? bet[u] / betNorm : 0.0;

    // -- Closeness centrality --
    // C(u) = (reachable-1) / sum(dist(u,v) for reachable v)
    // Normalised variant: multiply by (reachable-1)/(n-1) to handle disconnected graphs
    for (int u = 0; u < n; u++) {
        double sumDist   = 0.0;
        int    reachable = 0;
        for (int v = 0; v < n; v++) {
            if (v == u) continue;
            if (sp[u][v] < INF) {
                sumDist += sp[u][v];
                reachable++;
            }
        }
        if (sumDist > 0 && reachable > 0) {
            double raw = static_cast<double>(reachable) / sumDist;
            // Wasserman-Faust normalisation for disconnected graphs
            res.closeness[u] = raw * static_cast<double>(reachable) / (n - 1);
        }
    }

    // -- Composite strategic importance score --
    for (int u = 0; u < n; u++)
        res.composite[u] = (res.degree[u] + res.betweenness[u] + res.closeness[u]) / 3.0;

    return res;
}

// -----------------------------------------------------------------------------
//  Print
// -----------------------------------------------------------------------------

void Centrality::printResult(const CentralityResult& res, const Graph& g) {
    int n = g.size();

    std::cout << "\n  " << CLR_BORDER << "+" << std::string(72, '=') << "+" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|  " << CLR_RESULT
              << std::left << std::setw(70)
              << "NETWORK CENTRALITY ANALYSIS  -  Strategic Base Importance"
              << CLR_BORDER << "|" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "+" << std::string(72, '=') << "+" << CLR_RESET << "\n";

    // Header
    std::cout << "  " << CLR_BORDER << "| "
              << CLR_RESULT << std::left
              << std::setw(4)  << "Rank"
              << std::setw(24) << "Base"
              << std::setw(10) << "Degree"
              << std::setw(14) << "Betweenness"
              << std::setw(12) << "Closeness"
              << std::setw(10) << "Composite"
              << CLR_BORDER << "|" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "+" << std::string(72, '-') << "+" << CLR_RESET << "\n";

    // Sort by composite score descending
    std::vector<int> order(n);
    std::iota(order.begin(), order.end(), 0);
    std::sort(order.begin(), order.end(), [&](int a, int b){
        return res.composite[a] > res.composite[b];
    });

    for (int rank = 0; rank < n; rank++) {
        int u = order[rank];

        // Highlight top 3
        const char* col = (rank == 0) ? CLR_WARN :
                          (rank  < 3) ? CLR_OPTION : CLR_RESULT;

        std::string name = g.getName(u);
        if ((int)name.size() > 22) name = name.substr(0, 22);

        std::cout << "  " << CLR_BORDER << "| " << col
                  << std::left
                  << std::setw(4)  << ("#" + std::to_string(rank + 1))
                  << std::setw(24) << name
                  << std::fixed << std::setprecision(3)
                  << std::setw(10) << res.degree[u]
                  << std::setw(14) << res.betweenness[u]
                  << std::setw(12) << res.closeness[u]
                  << std::setw(10) << res.composite[u]
                  << CLR_BORDER << "|" << CLR_RESET << "\n";
    }
    std::cout << "  " << CLR_BORDER << "+" << std::string(72, '=') << "+" << CLR_RESET << "\n";

    // Tactical annotation for top base
    if (n > 0) {
        int top = order[0];
        std::cout << "\n  " << CLR_WARN
                  << "  HIGHEST PRIORITY BASE: [" << top << "] " << g.getName(top) << "\n"
                  << "  This base has the highest composite strategic importance.\n"
                  << "  It is a key relay/chokepoint — prioritise for defence and redundancy."
                  << CLR_RESET << "\n";
    }

    // Betweenness legend
    std::cout << "\n  " << CLR_RESULT
              << "  Metrics (all normalised 0-1):\n"
              << "    Degree      - fraction of all bases directly connected\n"
              << "    Betweenness - fraction of shortest paths routed through this base\n"
              << "    Closeness   - ability to quickly reach or supply all other bases\n"
              << "    Composite   - equal-weight average of the three"
              << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << std::string(74, '-') << CLR_RESET << "\n";
}

void Centrality::printComplexity(int V, int E, long long ms) {
    std::cout << "\n  " << CLR_BORDER << std::string(54, '-') << CLR_RESET << "\n";
    std::cout << "  " << CLR_RESULT << "  Complexity [Centrality Analysis]" << CLR_RESET << "\n";
    std::cout << "  " << CLR_RESULT
              << "    Time  : O(V*(V+E) log V)  [Dijkstra from each source]"
              << CLR_RESET << "\n";
    std::cout << "  " << CLR_RESULT
              << "    Space : O(V^2)  [all-pairs distance table]"
              << CLR_RESET << "\n";
    std::cout << "  " << CLR_RESULT
              << "    Input : V=" << V << "  E=" << E
              << "   Elapsed: " << ms << " ms"
              << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << std::string(54, '-') << CLR_RESET << "\n";
}
