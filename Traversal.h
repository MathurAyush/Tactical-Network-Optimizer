#pragma once
#include "Graph.h"
#include <vector>
#include <string>

// -----------------------------------------------------------------------------
//  Traversal - BFS and DFS on the military network
//  Returns the visit order as a list of node IDs.
//  Both algorithms handle disconnected graphs (visit all components).
// -----------------------------------------------------------------------------
class Traversal {
public:
    static std::vector<int> bfs(const Graph& g, int start = 0);
    static std::vector<int> bfs(const Graph& g, int start, std::vector<int>& parent);
    static std::vector<int> dfs(const Graph& g, int start = 0);
    static std::vector<int> dfs(const Graph& g, int start, std::vector<int>& parent);

    static void printResult(const std::vector<int>& order, const Graph& g,
                            const std::string& type, int start);
    static void printASCIITree(const std::vector<int>& parent, const Graph& g, int start);
    static void printComplexity(const std::string& type, int V, int E, long long ms);

private:
    static void dfsVisit(const Graph& g, int v,
                         std::vector<bool>& visited,
                         std::vector<int>& order);
    static void dfsVisit(const Graph& g, int v,
                         std::vector<bool>& visited,
                         std::vector<int>& order,
                         std::vector<int>& parent);
};
