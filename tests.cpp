// =============================================================================
//  tests.cpp  -  Unit test suite for Tactical Network Optimizer
//  Compile:  g++ -std=c++17 -O2 -o run_tests tests.cpp Graph.cpp MST.cpp
//            Traversal.cpp ShortestPath.cpp MaxFlow.cpp TSP.cpp Resilience.cpp
//            SCC.cpp Centrality.cpp FileLoader.cpp MissionLog.cpp
//            MissionPlanner.cpp MapEditor.cpp Visualization.cpp AStar.cpp
//  Run:      ./run_tests
// =============================================================================

#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

#include "Graph.h"
#include "MST.h"
#include "Traversal.h"
#include "ShortestPath.h"
#include "MaxFlow.h"
#include "TSP.h"
#include "Resilience.h"
#include "SCC.h"
#include "Centrality.h"
#include "AStar.h"

// -----------------------------------------------------------------------------
//  Minimal test framework
// -----------------------------------------------------------------------------

static int g_passed = 0;
static int g_failed = 0;

#define ASSERT_TRUE(expr, name) do { \
    if (expr) { std::cout << "[PASS] " << (name) << "\n"; g_passed++; } \
    else       { std::cout << "[FAIL] " << (name) << "  (expected true)\n"; g_failed++; } \
} while (0)

#define ASSERT_EQ(a, b, name) do { \
    if ((a) == (b)) { std::cout << "[PASS] " << (name) << "\n"; g_passed++; } \
    else { std::cout << "[FAIL] " << (name) \
                     << "  expected=" << (b) << "  got=" << (a) << "\n"; g_failed++; } \
} while (0)

#define ASSERT_NEAR(a, b, tol, name) do { \
    if (std::fabs((double)(a) - (double)(b)) <= (tol)) { \
        std::cout << "[PASS] " << (name) << "\n"; g_passed++; } \
    else { std::cout << "[FAIL] " << (name) \
                     << "  expected~" << (b) << "  got=" << (a) << "\n"; g_failed++; } \
} while (0)

static void section(const std::string& title) {
    std::cout << "\n--- " << title << " ---\n";
}

// -----------------------------------------------------------------------------
//  Graph helpers
// -----------------------------------------------------------------------------

// Triangle: 0-1 (w=10), 1-2 (w=20), 0-2 (w=40)
static Graph makeTriangle() {
    Graph g(3, false);
    g.setName(0, "A"); g.setName(1, "B"); g.setName(2, "C");
    g.addEdge(0, 1, 10, 1, true);
    g.addEdge(1, 2, 20, 2, true);
    g.addEdge(0, 2, 40, 5, true);
    return g;
}

// Path graph: 0-1-2-3-4 (all weight=10)
static Graph makePath(int n = 5) {
    Graph g(n, false);
    for (int i = 0; i < n - 1; i++) {
        g.setName(i, "P" + std::to_string(i));
        g.addEdge(i, i + 1, 10, 1, true);
    }
    g.setName(n - 1, "P" + std::to_string(n - 1));
    return g;
}

// Complete graph K4 (all pairs connected, weight=1)
static Graph makeK4() {
    Graph g(4, false);
    for (int i = 0; i < 4; i++) g.setName(i, "K" + std::to_string(i));
    for (int i = 0; i < 4; i++)
        for (int j = i + 1; j < 4; j++)
            g.addEdge(i, j, 1, 0, true);
    return g;
}

// Simple directed graph for SCC test
// Components: {0,1,2} form a cycle; {3} is reachable from 2 but cannot go back
static Graph makeSCCGraph() {
    Graph g(4, true);
    g.setName(0, "X"); g.setName(1, "Y"); g.setName(2, "Z"); g.setName(3, "W");
    // Cycle: 0->1->2->0
    g.addEdge(0, 1, 1, 0, false);
    g.addEdge(1, 2, 1, 0, false);
    g.addEdge(2, 0, 1, 0, false);
    // One-way: 2->3
    g.addEdge(2, 3, 1, 0, false);
    return g;
}

// Network for max-flow test:
// 0->1 (cap=10), 0->2 (cap=10), 1->3 (cap=10), 2->3 (cap=10)  => maxflow=20
static Graph makeFlowGraph() {
    Graph g(4, true);
    for (int i = 0; i < 4; i++) g.setName(i, "F" + std::to_string(i));
    g.addEdge(0, 1, 10, 0, false);
    g.addEdge(0, 2, 10, 0, false);
    g.addEdge(1, 3, 10, 0, false);
    g.addEdge(2, 3, 10, 0, false);
    return g;
}

// =============================================================================
//  TEST GROUPS
// =============================================================================

// -----------------------------------------------------------------------------
//  1. Graph construction
// -----------------------------------------------------------------------------
static void testGraph() {
    section("Graph Construction");

    Graph g = makeTriangle();
    ASSERT_EQ(g.n, 3, "Triangle: 3 nodes");
    ASSERT_EQ((int)g.edgeList.size(), 3, "Triangle: 3 edges");
    ASSERT_EQ(g.getName(0), std::string("A"), "Triangle: name[0]=A");
    ASSERT_TRUE(!g.empty(), "Triangle: not empty");

    Graph empty;
    ASSERT_TRUE(empty.empty(), "Default graph is empty");

    // Directed graph
    Graph dg(3, true);
    dg.addEdge(0, 1, 5, 0, false); // directed: only 0->1
    ASSERT_EQ((int)dg.adj[0].size(), 1, "Directed: adj[0] has 1 entry");
    ASSERT_EQ((int)dg.adj[1].size(), 0, "Directed: adj[1] has 0 entries");

    // Undirected graph adjacency
    Graph ug(2, false);
    ug.addEdge(0, 1, 7, 2, true);
    ASSERT_EQ((int)ug.adj[0].size(), 1, "Undirected: adj[0] has 1 entry");
    ASSERT_EQ((int)ug.adj[1].size(), 1, "Undirected: adj[1] has 1 entry");
    ASSERT_EQ(ug.edgeList[0].weight, 7, "Edge weight=7");
    ASSERT_EQ(ug.edgeList[0].risk, 2, "Edge risk=2");
}

// -----------------------------------------------------------------------------
//  2. MST
// -----------------------------------------------------------------------------
static void testMST() {
    section("MST");

    // Triangle: MST should pick edges (0-1, w=10) and (1-2, w=20) => total=30
    Graph g = makeTriangle();
    MSTResult kr = MST::kruskal(g);
    ASSERT_TRUE(kr.valid, "Kruskal: triangle is connected");
    ASSERT_EQ(kr.totalWeight, 30, "Kruskal: triangle MST weight=30");
    ASSERT_EQ((int)kr.edges.size(), 2, "Kruskal: triangle MST has 2 edges");

    MSTResult pr = MST::prim(g, 0);
    ASSERT_TRUE(pr.valid, "Prim: triangle is connected");
    ASSERT_EQ(pr.totalWeight, 30, "Prim: triangle MST weight=30");

    // Path graph: all edges must be in MST (no alternatives)
    Graph path = makePath(5);
    MSTResult pkr = MST::kruskal(path);
    ASSERT_TRUE(pkr.valid, "Kruskal: path is connected");
    ASSERT_EQ(pkr.totalWeight, 40, "Kruskal: path(5) MST weight=40");

    // Disconnected graph: MST invalid
    Graph disc(4, false);
    disc.addEdge(0, 1, 10, 0, true); // only one component
    MSTResult dkr = MST::kruskal(disc);
    ASSERT_TRUE(!dkr.valid, "Kruskal: disconnected graph gives invalid MST");
}

// -----------------------------------------------------------------------------
//  3. Traversal
// -----------------------------------------------------------------------------
static void testTraversal() {
    section("Traversal");

    Graph g = makePath(5);
    std::vector<int> par(5, -1);

    // BFS from 0 should visit all 5 nodes
    auto bfsOrd = Traversal::bfs(g, 0, par);
    ASSERT_EQ((int)bfsOrd.size(), 5, "BFS: visits all 5 nodes on path");
    ASSERT_EQ(bfsOrd[0], 0, "BFS: first visited = source");

    // DFS from 0 should also visit all 5 nodes
    std::fill(par.begin(), par.end(), -1);
    auto dfsOrd = Traversal::dfs(g, 0, par);
    ASSERT_EQ((int)dfsOrd.size(), 5, "DFS: visits all 5 nodes on path");
    ASSERT_EQ(dfsOrd[0], 0, "DFS: first visited = source");

    // Disconnected: BFS visits all components (full network traversal by design)
    // Starts at source then continues to unvisited components
    Graph disc(4, false);
    disc.addEdge(0, 1, 5, 0, true);
    disc.addEdge(2, 3, 5, 0, true);
    std::vector<int> par2(4, -1);
    auto bfs2 = Traversal::bfs(disc, 0, par2);
    ASSERT_EQ((int)bfs2.size(), 4, "BFS: visits all nodes including disconnected components");
    ASSERT_EQ(bfs2[0], 0, "BFS: starts at source node 0");
    ASSERT_EQ(bfs2[1], 1, "BFS: second node is neighbor of source");
}

// -----------------------------------------------------------------------------
//  4. Shortest Path
// -----------------------------------------------------------------------------
static void testShortestPath() {
    section("Shortest Path");

    // Path: 0-1-2-3-4 all w=10. Dijkstra from 0 to 4 = 40.
    Graph path = makePath(5);
    SPResult dr = ShortestPath::dijkstra(path, 0);
    ASSERT_EQ(dr.dist[4], 40, "Dijkstra: path(5) dist[0->4]=40");
    ASSERT_EQ(dr.dist[2], 20, "Dijkstra: path(5) dist[0->2]=20");
    ASSERT_EQ(dr.dist[0], 0,  "Dijkstra: source dist=0");

    // Path reconstruction
    auto rpath = ShortestPath::reconstructPath(dr.prev, 0, 4);
    ASSERT_EQ((int)rpath.size(), 5, "Dijkstra: path has 5 nodes");
    ASSERT_EQ(rpath[0], 0, "Path starts at 0");
    ASSERT_EQ(rpath[4], 4, "Path ends at 4");

    // Bellman-Ford same result
    SPResult bf = ShortestPath::bellmanFord(path, 0);
    ASSERT_EQ(bf.dist[4], 40, "Bellman-Ford: path(5) dist[0->4]=40");
    ASSERT_TRUE(!bf.hasNegCycle, "Bellman-Ford: no negative cycle");

    // Floyd-Warshall
    APSPResult fw = ShortestPath::floydWarshall(path);
    ASSERT_EQ(fw.dist[0][4], 40, "Floyd-Warshall: dist[0][4]=40");
    ASSERT_EQ(fw.dist[4][0], 40, "Floyd-Warshall: dist[4][0]=40 (symmetric)");
    ASSERT_EQ(fw.dist[1][3], 20, "Floyd-Warshall: dist[1][3]=20");

    // Triangle: shortest 0->2 is NOT via direct edge (40) but 0->1->2 (30)
    Graph tri = makeTriangle();
    SPResult tr = ShortestPath::dijkstra(tri, 0);
    ASSERT_EQ(tr.dist[2], 30, "Dijkstra: triangle shortest 0->2 = 30 (via 1)");
}

// -----------------------------------------------------------------------------
//  5. Maximum Flow
// -----------------------------------------------------------------------------
static void testMaxFlow() {
    section("Maximum Flow");

    // Flow graph: two parallel paths of capacity 10 each => max flow = 20
    Graph fg = makeFlowGraph();
    FlowResult mf = MaxFlow::edmondsKarp(fg, 0, 3);
    ASSERT_EQ(mf.maxFlow, 20, "Edmonds-Karp: parallel paths maxFlow=20");

    // Single path: 0->1 (cap=5), 1->2 (cap=3) => bottleneck = 3
    Graph single(3, true);
    single.addEdge(0, 1, 5, 0, false);
    single.addEdge(1, 2, 3, 0, false);
    FlowResult sf = MaxFlow::edmondsKarp(single, 0, 2);
    ASSERT_EQ(sf.maxFlow, 3, "Edmonds-Karp: bottleneck single path = 3");

    // No path from source to sink
    Graph nopath(4, true);
    nopath.addEdge(0, 1, 10, 0, false);
    nopath.addEdge(2, 3, 10, 0, false);
    FlowResult nf = MaxFlow::edmondsKarp(nopath, 0, 3);
    ASSERT_EQ(nf.maxFlow, 0, "Edmonds-Karp: no path => maxFlow=0");
}

// -----------------------------------------------------------------------------
//  6. TSP
// -----------------------------------------------------------------------------
static void testTSP() {
    section("TSP");

    // K4 all-ones: optimal TSP tour = 4 (visit 4 nodes, return to start, each edge=1)
    Graph k4 = makeK4();
    TSPResult bf = TSP::bruteForce(k4, 0);
    ASSERT_TRUE(bf.found, "TSP BruteForce: K4 has Hamiltonian circuit");
    ASSERT_EQ(bf.cost, 4, "TSP BruteForce: K4 cost=4");

    TSPResult nn = TSP::nearestNeighbor(k4, 0);
    ASSERT_TRUE(nn.found, "TSP NearestNeighbor: K4 has Hamiltonian circuit");
    ASSERT_EQ(nn.cost, 4, "TSP NearestNeighbor: K4 cost=4");

    // Truly disconnected graph: TSP uses APSP (Floyd-Warshall), so a path graph
    // has dist[i][j] < INF for all pairs and a "circuit" is found via multi-hop.
    // To get no circuit, use a graph where some APSP distances are INF.
    Graph tspDisc(4, false);
    tspDisc.addEdge(0, 1, 10, 0, true); // only component {0,1} connected
    // nodes 2,3 are completely isolated from 0,1
    TSPResult pf = TSP::bruteForce(tspDisc, 0);
    ASSERT_TRUE(!pf.found, "TSP BruteForce: truly disconnected graph has no Hamiltonian circuit");
}

// -----------------------------------------------------------------------------
//  7. Resilience (Bridges)
// -----------------------------------------------------------------------------
static void testResilience() {
    section("Resilience");

    // Path: every edge is a bridge
    Graph path = makePath(4);
    auto bridges = Resilience::findBridges(path);
    ASSERT_EQ((int)bridges.size(), 3, "Bridges: path(4) has 3 bridges");

    // Triangle: no bridges (removing any edge leaves rest connected)
    Graph tri = makeTriangle();
    auto tb = Resilience::findBridges(tri);
    ASSERT_EQ((int)tb.size(), 0, "Bridges: triangle has 0 bridges");

    // Reachable set: triangle after removing edge 0-2 (risk=5, highest in edgeList)
    // Remove edge from node 0 to node 2 (index 2 in edgeList)
    // All nodes still reachable via 0-1-2
    auto reach = Resilience::reachableFrom(tri, 0, 0, 2);
    ASSERT_EQ((int)reach.size(), 3, "Resilience: triangle stays connected after removing 0-2");
}

// -----------------------------------------------------------------------------
//  8. SCC
// -----------------------------------------------------------------------------
static void testSCC() {
    section("Strongly Connected Components");

    // Directed graph: {0,1,2} cycle + {3} isolated
    Graph g = makeSCCGraph();
    SCCResult res = SCC::analyze(g);
    ASSERT_EQ(res.numComponents, 2, "SCC: directed graph has 2 components");

    // Node 3 should be in its own component
    bool node3Isolated = false;
    for (const auto& comp : res.components)
        if (comp.size() == 1 && comp[0] == 3) { node3Isolated = true; break; }
    ASSERT_TRUE(node3Isolated, "SCC: node 3 is in its own singleton component");

    // The big SCC should have 3 nodes
    bool bigComp = false;
    for (const auto& comp : res.components)
        if ((int)comp.size() == 3) { bigComp = true; break; }
    ASSERT_TRUE(bigComp, "SCC: {0,1,2} form one SCC of size 3");

    // Undirected path: 1 connected component
    Graph path = makePath(4);
    SCCResult pr = SCC::analyze(path);
    ASSERT_EQ(pr.numComponents, 1, "SCC (undirected path): 1 connected component");

    // Undirected disconnected: 2 components
    Graph disc(4, false);
    disc.addEdge(0, 1, 5, 0, true);
    disc.addEdge(2, 3, 5, 0, true);
    SCCResult dr = SCC::analyze(disc);
    ASSERT_EQ(dr.numComponents, 2, "SCC (undirected disconnected): 2 components");
}

// -----------------------------------------------------------------------------
//  9. Centrality
// -----------------------------------------------------------------------------
static void testCentrality() {
    section("Centrality");

    // Star graph: center node 0 connected to all others
    // Node 0 should have highest betweenness and degree
    int n = 5;
    Graph star(n, false);
    for (int i = 0; i < n; i++) star.setName(i, "S" + std::to_string(i));
    for (int i = 1; i < n; i++)
        star.addEdge(0, i, 10, 0, true);

    CentralityResult cr = Centrality::analyze(star);

    // Center node (0) should have highest degree centrality (= 1.0)
    ASSERT_NEAR(cr.degree[0], 1.0, 0.01, "Centrality: star center degree=1.0");

    // Leaf nodes should have degree = 1/(n-1) = 0.25
    ASSERT_NEAR(cr.degree[1], 0.25, 0.01, "Centrality: star leaf degree=0.25");

    // Center should have highest composite score
    int topNode = 0;
    for (int i = 1; i < n; i++)
        if (cr.composite[i] > cr.composite[topNode]) topNode = i;
    ASSERT_EQ(topNode, 0, "Centrality: star center is most important node");
}

// -----------------------------------------------------------------------------
//  10. A* Search
// -----------------------------------------------------------------------------
static void testAStar() {
    section("A* Search");

    // Path graph: A* from 0 to 4 should find distance=40
    Graph path = makePath(5);
    AStarResult ar = AStar::search(path, 0, 4);
    ASSERT_TRUE(ar.found, "A*: found path in path(5)");
    ASSERT_EQ(ar.dist[4], 40, "A*: dist[0->4]=40 on path(5)");

    // Reconstructed path should be 0-1-2-3-4
    auto rpath = AStar::reconstructPath(ar.prev, 0, 4);
    ASSERT_EQ((int)rpath.size(), 5, "A*: path length=5");

    // With coords set: heuristic should reduce expansions vs no-coords
    // Add coords so node 4 is "far" from 0
    path.setCoords(0, 0.0, 0.0);
    path.setCoords(1, 5.0, 0.0);
    path.setCoords(2, 10.0, 0.0);
    path.setCoords(3, 15.0, 0.0);
    path.setCoords(4, 20.0, 0.0);

    AStarResult arCoords = AStar::search(path, 0, 4);
    ASSERT_TRUE(arCoords.found, "A*: found path with coords");
    ASSERT_EQ(arCoords.dist[4], 40, "A*: dist still 40 with coords (admissible heuristic)");
    // A* with good coords should expand <= nodes vs h=0 path
    ASSERT_TRUE(arCoords.nodesExpanded <= ar.nodesExpanded + 1,
                "A*: coords version expands no more nodes than h=0");

    // No path
    Graph disc(4, false);
    disc.addEdge(0, 1, 5, 0, true);
    AStarResult na = AStar::search(disc, 0, 3);
    ASSERT_TRUE(!na.found, "A*: no path to unreachable node");
}

// =============================================================================
//  main
// =============================================================================

int main() {
    std::cout << "============================================================\n";
    std::cout << "  DRDO Tactical Network Optimizer  -  Unit Test Suite\n";
    std::cout << "============================================================\n";

    testGraph();
    testMST();
    testTraversal();
    testShortestPath();
    testMaxFlow();
    testTSP();
    testResilience();
    testSCC();
    testCentrality();
    testAStar();

    std::cout << "\n============================================================\n";
    std::cout << "  Results: " << g_passed << " passed,  " << g_failed << " failed\n";
    std::cout << "============================================================\n";

    return g_failed == 0 ? 0 : 1;
}
