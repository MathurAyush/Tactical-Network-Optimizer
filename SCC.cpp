#include "SCC.h"
#include "Colors.h"
#include <stack>
#include <queue>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <sstream>

// -----------------------------------------------------------------------------
//  Tarjan's SCC — iterative to avoid system-stack overflow on large inputs
// -----------------------------------------------------------------------------

SCCResult SCC::tarjan(const Graph& g) {
    int n = g.size();
    SCCResult res;
    res.comp.assign(n, -1);

    std::vector<int>  disc(n, -1), low(n, 0);
    std::vector<bool> onStack(n, false);
    std::stack<int>   stk;
    int timer = 0;

    // Iterative Tarjan using an explicit call-stack frame
    struct Frame {
        int u, parentEdgeIdx;
    };

    int numComp = 0;

    for (int start = 0; start < n; start++) {
        if (disc[start] != -1) continue;

        std::stack<Frame> callStack;
        std::vector<int>  adjPtr(n, 0); // index into adj[u] for iterative DFS
        callStack.push({start, -1});
        disc[start] = low[start] = timer++;
        stk.push(start);
        onStack[start] = true;

        while (!callStack.empty()) {
            int u = callStack.top().u;

            if (adjPtr[u] < (int)g.adj[u].size()) {
                int v = g.adj[u][adjPtr[u]++].to;

                if (disc[v] == -1) {
                    disc[v] = low[v] = timer++;
                    stk.push(v);
                    onStack[v] = true;
                    callStack.push({v, -1});
                } else if (onStack[v]) {
                    low[u] = std::min(low[u], disc[v]);
                }
            } else {
                // All neighbours processed — pop
                callStack.pop();
                if (!callStack.empty()) {
                    int parent = callStack.top().u;
                    low[parent] = std::min(low[parent], low[u]);
                }

                // Root of an SCC?
                if (low[u] == disc[u]) {
                    std::vector<int> comp;
                    while (true) {
                        int v = stk.top(); stk.pop();
                        onStack[v] = false;
                        res.comp[v] = numComp;
                        comp.push_back(v);
                        if (v == u) break;
                    }
                    res.components.push_back(comp);
                    numComp++;
                }
            }
        }
    }

    res.numComponents = numComp;
    return res;
}

// -----------------------------------------------------------------------------
//  BFS connected-component labelling (for undirected graphs)
// -----------------------------------------------------------------------------

SCCResult SCC::connectedComponents(const Graph& g) {
    int n = g.size();
    SCCResult res;
    res.comp.assign(n, -1);
    int numComp = 0;

    for (int start = 0; start < n; start++) {
        if (res.comp[start] != -1) continue;
        std::queue<int> q;
        q.push(start);
        res.comp[start] = numComp;
        std::vector<int> comp;
        while (!q.empty()) {
            int u = q.front(); q.pop();
            comp.push_back(u);
            for (const auto& e : g.adj[u]) {
                if (res.comp[e.to] == -1) {
                    res.comp[e.to] = numComp;
                    q.push(e.to);
                }
            }
        }
        res.components.push_back(comp);
        numComp++;
    }

    res.numComponents = numComp;
    return res;
}

// -----------------------------------------------------------------------------
//  Public interface
// -----------------------------------------------------------------------------

SCCResult SCC::analyze(const Graph& g) {
    return g.directed ? tarjan(g) : connectedComponents(g);
}

// -----------------------------------------------------------------------------
//  Print
// -----------------------------------------------------------------------------

void SCC::printResult(const SCCResult& res, const Graph& g) {
    bool isDir = g.directed;
    std::string title = isDir ? "Strongly Connected Components (Tarjan's)"
                               : "Connected Components (undirected network)";

    std::cout << "\n  " << CLR_BORDER << "+" << std::string(52, '=') << "+" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "|  " << CLR_RESULT
              << std::left << std::setw(50) << title
              << CLR_BORDER << "|" << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << "+" << std::string(52, '=') << "+" << CLR_RESET << "\n";

    std::cout << "  " << CLR_RESULT
              << "  Total components : " << res.numComponents << CLR_RESET << "\n";

    if (isDir) {
        int isolated = 0;
        for (const auto& comp : res.components)
            if ((int)comp.size() == 1) isolated++;
        std::cout << "  " << CLR_RESULT
                  << "  Isolated nodes   : " << isolated << CLR_RESET << "\n";
        std::cout << "  " << CLR_RESULT
                  << "  Fully connected  : "
                  << (res.numComponents == 1 ? "YES - single SCC" : "NO  - multiple SCCs")
                  << CLR_RESET << "\n";
    } else {
        std::cout << "  " << CLR_RESULT
                  << "  Network connected: "
                  << (res.numComponents == 1 ? "YES" : "NO - " + std::to_string(res.numComponents) + " islands")
                  << CLR_RESET << "\n";
    }

    std::cout << "  " << CLR_BORDER << std::string(54, '-') << CLR_RESET << "\n";

    // Sort components by size descending for readability
    std::vector<int> order(res.numComponents);
    for (int i = 0; i < res.numComponents; i++) order[i] = i;
    std::sort(order.begin(), order.end(), [&](int a, int b){
        return res.components[a].size() > res.components[b].size();
    });

    for (int rank = 0; rank < res.numComponents; rank++) {
        int ci = order[rank];
        const auto& comp = res.components[ci];

        std::string label = isDir ? ("SCC #" + std::to_string(rank + 1))
                                  : ("Component #" + std::to_string(rank + 1));

        // Colour: large components green, singletons red
        const char* col = (comp.size() > 1) ? CLR_RESULT : CLR_WARN;
        std::cout << "  " << col << "  " << std::left << std::setw(14) << label
                  << " (" << comp.size() << " bases): ";

        for (size_t i = 0; i < comp.size(); i++) {
            if (i) std::cout << "  ->  ";
            std::cout << "[" << comp[i] << "] " << g.getName(comp[i]);
        }
        std::cout << CLR_RESET << "\n";

        // Tactical annotation for singletons in directed graphs
        if (isDir && comp.size() == 1) {
            std::cout << "  " << CLR_WARN
                      << "    *** ISOLATED: [" << comp[0] << "] " << g.getName(comp[0])
                      << " cannot reach or be reached from the main network ***"
                      << CLR_RESET << "\n";
        }
    }
    std::cout << "  " << CLR_BORDER << std::string(54, '-') << CLR_RESET << "\n";

    // Tactical summary for directed graphs
    if (isDir && res.numComponents > 1) {
        std::cout << "  " << CLR_WARN
                  << "  ALERT: Network is not fully strongly connected.\n"
                  << "  Commands issued from one SCC may not reach all bases.\n"
                  << "  Consider adding directed comm links between SCCs."
                  << CLR_RESET << "\n";
        std::cout << "  " << CLR_BORDER << std::string(54, '-') << CLR_RESET << "\n";
    }
}

void SCC::printComplexity(int V, int E, long long ms) {
    std::cout << "\n  " << CLR_BORDER << std::string(54, '-') << CLR_RESET << "\n";
    std::cout << "  " << CLR_RESULT << "  Complexity [Tarjan's SCC]" << CLR_RESET << "\n";
    std::cout << "  " << CLR_RESULT
              << "    Time  : O(V + E)   Space : O(V)"
              << CLR_RESET << "\n";
    std::cout << "  " << CLR_RESULT
              << "    Input : V=" << V << "  E=" << E
              << "   Elapsed: " << ms << " ms"
              << CLR_RESET << "\n";
    std::cout << "  " << CLR_BORDER << std::string(54, '-') << CLR_RESET << "\n";
}
