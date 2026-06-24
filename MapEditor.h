#pragma once
#include "Graph.h"
#include <string>

// -----------------------------------------------------------------------------
//  MapEditor - Dynamic Map Editor
//  Lets the operator add/remove bases and routes at runtime. The in-memory
//  Graph is rebuilt in place so every other menu immediately sees the change,
//  and the network is re-saved to map.txt so edits persist across sessions.
// -----------------------------------------------------------------------------
class MapEditor {
public:
    static void addBase(Graph& g, const std::string& name);
    static void addRoute(Graph& g, int from, int to, int distance, int risk);
    static bool removeBase(Graph& g, int id);
    static bool removeRoute(Graph& g, int from, int to);
};
