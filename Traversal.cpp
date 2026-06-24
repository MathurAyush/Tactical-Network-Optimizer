#include "Traversal.h"
#include "Colors.h"
#include "RiskUtil.h"
#include <queue>
#include <iostream>
#include <iomanip>
#include <functional>

// -----------------------------------------------------------------------------
//  BFS - without parent tracking (backward-compat)
// -----------------------------------------------------------------------------

std::vector<int> Traversal::bfs(const Graph& g, int start) {
    std::vector<int> dummy(g.size(), -1);
    return bfs(g, start, dummy);
}

// -----------------------------------------------------------------------------
//  BFS - with parent tracking
// -----------------------------------------------------------------------------

std::vector<int> Traversal::bfs(const Graph& g, int start, std::vector<int>& parent) {
    int n = g.size();
    std::vector<bool> visited(n, false);
    std::vector<int>  order;
    order.reserve(n);
    parent.assign(n, -1);

    auto runBFS = [&](int src) {
        std::queue<int> q;
        visited[src] = true;
        q.push(src);
        while (!q.empty()) {
            int u = q.front(); q.pop();
            order.push_back(u);
            for (const auto& e : g.adj[u]) {
                int v = e.to;
                if (!visited[v]) {
                    visited[v]  = true;
                    parent[v]   = u;
                    q.push(v);
                }
            }
        }
    };

    runBFS(start);
    for (int i = 0; i < n; i++)
        if (!visited[i]) runBFS(i);

    return order;
}

// -----------------------------------------------------------------------------
//  DFS - recursive helpers (with and without parent)
// -----------------------------------------------------------------------------

void Traversal::dfsVisit(const Graph& g, int v,
                          std::vector<bool>& visited,
                          std::vector<int>& order) {
    visited[v] = true;
    order.push_back(v);
    for (const auto& e : g.adj[v])
        if (!visited[e.to])
            dfsVisit(g, e.to, visited, order);
}

void Traversal::dfsVisit(const Graph& g, int v,
                          std::vector<bool>& visited,
                          std::vector<int>& order,
                          std::vector<int>& parent) {
    visited[v] = true;
    order.push_back(v);
    for (const auto& e : g.adj[v]) {
        if (!visited[e.to]) {
            parent[e.to] = v;
            dfsVisit(g, e.to, visited, order, parent);
        }
    }
}

// -----------------------------------------------------------------------------
//  DFS - without parent (backward-compat)
// -----------------------------------------------------------------------------

std::vector<int> Traversal::dfs(const Graph& g, int start) {
    int n = g.size();
    std::vector<bool> visited(n, false);
    std::vector<int>  order;
    order.reserve(n);
    dfsVisit(g, start, visited, order);
    for (int i = 0; i < n; i++)
        if (!visited[i]) dfsVisit(g, i, visited, order);
    return order;
}

// -----------------------------------------------------------------------------
//  DFS - with parent tracking
// -----------------------------------------------------------------------------

std::vector<int> Traversal::dfs(const Graph& g, int start, std::vector<int>& parent) {
    int n = g.size();
    std::vector<bool> visited(n, false);
    std::vector<int>  order;
    order.reserve(n);
    parent.assign(n, -1);
    dfsVisit(g, start, visited, order, parent);
    for (int i = 0; i < n; i++)
        if (!visited[i]) dfsVisit(g, i, visited, order, parent);
    return order;
}

// -----------------------------------------------------------------------------
//  Output - result table (consistent with MST style)
// -----------------------------------------------------------------------------

void Traversal::printResult(const std::vector<int>& order, const Graph& g,
                             const std::string& type, int start) {
    std::cout << "\n  " << CLR_RESULT << "[" << type << " Traversal from "
              << g.getName(start) << " (ID " << start << ")]" << CLR_RESET << "\n";
    RiskUtil::printLegend();
    std::cout << "  " << CLR_BORDER << std::string(62, '-') << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << std::left
              << "  " << std::setw(5) << "Step"
              << std::setw(6) << "ID"
              << std::setw(24) << "Base Name"
              << "Connections"
              << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << std::string(62, '-') << CLR_RESET << "\n";

    for (int i = 0; i < static_cast<int>(order.size()); i++) {
        int v = order[i];
        // Show neighbour count with risk colour of highest-risk edge
        int maxRisk = 0;
        for (const auto& e : g.adj[v])
            if (e.risk > maxRisk) maxRisk = e.risk;
        const char* rc = g.adj[v].empty() ? CLR_RESULT : RiskUtil::color(maxRisk);

        std::cout << "  " << CLR_RESULT << std::right
                  << "  " << std::setw(4) << (i + 1) << "   "
                  << "[" << std::setw(2) << v << "] "
                  << std::left << std::setw(24) << g.getName(v) << std::right
                  << rc << g.adj[v].size() << " route(s)  "
                  << RiskUtil::label(maxRisk) << CLR_RESET << "\n";
    }
    std::cout << "  " << CLR_BORDER << std::string(62, '-') << CLR_RESET << "\n";
    std::cout << "  " << CLR_RESULT << "  Total bases visited: " << order.size() << CLR_RESET << "\n";
}

// -----------------------------------------------------------------------------
//  ASCII traversal tree
// -----------------------------------------------------------------------------

void Traversal::printASCIITree(const std::vector<int>& parent, const Graph& g, int start) {
    int n = g.size();

    // Build children list from parent array
    std::vector<std::vector<int>> ch(n);
    for (int v = 0; v < n; v++)
        if (parent[v] != -1)
            ch[parent[v]].push_back(v);

    // Check for nodes unreachable from start
    bool hasDisconn = false;
    for (int v = 0; v < n; v++)
        if (v != start && parent[v] == -1 && ch[v].empty()) { hasDisconn = true; break; }

    std::cout << "\n  " << CLR_RESULT
              << "[Traversal Tree - rooted at " << g.getName(start) << "]"
              << CLR_RESET << "\n";
    std::cout << "  " << CLR_BASE << "[" << start << "] " << g.getName(start) << CLR_RESET << "\n";

    // Recursive print via std::function
    std::function<void(int, const std::string&, bool)> printNode;
    printNode = [&](int node, const std::string& prefix, bool isLast) {
        // Look up edge from parent to this node
        int par = parent[node];
        int ew = 0, er = 0;
        if (par >= 0) {
            for (const auto& e : g.adj[par])
                if (e.to == node) { ew = e.weight; er = e.risk; break; }
        }
        const char* rc = RiskUtil::color(er);

        std::cout << prefix
                  << (isLast ? "  \xE2\x94\x94\xE2\x94\x80\xE2\x94\x80 "
                             : "  \xE2\x94\x9C\xE2\x94\x80\xE2\x94\x80 ");
        std::cout << CLR_BASE << "[" << node << "] "
                  << std::left << std::setw(22) << g.getName(node) << CLR_RESET
                  << rc << "(d:" << std::setw(4) << ew
                  << " r:" << er << " " << RiskUtil::label(er) << ")" << CLR_RESET << "\n";

        std::string np = prefix + (isLast ? "       " : "  \xE2\x94\x82    ");
        for (size_t i = 0; i < ch[node].size(); i++) {
            bool last = (i + 1 == ch[node].size());
            printNode(ch[node][i], np, last);
        }
    };

    for (size_t i = 0; i < ch[start].size(); i++) {
        bool last = (i + 1 == ch[start].size()) && !hasDisconn;
        printNode(ch[start][i], "", last);
    }

    if (hasDisconn)
        std::cout << "  " << CLR_WARN << "  (* disconnected nodes not shown in tree)" << CLR_RESET << "\n";
}

// -----------------------------------------------------------------------------
//  Complexity stats block
// -----------------------------------------------------------------------------

void Traversal::printComplexity(const std::string& type, int V, int E, long long ms) {
    std::cout << "\n  " << CLR_BORDER << std::string(54, '-') << CLR_RESET << "\n";
    std::cout << "  " << CLR_RESULT << "  Complexity [" << type << "]" << CLR_RESET << "\n";
    std::cout << "  " << CLR_RESULT
              << "    Time  : O(V + E)       Space : O(V)" << CLR_RESET << "\n";
    std::cout << "  " << CLR_RESULT
              << "    Input : V=" << V << " nodes, E=" << E << " edges"
              << "   Elapsed: " << ms << " ms" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << std::string(54, '-') << CLR_RESET << "\n";
}
