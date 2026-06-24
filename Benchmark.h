#pragma once
#include "Graph.h"

// -----------------------------------------------------------------------------
//  Benchmark - empirical algorithm performance vs theoretical complexity
//
//  Generates random connected graphs of increasing size, runs each algorithm,
//  and prints a formatted comparison table. Lets you visually confirm that
//  measured runtimes grow at the expected asymptotic rate.
// -----------------------------------------------------------------------------
class Benchmark {
public:
    // Run the full suite and print the report
    static void runAll();

    // Build a random connected graph: n nodes, ~(n + extraEdges) edges
    // seed controls reproducibility
    static Graph generateRandom(int n, int extraEdges, int seed = 42);
};
