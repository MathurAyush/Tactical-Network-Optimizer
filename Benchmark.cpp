#include "Benchmark.h"
#include "MST.h"
#include "Traversal.h"
#include "ShortestPath.h"
#include "MaxFlow.h"
#include "TSP.h"
#include "SCC.h"
#include "Centrality.h"
#include "Colors.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <random>
#include <algorithm>
#include <sstream>

using Clock = std::chrono::steady_clock;

static long long ms(Clock::time_point t0) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               Clock::now() - t0).count();
}

// -----------------------------------------------------------------------------
//  Random connected graph generator
//  Guarantees connectivity by first building a random spanning tree (n-1 edges),
//  then appending extra random edges.
// -----------------------------------------------------------------------------

Graph Benchmark::generateRandom(int n, int extraEdges, int seed) {
    std::mt19937 rng(seed);
    Graph g(n, false);

    // Spanning tree via random Prüfer-like insertion
    std::vector<int> inTree = {0};
    std::vector<int> notIn;
    for (int i = 1; i < n; i++) notIn.push_back(i);

    while (!notIn.empty()) {
        std::uniform_int_distribution<int> inDist(0, (int)inTree.size() - 1);
        std::uniform_int_distribution<int> outDist(0, (int)notIn.size() - 1);
        int u  = inTree[inDist(rng)];
        int vi = outDist(rng);
        int v  = notIn[vi];
        notIn.erase(notIn.begin() + vi);
        inTree.push_back(v);

        std::uniform_int_distribution<int> wDist(10, 200);
        std::uniform_int_distribution<int> rDist(0, 10);
        g.addEdge(u, v, wDist(rng), rDist(rng), true);
    }

    // Extra random edges
    std::uniform_int_distribution<int> nodeDist(0, n - 1);
    std::uniform_int_distribution<int> wDist(10, 200);
    std::uniform_int_distribution<int> rDist(0, 10);
    int added = 0, attempts = 0;
    while (added < extraEdges && attempts < extraEdges * 10) {
        int u = nodeDist(rng), v = nodeDist(rng);
        attempts++;
        if (u == v) continue;
        g.addEdge(u, v, wDist(rng), rDist(rng), true);
        added++;
    }

    // Name nodes
    for (int i = 0; i < n; i++)
        g.setName(i, "B" + std::to_string(i));

    return g;
}

// -----------------------------------------------------------------------------
//  Run all benchmarks
// -----------------------------------------------------------------------------

struct BenchRow {
    int n, e;
    std::string algo;
    std::string theory;
    long long   timeMs;
    std::string result;
};

void Benchmark::runAll() {
    std::cout << "\n  " << CLR_BORDER << "+" << std::string(72, '=') << "+" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|  " << CLR_RESULT
              << std::left << std::setw(70)
              << "ALGORITHM BENCHMARK  -  Empirical vs Theoretical Complexity"
              << CLR_BORDER << "|" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "+" << std::string(72, '=') << "+" << CLR_RESET << "\n";
    std::cout << "  " << CLR_RESULT
              << "  Generating random connected graphs with seed=42...\n"
              << "  (highlighted rows >100 ms flag algorithms that may bottleneck large inputs)\n"
              << CLR_RESET;

    std::vector<BenchRow> rows;

    // Graph sizes: n=10, 50, 100, 200 with ~2n edges (sparse)
    const int sizes[]      = {10,  50,  100, 200};
    const int extraEdge[]  = {10,  50,  100, 200};

    for (int si = 0; si < 4; si++) {
        int n = sizes[si];
        int extra = extraEdge[si];
        Graph g = generateRandom(n, extra, 42);
        int E = static_cast<int>(g.edgeList.size());

        // 1. Kruskal MST
        {
            auto t0 = Clock::now();
            MSTResult r = MST::kruskal(g);
            long long t = ms(t0);
            rows.push_back({n, E, "Kruskal MST", "O(E log E)",
                            t, r.valid ? "w=" + std::to_string(r.totalWeight) : "disconn"});
        }

        // 2. Prim MST
        {
            auto t0 = Clock::now();
            MSTResult r = MST::prim(g, 0);
            long long t = ms(t0);
            rows.push_back({n, E, "Prim MST", "O((V+E) log V)",
                            t, r.valid ? "w=" + std::to_string(r.totalWeight) : "disconn"});
        }

        // 3. BFS
        {
            std::vector<int> par(n, -1);
            auto t0 = Clock::now();
            auto ord = Traversal::bfs(g, 0, par);
            long long t = ms(t0);
            rows.push_back({n, E, "BFS", "O(V + E)",
                            t, std::to_string(ord.size()) + " visited"});
        }

        // 4. DFS
        {
            std::vector<int> par(n, -1);
            auto t0 = Clock::now();
            auto ord = Traversal::dfs(g, 0, par);
            long long t = ms(t0);
            rows.push_back({n, E, "DFS", "O(V + E)",
                            t, std::to_string(ord.size()) + " visited"});
        }

        // 5. Dijkstra
        {
            auto t0 = Clock::now();
            auto r = ShortestPath::dijkstra(g, 0);
            long long t = ms(t0);
            int reach = 0;
            for (int i = 1; i < n; i++) if (r.dist[i] < INF) reach++;
            rows.push_back({n, E, "Dijkstra", "O((V+E) log V)",
                            t, std::to_string(reach) + " paths"});
        }

        // 6. Bellman-Ford
        {
            auto t0 = Clock::now();
            auto r = ShortestPath::bellmanFord(g, 0);
            long long t = ms(t0);
            rows.push_back({n, E, "Bellman-Ford", "O(V * E)",
                            t, r.hasNegCycle ? "neg-cycle" : "ok"});
        }

        // 7. Floyd-Warshall (skip n=200 to avoid multi-second run in demo)
        if (n <= 100) {
            auto t0 = Clock::now();
            auto r = ShortestPath::floydWarshall(g);
            long long t = ms(t0);
            rows.push_back({n, E, "Floyd-Warshall", "O(V^3)",
                            t, r.hasNegCycle ? "neg-cycle" : "all-pairs"});
        } else {
            rows.push_back({n, E, "Floyd-Warshall", "O(V^3)", -1, "skipped (n>100)"});
        }

        // 8. Edmonds-Karp Max Flow
        {
            auto t0 = Clock::now();
            auto r = MaxFlow::edmondsKarp(g, 0, n - 1);
            long long t = ms(t0);
            rows.push_back({n, E, "Edmonds-Karp", "O(V * E^2)",
                            t, "flow=" + std::to_string(r.maxFlow)});
        }

        // 9. TSP Nearest Neighbor
        {
            auto t0 = Clock::now();
            auto r = TSP::nearestNeighbor(g, 0);
            long long t = ms(t0);
            rows.push_back({n, E, "TSP NearNeighbor", "O(V^2)",
                            t, r.found ? "cost=" + std::to_string(r.cost) : "no circuit"});
        }

        // 10. TSP Brute Force (only n<=10 — O(n!) is lethal beyond that)
        if (n <= 10) {
            auto t0 = Clock::now();
            auto r = TSP::bruteForce(g, 0);
            long long t = ms(t0);
            rows.push_back({n, E, "TSP BruteForce", "O(V!)",
                            t, r.found ? "cost=" + std::to_string(r.cost) : "no circuit"});
        } else {
            rows.push_back({n, E, "TSP BruteForce", "O(V!)", -1,
                            "skipped (n>" + std::to_string(10) + ")"});
        }

        // 11. SCC / Connected Components
        {
            auto t0 = Clock::now();
            auto r = SCC::analyze(g);
            long long t = ms(t0);
            rows.push_back({n, E, "SCC (Tarjan's)", "O(V + E)",
                            t, std::to_string(r.numComponents) + " component(s)"});
        }

        // 12. Centrality (skip n=200 — O(V*(V+E)logV) is significant)
        if (n <= 100) {
            auto t0 = Clock::now();
            auto r = Centrality::analyze(g);
            long long t = ms(t0);
            // Find most central node
            int top = 0;
            for (int i = 1; i < n; i++)
                if (r.composite[i] > r.composite[top]) top = i;
            rows.push_back({n, E, "Centrality", "O(V*(V+E)logV)",
                            t, "top=[" + std::to_string(top) + "]"});
        } else {
            rows.push_back({n, E, "Centrality", "O(V*(V+E)logV)", -1, "skipped (n>100)"});
        }
    }

    // Replace -1 times with marker
    for (auto& r : rows)
        if (r.timeMs < 0) r.timeMs = -1; // printed as "---"

    // Custom print to handle -1
    std::cout << "\n  " << CLR_BORDER
              << "+======+======+====================+===================+========+==================+"
              << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "| "
              << CLR_RESULT << std::left
              << std::setw(5)  << "V"
              << std::setw(6)  << "E"
              << std::setw(21) << "Algorithm"
              << std::setw(20) << "Complexity"
              << std::setw(9)  << "ms"
              << std::setw(18) << "Result"
              << CLR_BORDER << "|" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER
              << "+------+------+--------------------+-------------------+--------+------------------+"
              << CLR_RESET << "\n";

    int prevN = -1;
    for (const auto& r : rows) {
        if (r.n != prevN && prevN != -1) {
            std::cout << "  " << CLR_BORDER
                      << "+------+------+--------------------+-------------------+--------+------------------+"
                      << CLR_RESET << "\n";
        }
        prevN = r.n;

        std::string algoStr   = r.algo.size()   > 19 ? r.algo.substr(0, 19)   : r.algo;
        std::string theoryStr = r.theory.size() > 18 ? r.theory.substr(0, 18) : r.theory;
        std::string resStr    = r.result.size() > 17 ? r.result.substr(0, 17) : r.result;

        const char* col = (r.timeMs > 100) ? CLR_WARN :
                          (r.timeMs < 0)   ? CLR_BORDER : CLR_RESULT;

        std::string timeStr = (r.timeMs < 0) ? "---" : std::to_string(r.timeMs);

        std::cout << "  " << CLR_BORDER << "| " << col
                  << std::left
                  << std::setw(5)  << r.n
                  << std::setw(6)  << r.e
                  << std::setw(21) << algoStr
                  << std::setw(20) << theoryStr
                  << std::setw(9)  << timeStr
                  << std::setw(18) << resStr
                  << CLR_BORDER << "|" << CLR_RESET << "\n";
    }
    std::cout << "  " << CLR_BORDER
              << "+======+======+====================+===================+========+==================+"
              << CLR_RESET << "\n";

    std::cout << "\n  " << CLR_RESULT
              << "  Key observations:\n"
              << "    O(V!)     : TSP Brute Force explodes — 12! = 479M ops\n"
              << "    O(V^3)    : Floyd-Warshall scales cubically — costly at V=200\n"
              << "    O(V*E^2)  : Edmonds-Karp is acceptable for sparse graphs\n"
              << "    O((V+E)logV): Dijkstra/Prim/A* are fast even at V=200\n"
              << "    O(V+E)    : BFS/DFS/Tarjan SCC are near-linear — always cheap"
              << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << std::string(74, '-') << CLR_RESET << "\n";
}
