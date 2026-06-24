#pragma once
#include "Graph.h"
#include <vector>

// -----------------------------------------------------------------------------
//  MissionPlanner - plans an optimal multi-base resupply / patrol mission
//  Given a starting base, a set of bases that must be visited, and a fuel /
//  distance budget, finds the shortest route (TSP over the selected subset,
//  expanded through actual shortest paths) that visits them all and returns
//  to base. If no route fits the budget, the closest (minimum-distance)
//  feasible route is reported instead.
// -----------------------------------------------------------------------------
struct MissionPlan {
    std::vector<int> route;        // full expanded route (every hop, in order)
    int  totalDistance = 0;
    int  totalRisk     = 0;
    int  fuelUsed      = 0;        // 1 fuel unit per unit of distance travelled
    bool withinBudget  = false;
    bool found         = false;    // false if no Hamiltonian circuit exists
};

class MissionPlanner {
public:
    static MissionPlan plan(const Graph& g, int start,
                            const std::vector<int>& targets, int budget);
    static void printPlan(const MissionPlan& mp, const Graph& g, int budget);
};
