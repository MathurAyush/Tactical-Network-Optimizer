#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <limits>
#include <algorithm>
#include <cstdlib>
#include <chrono>
#ifdef _WIN32
#  include <windows.h>
#  include <io.h>
#  define IS_TTY() (_isatty(_fileno(stdin)) != 0)
#else
#  include <unistd.h>
#  define IS_TTY() (isatty(STDIN_FILENO) != 0)
#endif

#include "Colors.h"
#include "RiskUtil.h"
#include "Graph.h"
#include "MST.h"
#include "Traversal.h"
#include "ShortestPath.h"
#include "MaxFlow.h"
#include "TSP.h"
#include "Visualization.h"
#include "FileLoader.h"
#include "Resilience.h"
#include "MissionPlanner.h"
#include "MapEditor.h"
#include "MissionLog.h"
#include "AStar.h"
#include "SCC.h"
#include "Centrality.h"
#include "Benchmark.h"

// -----------------------------------------------------------------------------
//  Utility: safe integer input
// -----------------------------------------------------------------------------

static int readInt(const std::string& prompt, int lo = 0, int hi = INT_MAX) {
    int val;
    while (true) {
        std::cout << prompt << std::flush;
        if (std::cin >> val && val >= lo && val <= hi) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return val;
        }
        if (std::cin.eof()) return lo;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "  Invalid input - enter a number between " << lo << " and " << hi << ".\n";
    }
}

static std::string readString(const std::string& prompt) {
    std::cout << prompt << std::flush;
    std::string s;
    if (std::cin >> s)
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return s;
}

// -----------------------------------------------------------------------------
//  Timing
// -----------------------------------------------------------------------------

using Clock = std::chrono::steady_clock;

static long long elapsedMs(Clock::time_point t0) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - t0).count();
}

// -----------------------------------------------------------------------------
//  Pause helper
// -----------------------------------------------------------------------------

static void pauseMenu() {
    if (!IS_TTY()) return;
    std::cout << "\n  Press ENTER to continue..." << std::flush;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// -- Module 1: MST -------------------------------------------------------------

static void menuMST(const Graph& g) {
    if (g.empty()) { std::cout << "  " << CLR_WARN << "No network loaded." << CLR_RESET << "\n"; return; }

    std::cout << "\n  " << CLR_BORDER << "+-------------------------------------+" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|    Minimum Spanning Tree (MST)      |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "+-------------------------------------+" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[1]" << CLR_RESET << CLR_BORDER << " Kruskal's Algorithm            |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[2]" << CLR_RESET << CLR_BORDER << " Prim's Algorithm               |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[3]" << CLR_RESET << CLR_BORDER << " Run Both & Compare             |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[0]" << CLR_RESET << CLR_BORDER << " Back                           |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "+-------------------------------------+" << CLR_RESET << "\n";

    int ch = readInt("  Choice: ", 0, 3);
    if (ch == 0) return;

    if (ch == 1 || ch == 3) {
        auto t0 = Clock::now();
        MSTResult kr = MST::kruskal(g);
        long long ms = elapsedMs(t0);
        MST::printResult(kr, g, "Kruskal");
        MST::printASCIITree(kr, g, 0);
        MST::printComplexity("Kruskal", g.n, static_cast<int>(g.edgeList.size()), ms);
        MissionLog::record("MST-Kruskal", "graph(" + std::to_string(g.n) + " bases)",
                           kr.valid ? ("MST weight=" + std::to_string(kr.totalWeight))
                                    : "disconnected - no MST", ms);
    }
    if (ch == 2 || ch == 3) {
        int start = readInt("  Starting base ID (0-" + std::to_string(g.n-1) + "): ",
                            0, g.n - 1);
        auto t0 = Clock::now();
        MSTResult pr = MST::prim(g, start);
        long long ms = elapsedMs(t0);
        MST::printResult(pr, g, "Prim");
        MST::printASCIITree(pr, g, start);
        MST::printComplexity("Prim", g.n, static_cast<int>(g.edgeList.size()), ms);
        MissionLog::record("MST-Prim", "start=" + std::to_string(start),
                           pr.valid ? ("MST weight=" + std::to_string(pr.totalWeight))
                                    : "disconnected - no MST", ms);
    }
    std::cin.get();
}

// -- Module 2: Traversal -------------------------------------------------------

static void menuTraversal(const Graph& g) {
    if (g.empty()) { std::cout << "  " << CLR_WARN << "No network loaded." << CLR_RESET << "\n"; return; }

    std::cout << "\n  " << CLR_BORDER << "+-------------------------------------+" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|        Network Traversal            |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "+-------------------------------------+" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[1]" << CLR_RESET << CLR_BORDER << " Breadth-First Search (BFS)     |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[2]" << CLR_RESET << CLR_BORDER << " Depth-First Search  (DFS)      |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[3]" << CLR_RESET << CLR_BORDER << " Run Both from same start       |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[0]" << CLR_RESET << CLR_BORDER << " Back                           |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "+-------------------------------------+" << CLR_RESET << "\n";

    int ch = readInt("  Choice: ", 0, 3);
    if (ch == 0) return;

    int start = readInt("  Starting base ID (0-" + std::to_string(g.n-1) + "): ",
                        0, g.n - 1);

    if (ch == 1 || ch == 3) {
        std::vector<int> parent(g.n, -1);
        auto t0 = Clock::now();
        auto order = Traversal::bfs(g, start, parent);
        long long ms = elapsedMs(t0);
        Traversal::printResult(order, g, "BFS", start);
        Traversal::printASCIITree(parent, g, start);
        Traversal::printComplexity("BFS", g.n, static_cast<int>(g.edgeList.size()), ms);
        MissionLog::record("Traversal-BFS", "start=" + std::to_string(start),
                           "visited " + std::to_string(order.size()) + " bases", ms);
    }
    if (ch == 2 || ch == 3) {
        std::vector<int> parent(g.n, -1);
        auto t0 = Clock::now();
        auto order = Traversal::dfs(g, start, parent);
        long long ms = elapsedMs(t0);
        Traversal::printResult(order, g, "DFS", start);
        Traversal::printASCIITree(parent, g, start);
        Traversal::printComplexity("DFS", g.n, static_cast<int>(g.edgeList.size()), ms);
        MissionLog::record("Traversal-DFS", "start=" + std::to_string(start),
                           "visited " + std::to_string(order.size()) + " bases", ms);
    }
    std::cin.get();
}

// Compare Fastest vs Safest path
static void compareSafestFastest(const Graph& g, int src, int dst) {
    std::cout << "\n  " << CLR_RESULT << "[Safest vs Fastest Route: "
              << g.getName(src) << " -> " << g.getName(dst) << "]" << CLR_RESET << "\n";
    RiskUtil::printLegend();
    std::cout << "  " << std::string(60, '-') << "\n";

    auto fastRes  = ShortestPath::dijkstra(g, src);
    auto fastPath = ShortestPath::reconstructPath(fastRes.prev, src, dst);

    Graph rg = g.riskGraph();
    auto safeRes  = ShortestPath::dijkstra(rg, src);
    auto safePath = ShortestPath::reconstructPath(safeRes.prev, src, dst);

    auto show = [&](const std::string& title, const std::vector<int>& path, int distTotal) {
        std::cout << "\n  " << CLR_OPTION << title << CLR_RESET << "\n";
        if (path.empty()) {
            std::cout << "  " << CLR_WARN << "  No route found." << CLR_RESET << "\n";
            return;
        }
        std::cout << "    ";
        RiskUtil::printColoredPath(g, path);
        std::cout << "\n";
        std::cout << "    Total distance : " << distTotal << "\n";
        std::cout << "    Total risk     : " << RiskUtil::pathRisk(g, path) << "\n";
    };

    show("FASTEST ROUTE (minimises distance)", fastPath,
         fastRes.dist[dst] >= INF ? -1 : fastRes.dist[dst]);
    show("SAFEST ROUTE (minimises risk)", safePath,
         [&]{
             int d = 0;
             for (size_t i = 0; i + 1 < safePath.size(); i++)
                 for (const auto& e : g.adj[safePath[i]])
                     if (e.to == safePath[i+1]) { d += e.weight; break; }
             return safePath.empty() ? -1 : d;
         }());

    std::cout << "  " << std::string(60, '-') << "\n";
}

// -- Module 3: Shortest Path ---------------------------------------------------

static void menuShortestPath(const Graph& g) {
    if (g.empty()) { std::cout << "  " << CLR_WARN << "No network loaded." << CLR_RESET << "\n"; return; }

    std::cout << "\n  " << CLR_BORDER << "+-------------------------------------+" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|      Shortest Path Analysis         |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "+-------------------------------------+" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[1]" << CLR_RESET << CLR_BORDER << " Dijkstra's Algorithm           |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[2]" << CLR_RESET << CLR_BORDER << " Bellman-Ford Algorithm         |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[3]" << CLR_RESET << CLR_BORDER << " Floyd-Warshall (All-Pairs)     |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[4]" << CLR_RESET << CLR_BORDER << " Safest vs Fastest Path         |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[5]" << CLR_RESET << CLR_BORDER << " A* Search (Heuristic)          |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[0]" << CLR_RESET << CLR_BORDER << " Back                           |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "+-------------------------------------+" << CLR_RESET << "\n";

    int ch = readInt("  Choice: ", 0, 5);
    if (ch == 0) return;

    if (ch == 5) {
        int src = readInt("  Source base ID (0-" + std::to_string(g.n-1) + "): ", 0, g.n - 1);
        int dst = readInt("  Destination base ID (0-" + std::to_string(g.n-1) + "): ", 0, g.n - 1);
        if (src == dst) { std::cout << "  " << CLR_WARN << "Source and destination must differ." << CLR_RESET << "\n"; return; }
        auto t0 = Clock::now();
        AStar::compareWithDijkstra(g, src, dst);
        long long ms = elapsedMs(t0);
        AStarResult ar = AStar::search(g, src, dst);
        AStar::printComplexity(g.n, static_cast<int>(g.edgeList.size()), ar.nodesExpanded, ms);
        MissionLog::record("AStar",
                           "src=" + std::to_string(src) + " dst=" + std::to_string(dst),
                           ar.found ? "dist=" + std::to_string(ar.dist[dst]) : "no path", ms);
        std::cin.get();
        return;
    }

    if (ch == 4) {
        int src = readInt("  Source base ID (0-" + std::to_string(g.n-1) + "): ", 0, g.n - 1);
        int dst = readInt("  Destination base ID (0-" + std::to_string(g.n-1) + "): ", 0, g.n - 1);
        if (src == dst) { std::cout << "  " << CLR_WARN << "Source and destination must differ." << CLR_RESET << "\n"; }
        else compareSafestFastest(g, src, dst);
        std::cin.get();
        return;
    }

    if (ch == 1 || ch == 2) {
        int src = readInt("  Source base ID (0-" + std::to_string(g.n-1) + "): ",
                          0, g.n - 1);
        std::cout << "  Destination ID (-1 for all bases): ";
        int dst;
        std::cin >> dst;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (dst < -1 || dst >= g.n) dst = -1;

        std::string in = "src=" + std::to_string(src) + " dst=" + (dst == -1 ? "all" : std::to_string(dst));
        if (ch == 1) {
            auto t0 = Clock::now();
            auto res = ShortestPath::dijkstra(g, src);
            long long ms = elapsedMs(t0);
            ShortestPath::printSP(res, g, src, dst);
            ShortestPath::printComplexity("Dijkstra", g.n, static_cast<int>(g.edgeList.size()), ms);
            std::string result = (dst == -1 || res.dist[dst] >= INF)
                                 ? "see all-pairs output" : ("dist=" + std::to_string(res.dist[dst]));
            MissionLog::record("ShortestPath-Dijkstra", in, result, ms);
        } else {
            auto t0 = Clock::now();
            auto res = ShortestPath::bellmanFord(g, src);
            long long ms = elapsedMs(t0);
            ShortestPath::printSP(res, g, src, dst);
            ShortestPath::printComplexity("Bellman-Ford", g.n, static_cast<int>(g.edgeList.size()), ms);
            std::string result = res.hasNegCycle ? "negative cycle detected"
                               : (dst == -1 || res.dist[dst] >= INF)
                                 ? "see all-pairs output" : ("dist=" + std::to_string(res.dist[dst]));
            MissionLog::record("ShortestPath-BellmanFord", in, result, ms);
        }
    } else if (ch == 3) {
        auto t0 = Clock::now();
        auto res = ShortestPath::floydWarshall(g);
        long long ms = elapsedMs(t0);
        ShortestPath::printAPSP(res, g);
        ShortestPath::printComplexity("Floyd-Warshall", g.n, static_cast<int>(g.edgeList.size()), ms);
        MissionLog::record("ShortestPath-FloydWarshall", "all-pairs(" + std::to_string(g.n) + " bases)",
                           res.hasNegCycle ? "negative cycle detected" : "all-pairs matrix computed", ms);
    }
    std::cin.get();
}

// -- Module 4: Max Flow --------------------------------------------------------

static void menuMaxFlow(const Graph& g) {
    if (g.empty()) { std::cout << "  " << CLR_WARN << "No network loaded." << CLR_RESET << "\n"; return; }

    std::cout << "\n  " << CLR_BORDER << "+-------------------------------------+" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|   Max Flow - Edmonds-Karp           |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|   Models maximum supply throughput  |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "+-------------------------------------+" << CLR_RESET << "\n";

    std::cout << "\n  Available bases:\n";
    for (int i = 0; i < g.n; i++)
        std::cout << "    [" << i << "] " << CLR_BASE << g.getName(i) << CLR_RESET << "\n";

    int src  = readInt("\n  Source base ID : ", 0, g.n - 1);
    int sink = readInt("  Sink   base ID : ", 0, g.n - 1);

    if (src == sink) { std::cout << "  " << CLR_WARN << "Source and sink must differ." << CLR_RESET << "\n"; return; }

    auto t0 = Clock::now();
    auto res = MaxFlow::edmondsKarp(g, src, sink);
    long long ms = elapsedMs(t0);
    MaxFlow::printResult(res, g, src, sink);
    MaxFlow::printComplexity(g.n, static_cast<int>(g.edgeList.size()), ms);
    MissionLog::record("MaxFlow-EdmondsKarp",
                       "source=" + std::to_string(src) + " sink=" + std::to_string(sink),
                       "maxFlow=" + std::to_string(res.maxFlow), ms);
    std::cin.get();
}

// -- Module 5: TSP -------------------------------------------------------------

static void menuTSP(const Graph& g) {
    if (g.empty()) { std::cout << "  " << CLR_WARN << "No network loaded." << CLR_RESET << "\n"; return; }

    std::cout << "\n  " << CLR_BORDER << "+-------------------------------------+" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|    Travelling Salesman Problem      |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|       (Patrol Route Planning)       |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "+-------------------------------------+" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[1]" << CLR_RESET << CLR_BORDER << " Brute Force (exact, n <= 12)    |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[2]" << CLR_RESET << CLR_BORDER << " Nearest Neighbor Heuristic     |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[3]" << CLR_RESET << CLR_BORDER << " Run Both & Compare             |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[0]" << CLR_RESET << CLR_BORDER << " Back                           |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "+-------------------------------------+" << CLR_RESET << "\n";

    int ch = readInt("  Choice: ", 0, 3);
    if (ch == 0) return;

    int start = readInt("  Starting base ID (0-" + std::to_string(g.n-1) + "): ",
                        0, g.n - 1);

    if (ch == 1 || ch == 3) {
        auto t0 = Clock::now();
        auto res = TSP::bruteForce(g, start);
        long long ms = elapsedMs(t0);
        TSP::printResult(res, g, "Brute Force");
        TSP::printComplexity("Brute Force", g.n, ms);
        MissionLog::record("TSP-BruteForce", "start=" + std::to_string(start),
                           res.found ? ("cost=" + std::to_string(res.cost)) : "no Hamiltonian circuit", ms);
    }
    if (ch == 2 || ch == 3) {
        auto t0 = Clock::now();
        auto res = TSP::nearestNeighbor(g, start);
        long long ms = elapsedMs(t0);
        TSP::printResult(res, g, "Nearest Neighbor");
        TSP::printComplexity("Nearest Neighbor", g.n, ms);
        MissionLog::record("TSP-NearestNeighbor", "start=" + std::to_string(start),
                           res.found ? ("cost=" + std::to_string(res.cost)) : "no Hamiltonian circuit", ms);
    }
    std::cin.get();
}

// -- Module 6: Network Resilience Analyzer -------------------------------------

static void menuResilience(const Graph& g) {
    if (g.empty()) { std::cout << "  " << CLR_WARN << "No network loaded." << CLR_RESET << "\n"; return; }
    if (g.edgeList.empty()) { std::cout << "  " << CLR_WARN << "Network has no routes to analyze." << CLR_RESET << "\n"; return; }

    std::cout << "\n  " << CLR_BORDER << "+-------------------------------------+" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|     Network Resilience Analyzer     |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "+-------------------------------------+" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[1]" << CLR_RESET << CLR_BORDER << " Simulate Enemy Attack on Route |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[2]" << CLR_RESET << CLR_BORDER << " Full Report (Bridges + MinCut) |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[0]" << CLR_RESET << CLR_BORDER << " Back                           |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "+-------------------------------------+" << CLR_RESET << "\n";

    int ch = readInt("  Choice: ", 0, 2);
    if (ch == 0) return;

    if (ch == 1) {
        std::cout << "\n  Routes (select one to sever):\n";
        for (size_t i = 0; i < g.edgeList.size(); i++) {
            const Edge& e = g.edgeList[i];
            std::cout << "    [" << std::setw(2) << i << "] "
                      << RiskUtil::color(e.risk) << g.getName(e.from) << " <-> " << g.getName(e.to)
                      << " (d:" << e.weight << " r:" << e.risk << " " << RiskUtil::label(e.risk) << ")"
                      << CLR_RESET << "\n";
        }
        int idx = readInt("\n  Route index to attack: ", 0, static_cast<int>(g.edgeList.size()) - 1);
        int ref = readInt("  Reference base for connectivity check (0-" + std::to_string(g.n-1) + "): ",
                          0, g.n - 1);
        auto t0 = Clock::now();
        Resilience::printAttackResult(g, idx, ref);
        long long ms = elapsedMs(t0);
        const Edge& e = g.edgeList[idx];
        MissionLog::record("Resilience-SimulateAttack",
                           "route=[" + g.getName(e.from) + "<->" + g.getName(e.to) + "] ref=" + std::to_string(ref),
                           "attack simulated, BFS connectivity rechecked", ms);
    } else if (ch == 2) {
        int ref = readInt("  Reference base for connectivity checks (0-" + std::to_string(g.n-1) + "): ",
                          0, g.n - 1);
        auto t0 = Clock::now();
        auto rep = Resilience::analyze(g);
        long long ms = elapsedMs(t0);
        Resilience::printReport(rep, g, ref);
        MissionLog::record("Resilience-FullReport", "ref=" + std::to_string(ref),
                           std::to_string(rep.bridges.size()) + " bridges, minCut=" + std::to_string(rep.minCutValue), ms);
    }
    std::cin.get();
}

// -- Module 7: Mission Planner --------------------------------------------------

static void menuMissionPlanner(const Graph& g) {
    if (g.empty()) { std::cout << "  " << CLR_WARN << "No network loaded." << CLR_RESET << "\n"; return; }

    std::cout << "\n  " << CLR_BORDER << "+-------------------------------------+" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|          Mission Planner            |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|   Multi-base resupply / patrol run  |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "+-------------------------------------+" << CLR_RESET << "\n";

    std::cout << "\n  Available bases:\n";
    for (int i = 0; i < g.n; i++)
        std::cout << "    [" << i << "] " << CLR_BASE << g.getName(i) << CLR_RESET << "\n";

    int start = readInt("\n  Starting / home base ID: ", 0, g.n - 1);
    int count = readInt("  How many bases must be visited: ", 1, g.n);

    std::vector<int> targets;
    for (int i = 0; i < count; i++) {
        int id = readInt("  Base #" + std::to_string(i+1) + " to visit (ID): ", 0, g.n - 1);
        targets.push_back(id);
    }
    int budget = readInt("  Fuel / distance budget: ", 0, INT_MAX / 4);

    std::ostringstream inDesc;
    inDesc << "start=" << start << " visit=[";
    for (size_t i = 0; i < targets.size(); i++) { if (i) inDesc << ","; inDesc << targets[i]; }
    inDesc << "] budget=" << budget;

    auto t0 = Clock::now();
    auto plan = MissionPlanner::plan(g, start, targets, budget);
    long long ms = elapsedMs(t0);
    MissionPlanner::printPlan(plan, g, budget);

    std::string result = !plan.found ? "no feasible route"
                       : ("distance=" + std::to_string(plan.totalDistance) +
                          " risk=" + std::to_string(plan.totalRisk) +
                          (plan.withinBudget ? " withinBudget=yes" : " withinBudget=no"));
    MissionLog::record("MissionPlanner", inDesc.str(), result, ms);
    std::cin.get();
}

// -- Module 8: Dynamic Map Editor ------------------------------------------------

static void menuMapEditor(Graph& g) {
    std::cout << "\n  " << CLR_BORDER << "+-------------------------------------+" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|        Dynamic Map Editor           |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "+-------------------------------------+" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[1]" << CLR_RESET << CLR_BORDER << " Add a new base                 |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[2]" << CLR_RESET << CLR_BORDER << " Add a new route                |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[3]" << CLR_RESET << CLR_BORDER << " Remove a base                  |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[4]" << CLR_RESET << CLR_BORDER << " Remove a route                 |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[0]" << CLR_RESET << CLR_BORDER << " Back                           |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "+-------------------------------------+" << CLR_RESET << "\n";

    int ch = readInt("  Choice: ", 0, 4);
    if (ch == 0) return;

    bool changed = false;

    if (ch == 1) {
        std::string name = readString("  Name for new base: ");
        if (name.empty()) name = "Base_" + std::to_string(g.n);
        MapEditor::addBase(g, name);
        std::cout << "  " << CLR_BORDER << "Base [" << (g.n - 1) << "] " << name << " added." << CLR_RESET << "\n";
        changed = true;
    } else if (ch == 2) {
        if (g.n < 2) { std::cout << "  " << CLR_WARN << "Need at least 2 bases to add a route." << CLR_RESET << "\n"; return; }
        int from = readInt("  From base ID (0-" + std::to_string(g.n-1) + "): ", 0, g.n - 1);
        int to   = readInt("  To base ID (0-"   + std::to_string(g.n-1) + "): ", 0, g.n - 1);
        int dist = readInt("  Distance: ", 0, INT_MAX / 4);
        int risk = readInt("  Risk score (0-10): ", 0, 10);
        if (from == to) { std::cout << "  " << CLR_WARN << "A route must connect two different bases." << CLR_RESET << "\n"; return; }
        MapEditor::addRoute(g, from, to, dist, risk);
        std::cout << "  " << CLR_BORDER << "Route " << g.getName(from) << " <-> " << g.getName(to)
                  << " (d:" << dist << " r:" << risk << ") added." << CLR_RESET << "\n";
        changed = true;
    } else if (ch == 3) {
        if (g.empty()) { std::cout << "  " << CLR_WARN << "No bases to remove." << CLR_RESET << "\n"; return; }
        int id = readInt("  Base ID to remove (0-" + std::to_string(g.n-1) + "): ", 0, g.n - 1);
        std::string name = g.getName(id);
        if (MapEditor::removeBase(g, id)) {
            std::cout << "  " << CLR_BORDER << "Base " << name << " removed." << CLR_RESET << "\n";
            changed = true;
        } else {
            std::cout << "  " << CLR_WARN << "Could not remove that base." << CLR_RESET << "\n";
        }
    } else if (ch == 4) {
        if (g.edgeList.empty()) { std::cout << "  " << CLR_WARN << "No routes to remove." << CLR_RESET << "\n"; return; }
        int from = readInt("  From base ID (0-" + std::to_string(g.n-1) + "): ", 0, g.n - 1);
        int to   = readInt("  To base ID (0-"   + std::to_string(g.n-1) + "): ", 0, g.n - 1);
        if (MapEditor::removeRoute(g, from, to)) {
            std::cout << "  " << CLR_BORDER << "Route " << g.getName(from) << " <-> " << g.getName(to)
                      << " removed." << CLR_RESET << "\n";
            changed = true;
        } else {
            std::cout << "  " << CLR_WARN << "No direct route exists between those bases." << CLR_RESET << "\n";
        }
    }

    if (changed) {
        FileLoader::save("map.txt", g);
        std::cout << "  " << CLR_RESULT
                  << "  Network updated and saved to map.txt."
                  << CLR_RESET << "\n";
    }
    std::cin.get();
}

// -- Module 9: Mission Log -------------------------------------------------------

static void menuMissionLog() {
    std::cout << "\n  " << CLR_BORDER << "+-------------------------------------+" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|           Mission Log               |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "+-------------------------------------+" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[1]" << CLR_RESET << CLR_BORDER << " View last 20 entries           |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[2]" << CLR_RESET << CLR_BORDER << " Export full log to report file |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[0]" << CLR_RESET << CLR_BORDER << " Back                           |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "+-------------------------------------+" << CLR_RESET << "\n";

    int ch = readInt("  Choice: ", 0, 2);
    if (ch == 0) return;

    if (ch == 1) {
        MissionLog::viewLast(20);
    } else if (ch == 2) {
        std::string fname = readString("  Report filename [mission_report.txt]: ");
        if (fname.empty()) fname = "mission_report.txt";
        MissionLog::exportReport(fname);
    }
    std::cin.get();
}

// -- Visualize -----------------------------------------------------------------

static void menuVisualize(const Graph& g) {
    if (g.empty()) { std::cout << "  " << CLR_WARN << "No network loaded." << CLR_RESET << "\n"; return; }

    std::cout << "\n  " << CLR_BORDER << "+-------------------------------------+" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|        Network Visualization        |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "+-------------------------------------+" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[1]" << CLR_RESET << CLR_BORDER << " ASCII Network Map              |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[2]" << CLR_RESET << CLR_BORDER << " Adjacency List                 |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[3]" << CLR_RESET << CLR_BORDER << " Adjacency Matrix               |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[4]" << CLR_RESET << CLR_BORDER << " Network Statistics             |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[5]" << CLR_RESET << CLR_BORDER << " Show All                       |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[0]" << CLR_RESET << CLR_BORDER << " Back                           |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "+-------------------------------------+" << CLR_RESET << "\n";

    int ch = readInt("  Choice: ", 0, 5);
    switch (ch) {
        case 1: Visualization::printASCIIMap(g);    break;
        case 2: Visualization::printNetwork(g);     break;
        case 3: Visualization::printAdjMatrix(g);   break;
        case 4: Visualization::printStats(g);       break;
        case 5:
            Visualization::printASCIIMap(g);
            Visualization::printNetwork(g);
            Visualization::printAdjMatrix(g);
            Visualization::printStats(g);
            break;
        default: break;
    }
    std::cin.get();
}

// -- Load / build network ------------------------------------------------------

static void menuLoad(Graph& g) {
    std::cout << "\n  " << CLR_BORDER << "+-------------------------------------+" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|         Load Network                |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "+-------------------------------------+" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[1]" << CLR_RESET << CLR_BORDER << " Load from file (map.txt)       |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[2]" << CLR_RESET << CLR_BORDER << " Build manually                 |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[3]" << CLR_RESET << CLR_BORDER << " Save current network to file   |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[0]" << CLR_RESET << CLR_BORDER << " Back                           |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "+-------------------------------------+" << CLR_RESET << "\n";

    int ch = readInt("  Choice: ", 0, 3);

    if (ch == 1) {
        std::string fname = readString("  Filename [map.txt]: ");
        if (fname.empty()) fname = "map.txt";
        FileLoader::load(fname, g);
    } else if (ch == 2) {
        int n    = readInt("  Number of bases: ", 1, 200);
        int dirI = readInt("  Directed? (1=yes, 0=no): ", 0, 1);
        g.reset(n, dirI == 1);
        for (int i = 0; i < n; i++) {
            std::cout << "  Name for base [" << i << "]: ";
            std::string name;
            std::getline(std::cin, name);
            if (name.empty()) name = "Base_" + std::to_string(i);
            g.setName(i, name);
        }
        int m = readInt("  Number of edges to add: ", 0, n * n);
        for (int i = 0; i < m; i++) {
            std::cout << "  Edge " << (i+1) << " - from to weight risk(0-10): ";
            int u, v, w, risk;
            std::cin >> u >> v >> w >> risk;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            risk = std::max(0, std::min(10, risk));
            g.addEdge(u, v, w, risk, !g.directed);
        }
        std::cout << "  Network built with " << g.n << " bases and "
                  << g.edgeList.size() << " routes.\n";
    } else if (ch == 3) {
        if (g.empty()) { std::cout << "  No network to save.\n"; return; }
        std::string fname = readString("  Filename [network_out.txt]: ");
        if (fname.empty()) fname = "network_out.txt";
        FileLoader::save(fname, g);
    }
    std::cin.get();
}

// -- Module 13: Strongly Connected Components ----------------------------------

static void menuSCC(const Graph& g) {
    if (g.empty()) { std::cout << "  " << CLR_WARN << "No network loaded." << CLR_RESET << "\n"; return; }

    std::cout << "\n  " << CLR_BORDER << "+-------------------------------------+" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|  Strongly Connected Components      |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "+-------------------------------------+" << CLR_RESET << "\n";
    std::cout << "  " << (g.directed ? CLR_RESULT : CLR_WARN)
              << "  Network type: " << (g.directed ? "DIRECTED (Tarjan's SCC)" : "UNDIRECTED (Connected Components)")
              << CLR_RESET << "\n";

    auto t0 = Clock::now();
    SCCResult res = SCC::analyze(g);
    long long ms = elapsedMs(t0);
    SCC::printResult(res, g);
    SCC::printComplexity(g.n, static_cast<int>(g.edgeList.size()), ms);
    MissionLog::record("SCC-Tarjan",
                       "graph(" + std::to_string(g.n) + " bases, " +
                       (g.directed ? "directed" : "undirected") + ")",
                       std::to_string(res.numComponents) + " component(s)", ms);
    std::cin.get();
}

// -- Module 14: Network Centrality Analysis ------------------------------------

static void menuCentrality(const Graph& g) {
    if (g.empty()) { std::cout << "  " << CLR_WARN << "No network loaded." << CLR_RESET << "\n"; return; }

    std::cout << "\n  " << CLR_BORDER << "+-------------------------------------+" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|     Network Centrality Analysis     |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "+-------------------------------------+" << CLR_RESET << "\n";
    std::cout << "  " << CLR_RESULT << "  Analysing strategic importance of each base...\n" << CLR_RESET;

    auto t0 = Clock::now();
    CentralityResult res = Centrality::analyze(g);
    long long ms = elapsedMs(t0);
    Centrality::printResult(res, g);
    Centrality::printComplexity(g.n, static_cast<int>(g.edgeList.size()), ms);

    // Log the top base
    int top = 0;
    for (int i = 1; i < g.n; i++)
        if (res.composite[i] > res.composite[top]) top = i;
    MissionLog::record("Centrality",
                       "graph(" + std::to_string(g.n) + " bases)",
                       "top=[" + std::to_string(top) + "] " + g.getName(top), ms);
    std::cin.get();
}

// -- Module 15: Algorithm Benchmark --------------------------------------------

static void menuBenchmark() {
    std::cout << "\n  " << CLR_BORDER << "+-------------------------------------+" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|       Algorithm Benchmark           |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|  Empirical vs Theoretical Runtimes  |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "+-------------------------------------+" << CLR_RESET << "\n";
    std::cout << "  " << CLR_RESULT
              << "  Running all algorithms on random graphs (n=10,50,100,200)...\n"
              << CLR_RESET;
    Benchmark::runAll();
    std::cin.get();
}

// -- Module 12: Run All Algorithms + Summary -----------------------------------

struct SummaryRow {
    std::string name;
    std::string result;
    int V, E;
    long long ms;
};

static void menuRunAll(const Graph& g) {
    if (g.empty()) { std::cout << "  " << CLR_WARN << "No network loaded." << CLR_RESET << "\n"; return; }

    int V = g.n;
    int E = static_cast<int>(g.edgeList.size());

    std::vector<SummaryRow> summary;

    // ── 1. MST Kruskal ──────────────────────────────────────────────────────
    std::cout << "\n  " << CLR_OPTION
              << ">>> [1/6] Running Kruskal MST..." << CLR_RESET << "\n";
    {
        auto t0 = Clock::now();
        MSTResult kr = MST::kruskal(g);
        long long ms = elapsedMs(t0);
        MST::printResult(kr, g, "Kruskal");
        MST::printASCIITree(kr, g, 0);
        MST::printComplexity("Kruskal", V, E, ms);
        std::string r = kr.valid ? "weight=" + std::to_string(kr.totalWeight) : "disconnected";
        summary.push_back({"Kruskal MST", r, V, E, ms});
    }

    // ── 2. BFS ──────────────────────────────────────────────────────────────
    std::cout << "\n  " << CLR_OPTION
              << ">>> [2/6] Running BFS from node 0..." << CLR_RESET << "\n";
    {
        std::vector<int> parent(V, -1);
        auto t0 = Clock::now();
        auto order = Traversal::bfs(g, 0, parent);
        long long ms = elapsedMs(t0);
        Traversal::printResult(order, g, "BFS", 0);
        Traversal::printASCIITree(parent, g, 0);
        Traversal::printComplexity("BFS", V, E, ms);
        summary.push_back({"BFS (from [0])", std::to_string(order.size()) + " visited", V, E, ms});
    }

    // ── 3. DFS ──────────────────────────────────────────────────────────────
    std::cout << "\n  " << CLR_OPTION
              << ">>> [3/6] Running DFS from node 0..." << CLR_RESET << "\n";
    {
        std::vector<int> parent(V, -1);
        auto t0 = Clock::now();
        auto order = Traversal::dfs(g, 0, parent);
        long long ms = elapsedMs(t0);
        Traversal::printResult(order, g, "DFS", 0);
        Traversal::printASCIITree(parent, g, 0);
        Traversal::printComplexity("DFS", V, E, ms);
        summary.push_back({"DFS (from [0])", std::to_string(order.size()) + " visited", V, E, ms});
    }

    // ── 4. Dijkstra ─────────────────────────────────────────────────────────
    std::cout << "\n  " << CLR_OPTION
              << ">>> [4/6] Running Dijkstra from node 0..." << CLR_RESET << "\n";
    {
        auto t0 = Clock::now();
        auto res = ShortestPath::dijkstra(g, 0);
        long long ms = elapsedMs(t0);
        ShortestPath::printSP(res, g, 0, -1);
        ShortestPath::printComplexity("Dijkstra", V, E, ms);
        int reachable = 0;
        for (int i = 1; i < V; i++) if (res.dist[i] < INF) reachable++;
        summary.push_back({"Dijkstra src=0",
                           std::to_string(reachable) + " paths", V, E, ms});
    }

    // ── 5. Max Flow ─────────────────────────────────────────────────────────
    int sink = g.n - 1;
    std::cout << "\n  " << CLR_OPTION
              << ">>> [5/6] Running Max Flow (0 -> " << sink << ")..." << CLR_RESET << "\n";
    {
        auto t0 = Clock::now();
        auto res = MaxFlow::edmondsKarp(g, 0, sink);
        long long ms = elapsedMs(t0);
        MaxFlow::printResult(res, g, 0, sink);
        MaxFlow::printComplexity(V, E, ms);
        summary.push_back({"MaxFlow 0->" + std::to_string(sink),
                           "flow=" + std::to_string(res.maxFlow), V, E, ms});
    }

    // ── 6. TSP Nearest Neighbor ─────────────────────────────────────────────
    std::cout << "\n  " << CLR_OPTION
              << ">>> [6/6] Running TSP Nearest Neighbor from node 0..." << CLR_RESET << "\n";
    {
        auto t0 = Clock::now();
        auto res = TSP::nearestNeighbor(g, 0);
        long long ms = elapsedMs(t0);
        TSP::printResult(res, g, "Nearest Neighbor");
        TSP::printComplexity("Nearest Neighbor", V, ms);
        std::string r = res.found ? "cost=" + std::to_string(res.cost) : "no circuit";
        summary.push_back({"TSP NearNeighbor", r, V, E, ms});
    }

    // ── Summary Table ────────────────────────────────────────────────────────
    std::cout << "\n\n";
    std::cout << "  " << CLR_BORDER
              << "+==================+================+=====+=====+========+"
              << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER
              << "|  RUN ALL - ALGORITHM SUMMARY                          |"
              << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER
              << "+==================+================+=====+=====+========+"
              << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "| " << CLR_RESULT
              << std::left << std::setw(16) << "Algorithm"
              << CLR_BORDER << " | " << CLR_RESULT
              << std::setw(14) << "Result"
              << CLR_BORDER << " | " << CLR_RESULT
              << std::right << std::setw(3) << "V"
              << CLR_BORDER << " | " << CLR_RESULT
              << std::setw(3) << "E"
              << CLR_BORDER << " | " << CLR_RESULT
              << std::setw(6) << "ms"
              << CLR_BORDER << " |" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER
              << "+==================+================+=====+=====+========+"
              << CLR_RESET << "\n";

    for (const auto& row : summary) {
        std::string name14 = row.name.size() > 16 ? row.name.substr(0, 16) : row.name;
        std::string res14  = row.result.size() > 14 ? row.result.substr(0, 14) : row.result;
        std::cout << "  " << CLR_BORDER << "| " << CLR_RESULT
                  << std::left << std::setw(16) << name14
                  << CLR_BORDER << " | " << CLR_RESULT
                  << std::setw(14) << res14
                  << CLR_BORDER << " | " << CLR_RESULT
                  << std::right << std::setw(3) << row.V
                  << CLR_BORDER << " | " << CLR_RESULT
                  << std::setw(3) << row.E
                  << CLR_BORDER << " | " << CLR_RESULT
                  << std::setw(6) << row.ms
                  << CLR_BORDER << " |" << CLR_RESET << "\n";
    }
    std::cout << "  " << CLR_BORDER
              << "+==================+================+=====+=====+========+"
              << CLR_RESET << "\n";
    std::cin.get();
}

// -----------------------------------------------------------------------------
//  Main
// -----------------------------------------------------------------------------

int main() {
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hConsole, &mode);
    SetConsoleMode(hConsole, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif
    system("chcp 65001 > nul");
    std::cout.setf(std::ios_base::unitbuf);

    Graph g;
    Visualization::printBanner();

    // Auto-load map.txt if present
    {
        std::ifstream probe("map.txt");
        if (probe.good()) {
            probe.close();
            std::cout << "\n  Auto-loading map.txt...\n";
            FileLoader::load("map.txt", g);
        }
    }

    while (true) {
        std::cout << "\n";
        std::cout << "  " << CLR_BORDER << "+============================================+" << CLR_RESET << "\n";
        std::cout << "  " << CLR_BORDER << "|       MAIN COMMAND CENTER                  |" << CLR_RESET << "\n";
        std::cout << "  " << CLR_BORDER << "+============================================+" << CLR_RESET << "\n";
        if (!g.empty())
            std::cout << "  " << CLR_BORDER << "|  Network: " << CLR_RESET
                      << std::left << std::setw(31)
                      << (std::to_string(g.n) + " bases, " + std::to_string(g.edgeList.size()) + " routes")
                      << CLR_BORDER << "|" << CLR_RESET << "\n" << std::right;
        std::cout << "  " << CLR_BORDER << "+============================================+" << CLR_RESET << "\n";
        std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[1]" << CLR_RESET << CLR_BORDER << "  Load / Build Network               |" << CLR_RESET << "\n";
        std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[2]" << CLR_RESET << CLR_BORDER << "  Visualize Network                  |" << CLR_RESET << "\n";
        std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[3]" << CLR_RESET << CLR_BORDER << "  Minimum Spanning Tree (MST)        |" << CLR_RESET << "\n";
        std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[4]" << CLR_RESET << CLR_BORDER << "  Network Traversal (BFS / DFS)      |" << CLR_RESET << "\n";
        std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[5]" << CLR_RESET << CLR_BORDER << "  Shortest Path Analysis             |" << CLR_RESET << "\n";
        std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[6]" << CLR_RESET << CLR_BORDER << "  Maximum Flow Analysis              |" << CLR_RESET << "\n";
        std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[7]" << CLR_RESET << CLR_BORDER << "  TSP Route Optimization             |" << CLR_RESET << "\n";
        std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[8]" << CLR_RESET << CLR_BORDER << "  Network Resilience Analyzer        |" << CLR_RESET << "\n";
        std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[9]" << CLR_RESET << CLR_BORDER << "  Mission Planner (TSP + Budget)     |" << CLR_RESET << "\n";
        std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[10]" << CLR_RESET << CLR_BORDER << " Dynamic Map Editor                 |" << CLR_RESET << "\n";
        std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[11]" << CLR_RESET << CLR_BORDER << " Mission Log (view / export)        |" << CLR_RESET << "\n";
        std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[12]" << CLR_RESET << CLR_BORDER << " Run All Algorithms + Summary       |" << CLR_RESET << "\n";
        std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[13]" << CLR_RESET << CLR_BORDER << " Strongly Connected Components      |" << CLR_RESET << "\n";
        std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[14]" << CLR_RESET << CLR_BORDER << " Network Centrality Analysis        |" << CLR_RESET << "\n";
        std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[15]" << CLR_RESET << CLR_BORDER << " Algorithm Benchmark                |" << CLR_RESET << "\n";
        std::cout << "  " << CLR_BORDER << "|  " << CLR_OPTION << "[0]" << CLR_RESET << CLR_BORDER << "  Exit                               |" << CLR_RESET << "\n";
        std::cout << "  " << CLR_BORDER << "+============================================+" << CLR_RESET << "\n";

        int choice = readInt("  Command: ", 0, 15);
        if (std::cin.eof()) {
            std::cout << "\n  " << CLR_RESULT << "[EOF - session ended]" << CLR_RESET << "\n\n";
            return 0;
        }
        std::cout << "\n";

        switch (choice) {
            case 0:
                std::cout << "  " << CLR_RESULT << "[Tactical Network Optimizer - session ended]" << CLR_RESET << "\n\n";
                return 0;
            case 1:  menuLoad(g);              break;
            case 2:  menuVisualize(g);         break;
            case 3:  menuMST(g);               break;
            case 4:  menuTraversal(g);         break;
            case 5:  menuShortestPath(g);      break;
            case 6:  menuMaxFlow(g);           break;
            case 7:  menuTSP(g);               break;
            case 8:  menuResilience(g);        break;
            case 9:  menuMissionPlanner(g);    break;
            case 10: menuMapEditor(g);         break;
            case 11: menuMissionLog();         break;
            case 12: menuRunAll(g);            break;
            case 13: menuSCC(g);               break;
            case 14: menuCentrality(g);        break;
            case 15: menuBenchmark();          break;
        }
        pauseMenu();
    }
}
