#pragma once
#include "Graph.h"
#include <string>

// -----------------------------------------------------------------------------
//  FileLoader - reads / writes the .txt network description format
//
//  map.txt grammar
//  ---------------
//  Lines starting with '#' are comments and are ignored.
//
//  BASES <n>          - declares n bases; next n non-comment lines are names
//  DIRECTED <0|1>     - 0 = undirected (default), 1 = directed
//  EDGES              - following non-comment lines are edges:
//                         <from_id>  <to_id>  <weight>  [<risk 0-10>]
//                       (risk is optional; omitted edges default to risk 0)
// -----------------------------------------------------------------------------
class FileLoader {
public:
    // Returns true on success; g is populated in-place.
    static bool load(const std::string& filename, Graph& g);

    // Serialise g back to the same format (useful for saving user edits).
    static bool save(const std::string& filename, const Graph& g);
};
