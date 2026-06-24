#pragma once
#include "Graph.h"
#include <vector>

struct SCCResult {
    std::vector<int>              comp;        // comp[v] = component id (0-based)
    int                           numComponents = 0;
    std::vector<std::vector<int>> components;  // components[id] = vertex list
};

// -----------------------------------------------------------------------------
//  SCC - Strongly Connected Components
//    Directed graphs  : Tarjan's algorithm  O(V + E)
//    Undirected graphs: BFS connected-component labelling (every connected
//                       component is trivially strongly connected)
//
//  Military application: identifies isolated sub-networks — bases that cannot
//  communicate with HQ even through relay chains in a directed comm network.
// -----------------------------------------------------------------------------
class SCC {
public:
    static SCCResult analyze(const Graph& g);
    static void printResult(const SCCResult& res, const Graph& g);
    static void printComplexity(int V, int E, long long ms);

private:
    // Tarjan's recursive DFS (iterative to avoid stack overflow on large graphs)
    static SCCResult tarjan(const Graph& g);
    // BFS flood-fill for undirected graphs
    static SCCResult connectedComponents(const Graph& g);
};
