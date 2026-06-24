#include "MapEditor.h"
#include <vector>

// -- Add a new base (node) -----------------------------------------------------

void MapEditor::addBase(Graph& g, const std::string& name) {
    Graph ng(g.n + 1, g.directed);
    for (int i = 0; i < g.n; i++)
        ng.names[i] = g.names[i];
    ng.names[g.n] = name;
    for (const Edge& e : g.edgeList)
        ng.addEdge(e.from, e.to, e.weight, e.risk, !g.directed);
    g = ng;
}

// -- Add a new route (edge) -----------------------------------------------------

void MapEditor::addRoute(Graph& g, int from, int to, int distance, int risk) {
    g.addEdge(from, to, distance, risk, !g.directed);
}

// -- Remove a base (re-indexes all surviving bases) -----------------------------

bool MapEditor::removeBase(Graph& g, int id) {
    if (id < 0 || id >= g.n) return false;

    int newN = g.n - 1;
    Graph ng(newN, g.directed);

    std::vector<int> remap(g.n, -1);
    int idx = 0;
    for (int i = 0; i < g.n; i++) {
        if (i == id) continue;
        remap[i] = idx;
        ng.names[idx] = g.names[i];
        idx++;
    }
    for (const Edge& e : g.edgeList) {
        if (e.from == id || e.to == id) continue;
        ng.addEdge(remap[e.from], remap[e.to], e.weight, e.risk, !g.directed);
    }
    g = ng;
    return true;
}

// -- Remove a route (edge) -------------------------------------------------------

bool MapEditor::removeRoute(Graph& g, int from, int to) {
    if (g.getRisk(from, to) < 0) return false; // no direct edge
    g.removeEdge(from, to);
    return true;
}
