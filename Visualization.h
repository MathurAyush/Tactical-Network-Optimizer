#pragma once
#include "Graph.h"

// -----------------------------------------------------------------------------
//  Visualization - ASCII rendering of the tactical network
//    * printBanner()     - application title art
//    * printNetwork()    - formatted adjacency list
//    * printAdjMatrix()  - compact nxn weight matrix
//    * printASCIIMap()   - routes shown as [From] ---> [To] with weights
//    * printStats()      - graph statistics summary
// -----------------------------------------------------------------------------
class Visualization {
public:
    static void printBanner();
    static void printNetwork(const Graph& g);
    static void printAdjMatrix(const Graph& g);
    static void printASCIIMap(const Graph& g);
    static void printStats(const Graph& g);
};
