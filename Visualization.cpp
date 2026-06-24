#include "Visualization.h"
#include "Colors.h"
#include "RiskUtil.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

// -----------------------------------------------------------------------------
//  Banner
// -----------------------------------------------------------------------------

void Visualization::printBanner() {
    std::cout << CLR_BORDER << R"(
  +==============================================================+
  |      MILITARY LOGISTICS & TACTICAL NETWORK OPTIMIZER        |
  |                  Graph Algorithm Suite                       |
  |         C++ Implementation  |  Strategic Command            |
  +==============================================================+
)" << CLR_RESET;
}

// -----------------------------------------------------------------------------
//  Network - formatted adjacency list
// -----------------------------------------------------------------------------

void Visualization::printNetwork(const Graph& g) {
    if (g.empty()) { std::cout << "  (no network loaded)\n"; return; }

    std::cout << "\n  " << CLR_BORDER << "TACTICAL NETWORK - Adjacency List" << CLR_RESET << "\n";
    RiskUtil::printLegend();
    std::cout << "  " << CLR_BORDER << std::string(60, '=') << CLR_RESET << "\n";

    for (int u = 0; u < g.n; u++) {
        std::cout << "\n  [" << std::setw(2) << u << "] "
                  << CLR_BASE << g.getName(u) << CLR_RESET << "\n";

        if (g.adj[u].empty()) {
            std::cout << "        (no outgoing routes)\n";
        } else {
            for (const auto& e : g.adj[u]) {
                int v = e.to, w = e.weight, r = e.risk;
                const char* rc = RiskUtil::color(r);
                std::cout << "        " << rc << "->" << CLR_RESET
                          << " [" << std::setw(2) << v << "] "
                          << CLR_BASE << std::left << std::setw(22) << g.getName(v)
                          << CLR_RESET << std::right
                          << " (dist: " << std::setw(4) << w << ", "
                          << rc << "risk: " << std::setw(2) << r << " " << RiskUtil::label(r)
                          << CLR_RESET << ")\n";
            }
        }
    }
    std::cout << "  " << CLR_BORDER << std::string(60, '=') << CLR_RESET << "\n";
}

// -----------------------------------------------------------------------------
//  Adjacency Matrix
// -----------------------------------------------------------------------------

void Visualization::printAdjMatrix(const Graph& g) {
    if (g.empty()) { std::cout << "  (no network loaded)\n"; return; }

    int n = g.n;
    if (n > 20) {
        std::cout << "  (matrix display skipped - graph has " << n
                  << " nodes; limit is 20)\n";
        return;
    }

    auto mat = g.getAdjMatrix();

    std::cout << "\n  " << CLR_BORDER << "ADJACENCY MATRIX  (INF = no direct route)" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << std::string(10 + 7 * n, '-') << CLR_RESET << "\n";

    // Column headers
    std::cout << "  " << std::setw(8) << " ";
    for (int v = 0; v < n; v++)
        std::cout << std::setw(7) << ("[" + std::to_string(v) + "]");
    std::cout << "\n  " << CLR_BORDER << std::string(8 + 7 * n, '-') << CLR_RESET << "\n";

    for (int u = 0; u < n; u++) {
        std::cout << "  [" << std::setw(2) << u << "]   ";
        for (int v = 0; v < n; v++) {
            if (mat[u][v] >= INF)
                std::cout << std::setw(7) << "INF";
            else if (u == v)
                std::cout << std::setw(7) << "  0";
            else
                std::cout << std::setw(7) << mat[u][v];
        }
        std::cout << "   " << CLR_BASE << g.getName(u) << CLR_RESET << "\n";
    }
    std::cout << "  " << CLR_BORDER << std::string(10 + 7 * n, '-') << CLR_RESET << "\n";
}

// -----------------------------------------------------------------------------
//  ASCII Map - each route shown as [From] ---> [To] with its weight
// -----------------------------------------------------------------------------

void Visualization::printASCIIMap(const Graph& g) {
    if (g.empty()) { std::cout << "  (no network loaded)\n"; return; }

    std::cout << CLR_BORDER << "=== ASCII NETWORK MAP ===" << CLR_RESET << "\n";
    RiskUtil::printLegend();

    for (int u = 0; u < g.n; u++) {
        for (const auto& e : g.adj[u]) {
            int v = e.to, w = e.weight, r = e.risk;
            const char* rc = RiskUtil::color(r);
            std::cout << "[" << CLR_BASE << g.getName(u) << CLR_RESET << "] "
                      << rc << "---(d:" << w << " r:" << r << " " << RiskUtil::label(r)
                      << ")--->" << CLR_RESET
                      << " [" << CLR_BASE << g.getName(v) << CLR_RESET << "]\n";
        }
    }
}

// -----------------------------------------------------------------------------
//  Statistics
// -----------------------------------------------------------------------------

void Visualization::printStats(const Graph& g) {
    if (g.empty()) { std::cout << "  (no network loaded)\n"; return; }

    int totalEdges = static_cast<int>(g.edgeList.size());
    int minW = INF, maxW = 0, sumW = 0;
    int minR = 11, maxR = 0, sumR = 0;
    for (const Edge& e : g.edgeList) {
        minW = std::min(minW, e.weight);
        maxW = std::max(maxW, e.weight);
        sumW += e.weight;
        minR = std::min(minR, e.risk);
        maxR = std::max(maxR, e.risk);
        sumR += e.risk;
    }

    int busiest = 0;
    for (int u = 1; u < g.n; u++)
        if (g.adj[u].size() > g.adj[busiest].size()) busiest = u;

    std::cout << "\n  " << CLR_BORDER << "NETWORK STATISTICS" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << std::string(50, '=') << CLR_RESET << "\n";
    std::cout << "  Total bases (nodes)   : " << g.n        << "\n";
    std::cout << "  Total routes (edges)  : " << totalEdges << "\n";
    std::cout << "  Network type          : "
              << (g.directed ? "Directed" : "Undirected") << "\n";
    std::cout << "  Most connected base   : " << CLR_BASE << g.getName(busiest) << CLR_RESET
              << " (" << g.adj[busiest].size() << " connections)\n";
    if (totalEdges > 0) {
        std::cout << "  Min route distance    : " << minW << "\n";
        std::cout << "  Max route distance    : " << maxW << "\n";
        std::cout << "  Avg route distance    : " << sumW / totalEdges << "\n";
        std::cout << "  Total network cost    : " << sumW << "\n";
        std::cout << "  Min route risk        : " << RiskUtil::color(minR) << minR
                  << " (" << RiskUtil::label(minR) << ")" << CLR_RESET << "\n";
        std::cout << "  Max route risk        : " << RiskUtil::color(maxR) << maxR
                  << " (" << RiskUtil::label(maxR) << ")" << CLR_RESET << "\n";
        std::cout << "  Avg route risk        : " << sumR / totalEdges << "\n";
    }
    std::cout << "  " << CLR_BORDER << std::string(50, '=') << CLR_RESET << "\n";
}
