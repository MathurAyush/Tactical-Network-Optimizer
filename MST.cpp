#include "MST.h"
#include "Colors.h"
#include "RiskUtil.h"
#include <algorithm>
#include <iostream>
#include <queue>
#include <iomanip>
#include <sstream>
#include <functional>

// -----------------------------------------------------------------------------
//  Union-Find helpers
// -----------------------------------------------------------------------------

int MST::find(std::vector<int>& parent, int x) {
    if (parent[x] != x)
        parent[x] = find(parent, parent[x]);
    return parent[x];
}

void MST::unite(std::vector<int>& parent, std::vector<int>& rnk, int x, int y) {
    int px = find(parent, x), py = find(parent, y);
    if (px == py) return;
    if (rnk[px] < rnk[py]) std::swap(px, py);
    parent[py] = px;
    if (rnk[px] == rnk[py]) rnk[px]++;
}

// -----------------------------------------------------------------------------
//  Kruskal's algorithm
// -----------------------------------------------------------------------------

MSTResult MST::kruskal(const Graph& g) {
    MSTResult res;
    int n = g.size();
    if (n == 0) return res;

    std::vector<Edge> sorted = g.edgeList;
    std::sort(sorted.begin(), sorted.end(),
              [](const Edge& a, const Edge& b){ return a.weight < b.weight; });

    std::vector<int> parent(n), rnk(n, 0);
    for (int i = 0; i < n; i++) parent[i] = i;

    for (const Edge& e : sorted) {
        int pu = find(parent, e.from);
        int pv = find(parent, e.to);
        if (pu != pv) {
            unite(parent, rnk, pu, pv);
            res.edges.push_back(e);
            res.totalWeight += e.weight;
        }
    }
    res.valid = (static_cast<int>(res.edges.size()) == n - 1);
    return res;
}

// -----------------------------------------------------------------------------
//  Prim's algorithm
// -----------------------------------------------------------------------------

MSTResult MST::prim(const Graph& g, int start) {
    MSTResult res;
    int n = g.size();
    if (n == 0) return res;

    std::vector<bool> inMST(n, false);
    std::vector<int>  key(n, INF);
    std::vector<int>  par(n, -1);

    using pii = std::pair<int,int>;
    std::priority_queue<pii, std::vector<pii>, std::greater<pii>> pq;

    key[start] = 0;
    pq.push({0, start});

    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (inMST[u]) continue;
        inMST[u] = true;

        for (const auto& e : g.adj[u]) {
            int v = e.to, w = e.weight;
            if (!inMST[v] && w < key[v]) {
                key[v]  = w;
                par[v]  = u;
                pq.push({w, v});
            }
        }
    }

    for (int v = 0; v < n; v++) {
        if (v == start) continue;
        if (par[v] == -1) { res.valid = false; return res; }
        res.edges.push_back({par[v], v, key[v], g.getRisk(par[v], v)});
        res.totalWeight += key[v];
    }
    res.valid = true;
    return res;
}

// -----------------------------------------------------------------------------
//  Output - result table
// -----------------------------------------------------------------------------

void MST::printResult(const MSTResult& res, const Graph& g,
                      const std::string& algorithm) {
    std::cout << "\n  " << CLR_RESULT << "[" << algorithm << " MST Result]" << CLR_RESET << "\n";
    RiskUtil::printLegend();
    std::cout << "  " << CLR_BORDER << std::string(62, '-') << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << std::left
              << "  " << std::setw(4) << "#"
              << std::setw(6)  << "From"
              << std::setw(22) << "Base"
              << "   Weight   Risk"
              << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << std::string(62, '-') << CLR_RESET << "\n";

    if (!res.valid) {
        std::cout << "  " << CLR_WARN << "  Graph is disconnected - no spanning tree exists." << CLR_RESET << "\n";
        std::cout << "  " << CLR_BORDER << std::string(62, '-') << CLR_RESET << "\n";
        return;
    }

    int idx = 1;
    for (const Edge& e : res.edges) {
        const char* rc = RiskUtil::color(e.risk);
        std::cout << "  " << CLR_RESULT << std::right
                  << "  " << std::setw(3) << idx++ << "  "
                  << "[" << std::setw(2) << e.from << "] "
                  << std::left << std::setw(20) << g.getName(e.from) << std::right
                  << rc << " <--(" << std::setw(4) << e.weight
                  << " r:" << std::setw(2) << e.risk
                  << " " << std::left << std::setw(7) << RiskUtil::label(e.risk)
                  << ")--> " << CLR_RESET
                  << "[" << std::setw(2) << e.to << "] " << g.getName(e.to) << "\n";
    }
    std::cout << "  " << CLR_BORDER << std::string(62, '-') << CLR_RESET << "\n";
    std::cout << "  " << CLR_RESULT << "  Total MST weight : " << res.totalWeight << CLR_RESET << "\n";
    std::cout << "  " << CLR_RESULT << "  Edges selected   : "
              << res.edges.size() << " / " << (g.size() - 1) << " required" << CLR_RESET << "\n";
}

// -----------------------------------------------------------------------------
//  ASCII tree view of the MST structure
// -----------------------------------------------------------------------------

// File-scope recursive helper (not in header)
static void printMSTNodeR(
    const std::vector<std::vector<std::pair<int,Edge>>>& ch,
    const Graph& g, int node, const Edge& eToNode,
    const std::string& prefix, bool isLast)
{
    const char* rc = RiskUtil::color(eToNode.risk);
    std::cout << prefix
              << (isLast ? "  \xE2\x94\x94\xE2\x94\x80\xE2\x94\x80 "
                         : "  \xE2\x94\x9C\xE2\x94\x80\xE2\x94\x80 ");
    std::cout << CLR_BASE << "[" << node << "] "
              << std::left << std::setw(22) << g.getName(node) << CLR_RESET
              << rc << "(d:" << std::setw(4) << eToNode.weight
              << " r:" << eToNode.risk
              << " " << RiskUtil::label(eToNode.risk) << ")" << CLR_RESET << "\n";

    std::string np = prefix + (isLast ? "       " : "  \xE2\x94\x82    ");
    for (size_t i = 0; i < ch[node].size(); i++) {
        bool last = (i + 1 == ch[node].size());
        printMSTNodeR(ch, g, ch[node][i].first, ch[node][i].second, np, last);
    }
}

void MST::printASCIITree(const MSTResult& result, const Graph& g, int root) {
    if (!result.valid) return;
    int n = g.size();

    // Build undirected adjacency from MST edge list
    std::vector<std::vector<std::pair<int,Edge>>> adj(n);
    for (const Edge& e : result.edges) {
        adj[e.from].push_back({e.to,   e});
        Edge rev = e; rev.from = e.to; rev.to = e.from;
        adj[e.to].push_back({e.from, rev});
    }

    // BFS from root to orient as parent -> children
    std::vector<bool> visited(n, false);
    std::vector<std::vector<std::pair<int,Edge>>> ch(n);
    std::queue<int> q;
    visited[root] = true;
    q.push(root);
    while (!q.empty()) {
        int u = q.front(); q.pop();
        for (auto& [v, e] : adj[u]) {
            if (!visited[v]) {
                visited[v] = true;
                Edge ne = e; ne.from = u; ne.to = v;
                ch[u].push_back({v, ne});
                q.push(v);
            }
        }
    }

    std::cout << "\n  " << CLR_RESULT
              << "[MST Tree Structure - rooted at " << g.getName(root) << "]"
              << CLR_RESET << "\n";
    std::cout << "  " << CLR_BASE << "[" << root << "] " << g.getName(root) << CLR_RESET << "\n";
    for (size_t i = 0; i < ch[root].size(); i++) {
        bool last = (i + 1 == ch[root].size());
        printMSTNodeR(ch, g, ch[root][i].first, ch[root][i].second, "", last);
    }
}

// -----------------------------------------------------------------------------
//  Complexity stats block
// -----------------------------------------------------------------------------

void MST::printComplexity(const std::string& algo, int V, int E, long long ms) {
    std::string timeO, spaceO;
    if (algo == "Kruskal") { timeO = "O(E log E)     "; spaceO = "O(V + E)"; }
    else                    { timeO = "O((V+E) log V) "; spaceO = "O(V)    "; }

    std::cout << "\n  " << CLR_BORDER << std::string(54, '-') << CLR_RESET << "\n";
    std::cout << "  " << CLR_RESULT << "  Complexity [" << algo << " MST]" << CLR_RESET << "\n";
    std::cout << "  " << CLR_RESULT << "    Time  : " << timeO
              << "   Space : " << spaceO << CLR_RESET << "\n";
    std::cout << "  " << CLR_RESULT << "    Input : V=" << V << " nodes, E=" << E << " edges"
              << "   Elapsed: " << ms << " ms" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << std::string(54, '-') << CLR_RESET << "\n";
}
