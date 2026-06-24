#include "TSP.h"
#include "Colors.h"
#include "RiskUtil.h"
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <numeric>

// -----------------------------------------------------------------------------
//  Brute Force - generate all permutations of interior nodes
// -----------------------------------------------------------------------------

void TSP::permute(const std::vector<std::vector<int>>& mat,
                  std::vector<int>& nodes, int l, int r,
                  int start, TSPResult& best) {
    if (l == r) {
        int cost = 0;
        bool feasible = true;

        int prev = start;
        for (int node : nodes) {
            if (mat[prev][node] >= INF) { feasible = false; break; }
            cost += mat[prev][node];
            prev = node;
        }
        if (feasible && mat[prev][start] < INF) {
            cost += mat[prev][start];
            if (cost < best.cost) {
                best.cost  = cost;
                best.found = true;
                best.path.clear();
                best.path.push_back(start);
                for (int n : nodes) best.path.push_back(n);
                best.path.push_back(start);
            }
        }
        return;
    }
    for (int i = l; i <= r; i++) {
        std::swap(nodes[l], nodes[i]);
        permute(mat, nodes, l + 1, r, start, best);
        std::swap(nodes[l], nodes[i]);
    }
}

TSPResult TSP::bruteForce(const Graph& g, int start) {
    TSPResult res;
    int n = g.size();
    if (n <= 1) { res.found = true; res.cost = 0; return res; }

    if (n > 12) {
        std::cout << "\n  " << CLR_WARN << "  Brute force is impractical for n > 12 (this graph has "
                  << n << " nodes).\n"
                  << "  Use the Nearest Neighbor heuristic instead." << CLR_RESET << "\n";
        return res;
    }

    auto apsp = ShortestPath::floydWarshall(g);
    auto& mat = apsp.dist;

    std::vector<int> nodes;
    for (int i = 0; i < n; i++)
        if (i != start) nodes.push_back(i);

    permute(mat, nodes, 0, static_cast<int>(nodes.size()) - 1, start, res);
    return res;
}

// -----------------------------------------------------------------------------
//  Nearest Neighbor Heuristic
// -----------------------------------------------------------------------------

TSPResult TSP::nearestNeighbor(const Graph& g, int start) {
    TSPResult res;
    int n = g.size();
    if (n <= 1) { res.found = true; res.cost = 0; return res; }

    auto apsp = ShortestPath::floydWarshall(g);
    auto mat   = apsp.dist;

    std::vector<bool> visited(n, false);
    res.path.push_back(start);
    visited[start] = true;
    res.cost  = 0;
    res.found = true;

    int current = start;
    for (int step = 0; step < n - 1; step++) {
        int best  = -1;
        int bestW = INF;
        for (int v = 0; v < n; v++) {
            if (!visited[v] && mat[current][v] < bestW) {
                bestW = mat[current][v];
                best  = v;
            }
        }
        if (best == -1 || bestW >= INF) { res.found = false; break; }
        visited[best] = true;
        res.path.push_back(best);
        res.cost  += bestW;
        current    = best;
    }

    if (res.found) {
        if (mat[current][start] >= INF) {
            res.found = false;
        } else {
            res.cost += mat[current][start];
            res.path.push_back(start);
        }
    }
    return res;
}

// -----------------------------------------------------------------------------
//  Output
// -----------------------------------------------------------------------------

void TSP::printResult(const TSPResult& res, const Graph& g,
                      const std::string& algorithm) {
    std::cout << "\n  " << CLR_RESULT << "[TSP - " << algorithm << "]" << CLR_RESET << "\n";
    RiskUtil::printLegend();
    std::cout << "  " << CLR_BORDER << std::string(62, '-') << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << std::left
              << "  " << std::setw(5) << "Stop"
              << std::setw(6) << "ID"
              << std::setw(24) << "Base Name"
              << "Edge to next (dist / risk)" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << std::string(62, '-') << CLR_RESET << "\n";

    if (!res.found) {
        std::cout << "  " << CLR_WARN
                  << "  No complete Hamiltonian circuit found (graph may be disconnected)."
                  << CLR_RESET << "\n";
        std::cout << "  " << CLR_BORDER << std::string(62, '-') << CLR_RESET << "\n";
        return;
    }

    for (int i = 0; i < static_cast<int>(res.path.size()); i++) {
        int node = res.path[i];
        std::cout << "  " << CLR_RESULT << std::right
                  << "  " << std::setw(4) << (i + 1) << "   "
                  << "[" << std::setw(2) << node << "] "
                  << std::left << std::setw(24) << g.getName(node) << std::right;

        // Edge to next node
        if (i + 1 < static_cast<int>(res.path.size())) {
            int next = res.path[i + 1];
            int er = g.getRisk(node, next);
            if (er < 0) er = 0;
            int ew = 0;
            for (const auto& e : g.adj[node])
                if (e.to == next) { ew = e.weight; break; }
            const char* rc = RiskUtil::color(er);
            if (ew > 0) {
                // direct edge
                std::cout << rc << "  d:" << std::setw(4) << ew
                          << " r:" << er << " " << RiskUtil::label(er) << CLR_RESET;
            } else {
                // multi-hop via APSP
                std::cout << CLR_RESULT << "  (via shortest path)" << CLR_RESET;
            }
        } else {
            std::cout << CLR_RESULT << "  [circuit complete]" << CLR_RESET;
        }
        std::cout << "\n";
    }
    std::cout << "  " << CLR_BORDER << std::string(62, '-') << CLR_RESET << "\n";
    std::cout << "  " << CLR_RESULT << "  Total route cost : " << res.cost << CLR_RESET << "\n";
    std::cout << "  " << CLR_RESULT << "  Stops in circuit : " << res.path.size() << CLR_RESET << "\n";
}

// -----------------------------------------------------------------------------
//  Complexity stats block
// -----------------------------------------------------------------------------

void TSP::printComplexity(const std::string& algo, int n, long long ms) {
    std::string timeO;
    if (algo == "Brute Force")      timeO = "O(n!)          ";
    else                            timeO = "O(n^2)         ";

    std::cout << "\n  " << CLR_BORDER << std::string(54, '-') << CLR_RESET << "\n";
    std::cout << "  " << CLR_RESULT << "  Complexity [TSP - " << algo << "]" << CLR_RESET << "\n";
    std::cout << "  " << CLR_RESULT
              << "    Time  : " << timeO
              << "   Space : O(n)" << CLR_RESET << "\n";
    std::cout << "  " << CLR_RESULT
              << "    Input : n=" << n << " nodes"
              << "              Elapsed: " << ms << " ms" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << std::string(54, '-') << CLR_RESET << "\n";
}
