#pragma once
#include <vector>
#include <string>
#include <climits>

// Sentinel for "no connection / infinity"
static const int INF = INT_MAX / 2;

// A single directed edge (from -> to with given weight and risk score 0-10)
struct Edge {
    int from, to, weight;
    int risk = 0;       // threat level along this route, 0 (safe) - 10 (danger)
};

// One adjacency-list entry: destination, transit weight (distance), and risk
struct AdjEdge {
    int to;
    int weight;
    int risk;
};

// -----------------------------------------------------------------------------
//  Graph
//  Central data structure for the entire optimizer.
//  Stores an adjacency list AND a unique edge list (used by Kruskal / flow).
//  When addEdge() is called with bidirectional=true the adjacency list gets
//  both directions, but the edgeList stores only one canonical entry so
//  Kruskal doesn't double-count.
// -----------------------------------------------------------------------------
class Graph {
public:
    int  n;           // number of vertices (military bases)
    bool directed;    // true -> directed arcs, false -> undirected roads

    std::vector<std::string>               names;   // base names indexed by id
    std::vector<std::vector<AdjEdge>>      adj;     // adj[u] = {(v, weight, risk), ...}
    std::vector<Edge>                      edgeList; // one entry per logical edge

    // Optional spatial coordinates for A* heuristic (0,0 if not set)
    std::vector<double> coordX, coordY;

    // -- Construction ---------------------------------------------------------
    Graph();
    Graph(int n, bool directed = false);

    // Re-initialise to a fresh graph of size n
    void reset(int n, bool directed = false);

    // -- Mutation --------------------------------------------------------------
    // bidirectional=true  -> undirected: both adj directions, one edgeList entry
    // bidirectional=false -> directed:   one adj direction,   one edgeList entry
    void addEdge(int u, int v, int w, int risk = 0, bool bidirectional = true);

    // Remove every edge between u and v (both directions if undirected)
    void removeEdge(int u, int v);

    void setName(int id, const std::string& name);
    void setCoords(int id, double x, double y);

    // -- Query -----------------------------------------------------------------
    std::string getName(int id) const;
    double      getX(int id)    const;
    double      getY(int id)    const;
    bool        hasCoords()     const; // true if any node has non-zero coords
    int  size()  const;
    bool empty() const;

    // Returns nxn adjacency matrix (INF where no direct edge)
    std::vector<std::vector<int>> getAdjMatrix() const;

    // Look up the risk score of the direct edge u->v (-1 if no direct edge)
    int getRisk(int u, int v) const;

    // Returns a copy of this graph where every edge weight is replaced by
    // its risk score - lets the existing distance-based algorithms (e.g.
    // Dijkstra) be reused to compute "safest" routes that minimise risk.
    Graph riskGraph() const;
};
