#include "MissionPlanner.h"
#include "ShortestPath.h"
#include "Colors.h"
#include "RiskUtil.h"
#include <algorithm>
#include <iostream>
#include <iomanip>

// Stitch together the actual node-by-node route between consecutive stops
// using the Floyd-Warshall 'next' hop table (avoids duplicating junctions).
static std::vector<int> expandRoute(const APSPResult& apsp, const std::vector<int>& order) {
    std::vector<int> full;
    for (size_t i = 0; i + 1 < order.size(); i++) {
        int u = order[i], v = order[i + 1];
        std::vector<int> seg;
        int cur = u;
        seg.push_back(cur);
        int guard = 0;
        while (cur != v && apsp.next[cur][v] != -1 && guard++ < 1000) {
            cur = apsp.next[cur][v];
            seg.push_back(cur);
        }
        if (!full.empty()) full.pop_back(); // drop duplicated junction node
        full.insert(full.end(), seg.begin(), seg.end());
    }
    return full;
}

MissionPlan MissionPlanner::plan(const Graph& g, int start,
                                 const std::vector<int>& targets, int budget) {
    MissionPlan mp;

    // De-duplicate target list and ensure 'start' is the anchor
    std::vector<int> stops;
    for (int t : targets)
        if (t != start && t >= 0 && t < g.size() &&
            std::find(stops.begin(), stops.end(), t) == stops.end())
            stops.push_back(t);

    if (stops.empty()) {
        mp.found = true;
        mp.route = {start};
        mp.withinBudget = true;
        return mp;
    }

    auto apsp = ShortestPath::floydWarshall(g);
    auto& mat = apsp.dist;

    std::vector<int> bestOrder;
    int bestCost = INF;
    bool feasible = false;

    if (stops.size() <= 8) {
        // Exact: brute force over all permutations of the stops
        std::vector<int> perm = stops;
        std::sort(perm.begin(), perm.end());
        do {
            int cost = 0;
            bool ok = true;
            int prev = start;
            for (int x : perm) {
                if (mat[prev][x] >= INF) { ok = false; break; }
                cost += mat[prev][x];
                prev = x;
            }
            if (ok && mat[prev][start] < INF) {
                cost += mat[prev][start];
                if (cost < bestCost) {
                    bestCost = cost;
                    bestOrder = perm;
                    feasible = true;
                }
            }
        } while (std::next_permutation(perm.begin(), perm.end()));
    } else {
        // Heuristic: nearest-neighbour for larger stop lists
        std::vector<bool> visited(g.size(), false);
        visited[start] = true;
        int current = start;
        bool ok = true;
        for (size_t step = 0; step < stops.size(); step++) {
            int best = -1, bestW = INF;
            for (int x : stops) {
                if (!visited[x] && mat[current][x] < bestW) { bestW = mat[current][x]; best = x; }
            }
            if (best == -1 || bestW >= INF) { ok = false; break; }
            visited[best] = true;
            bestOrder.push_back(best);
            current = best;
        }
        if (ok && mat[current][start] < INF) {
            bestCost = 0;
            int prev = start;
            for (int x : bestOrder) { bestCost += mat[prev][x]; prev = x; }
            bestCost += mat[prev][start];
            feasible = true;
        }
    }

    if (!feasible) {
        mp.found = false;
        return mp;
    }

    std::vector<int> order = {start};
    order.insert(order.end(), bestOrder.begin(), bestOrder.end());
    order.push_back(start);

    mp.route         = expandRoute(apsp, order);
    mp.totalDistance = bestCost;
    mp.totalRisk     = RiskUtil::pathRisk(g, mp.route);
    mp.fuelUsed      = mp.totalDistance; // 1 fuel unit per unit distance
    mp.withinBudget  = (mp.totalDistance <= budget);
    mp.found         = true;
    return mp;
}

// -----------------------------------------------------------------------------
//  Output
// -----------------------------------------------------------------------------

void MissionPlanner::printPlan(const MissionPlan& mp, const Graph& g, int budget) {
    std::cout << "\n  " << CLR_RESULT << "[Mission Planner - Optimal Route]" << CLR_RESET << "\n";
    RiskUtil::printLegend();
    std::cout << "  " << std::string(60, '-') << "\n";

    if (!mp.found) {
        std::cout << "  " << CLR_WARN
                  << "! No route exists that visits every selected base and returns "
                     "(network may be disconnected)." << CLR_RESET << "\n";
        return;
    }

    std::cout << "  " << CLR_RESULT << "Route:" << CLR_RESET << "\n    ";
    RiskUtil::printColoredPath(g, mp.route);
    std::cout << "\n";
    std::cout << "  " << std::string(60, '-') << "\n";
    std::cout << "  Total distance : " << mp.totalDistance
              << "   (fuel/distance budget: " << budget << ")\n";
    std::cout << "  Total risk     : " << RiskUtil::color(mp.totalRisk > 10 ? 10 : mp.totalRisk)
              << mp.totalRisk << CLR_RESET << "\n";
    std::cout << "  Fuel used      : " << mp.fuelUsed << "\n";

    if (mp.withinBudget) {
        std::cout << "  " << CLR_BORDER << "[OK] Mission fits within budget." << CLR_RESET << "\n";
    } else {
        std::cout << "  " << CLR_WARN
                  << "[OVER BUDGET] No route visiting all selected bases fits the budget - "
                     "showing the closest feasible route (minimum total distance)."
                  << CLR_RESET << "\n";
    }
}
