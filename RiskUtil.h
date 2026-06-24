#pragma once
#include "Colors.h"
#include "Graph.h"
#include <iostream>
#include <string>
#include <vector>

// -----------------------------------------------------------------------------
//  RiskUtil - shared helpers for color-coding routes by threat/risk level
//    Risk  0-3 : SAFE     -> bright green  \033[92m
//    Risk  4-6 : CAUTION  -> bright yellow \033[93m
//    Risk  7-10: DANGER   -> bright red    \033[91m
// -----------------------------------------------------------------------------
namespace RiskUtil {

inline const char* color(int risk) {
    if (risk <= 3) return "\033[92m";   // SAFE
    if (risk <= 6) return "\033[93m";   // CAUTION
    return "\033[91m";                   // DANGER
}

inline const char* label(int risk) {
    if (risk <= 3) return "SAFE";
    if (risk <= 6) return "CAUTION";
    return "DANGER";
}

// Risk legend printed at the top of every route-related output
inline void printLegend() {
    std::cout << "  " << "\033[92m[GREEN=SAFE]" << CLR_RESET << " "
              << "\033[93m[YELLOW=CAUTION]" << CLR_RESET << " "
              << "\033[91m[RED=DANGER]" << CLR_RESET << "\n";
}

// Sum of risk scores along a node-id path (using direct-edge lookups)
inline int pathRisk(const Graph& g, const std::vector<int>& path) {
    int total = 0;
    for (size_t i = 0; i + 1 < path.size(); i++) {
        int r = g.getRisk(path[i], path[i + 1]);
        if (r > 0) total += r;
    }
    return total;
}

// Print a node-id path as a colour-coded chain: A --(risk)--> B --(risk)--> C
inline void printColoredPath(const Graph& g, const std::vector<int>& path) {
    for (size_t i = 0; i < path.size(); i++) {
        if (i > 0) {
            int r = g.getRisk(path[i - 1], path[i]);
            if (r < 0) r = 0;
            std::cout << " " << color(r) << "--(" << r << ")-->" << CLR_RESET << " ";
        }
        std::cout << CLR_BASE << g.getName(path[i]) << CLR_RESET;
    }
}

} // namespace RiskUtil
