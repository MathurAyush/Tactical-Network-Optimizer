#include "Graph.h"
#include <stdexcept>
#include <algorithm>

// -- Constructors --------------------------------------------------------------

Graph::Graph() : n(0), directed(false) {}

Graph::Graph(int n, bool directed) : n(n), directed(directed) {
    names.resize(n);
    adj.resize(n);
    coordX.assign(n, 0.0);
    coordY.assign(n, 0.0);
    for (int i = 0; i < n; i++)
        names[i] = "Base_" + std::to_string(i);
}

// -- reset ---------------------------------------------------------------------

void Graph::reset(int newN, bool dir) {
    n        = newN;
    directed = dir;
    names.assign(n, "");
    adj.assign(n, {});
    edgeList.clear();
    coordX.assign(n, 0.0);
    coordY.assign(n, 0.0);
}

// -- addEdge -------------------------------------------------------------------

void Graph::addEdge(int u, int v, int w, int risk, bool bidirectional) {
    if (u < 0 || u >= n || v < 0 || v >= n)
        return; // silently ignore out-of-range

    adj[u].push_back({v, w, risk});
    if (bidirectional)
        adj[v].push_back({u, w, risk});

    // Store one canonical entry for algorithms that iterate over edges
    edgeList.push_back({u, v, w, risk});
}

// -- removeEdge ----------------------------------------------------------------

void Graph::removeEdge(int u, int v) {
    auto stripFrom = [&](int x, int y) {
        auto& list = adj[x];
        list.erase(std::remove_if(list.begin(), list.end(),
                                  [&](const AdjEdge& e){ return e.to == y; }),
                   list.end());
    };
    stripFrom(u, v);
    stripFrom(v, u);

    edgeList.erase(std::remove_if(edgeList.begin(), edgeList.end(),
                                  [&](const Edge& e){
                                      return (e.from == u && e.to == v) ||
                                             (e.from == v && e.to == u);
                                  }),
                   edgeList.end());
}

// -- Names ---------------------------------------------------------------------

void Graph::setName(int id, const std::string& name) {
    if (id >= 0 && id < n)
        names[id] = name;
}

void Graph::setCoords(int id, double x, double y) {
    if (id >= 0 && id < n) {
        coordX[id] = x;
        coordY[id] = y;
    }
}

std::string Graph::getName(int id) const {
    if (id >= 0 && id < n)
        return names[id];
    return "Unknown";
}

double Graph::getX(int id) const { return (id >= 0 && id < n) ? coordX[id] : 0.0; }
double Graph::getY(int id) const { return (id >= 0 && id < n) ? coordY[id] : 0.0; }

bool Graph::hasCoords() const {
    for (int i = 0; i < n; i++)
        if (coordX[i] != 0.0 || coordY[i] != 0.0) return true;
    return false;
}

// -- Queries -------------------------------------------------------------------

int  Graph::size()  const { return n; }
bool Graph::empty() const { return n == 0; }

std::vector<std::vector<int>> Graph::getAdjMatrix() const {
    std::vector<std::vector<int>> mat(n, std::vector<int>(n, INF));
    for (int i = 0; i < n; i++)
        mat[i][i] = 0;
    for (int u = 0; u < n; u++)
        for (const auto& e : adj[u])
            if (e.weight < mat[u][e.to])
                mat[u][e.to] = e.weight;
    return mat;
}

int Graph::getRisk(int u, int v) const {
    if (u < 0 || u >= n) return -1;
    for (const auto& e : adj[u])
        if (e.to == v) return e.risk;
    return -1;
}

Graph Graph::riskGraph() const {
    Graph rg(n, directed);
    rg.names  = names;
    rg.coordX = coordX;
    rg.coordY = coordY;
    for (const Edge& e : edgeList)
        rg.addEdge(e.from, e.to, e.risk, e.risk, !directed);
    return rg;
}
