# Tactical Network Optimizer

> A C++ terminal application that models a military logistics network as a weighted graph and runs a full suite of 15 graph algorithms — built for DRDO-style analysis with an ANSI-colored CMD interface and zero external dependencies.

---

## Features

- **15 graph algorithms** spanning shortest paths, flow networks, spanning trees, and more
- **Military-domain framing** — bases, supply routes, patrol circuits, attack simulations
- **Risk-colored output** — GREEN (safe) · YELLOW (caution) · RED (danger) on every route
- **ASCII tree / map visualizations** rendered directly in the terminal
- **Empirical benchmarking** — measured runtimes vs theoretical Big-O on n = 10 / 50 / 100 / 200
- **Persistent mission log** — timestamped record of every algorithm run
- **Runtime map editor** — add/remove bases and routes; auto-saves to `map.txt`
- **64-test unit suite** with exact expected values on known graphs

---

## Algorithms

| # | Algorithm | Module | Time Complexity | Military Use |
|---|-----------|--------|-----------------|--------------|
| 1 | Kruskal's MST | `MST` | O(E log E) | Minimum-cost comms backbone |
| 2 | Prim's MST | `MST` | O((V+E) log V) | Greedy backbone from HQ |
| 3 | BFS | `Traversal` | O(V + E) | Level-order base reachability |
| 4 | DFS | `Traversal` | O(V + E) | Deep-path reconnaissance |
| 5 | Dijkstra | `ShortestPath` | O((V+E) log V) | Fastest supply route |
| 6 | Bellman-Ford | `ShortestPath` | O(V·E) | Routes with variable threat weights |
| 7 | Floyd-Warshall | `ShortestPath` | O(V³) | All-pairs supply distance matrix |
| 8 | A\* Search | `AStar` | O((V+E) log V) | Heuristic-guided pathfinding |
| 9 | Edmonds-Karp | `MaxFlow` | O(V·E²) | Max supply throughput / bottleneck |
| 10 | TSP Brute Force | `TSP` | O(V!) | Exact optimal patrol circuit (n ≤ 12) |
| 11 | TSP Nearest Neighbor | `TSP` | O(V²) | Fast heuristic patrol circuit |
| 12 | Bridge Detection | `Resilience` | O(V + E) | Critical route identification |
| 13 | Global Min-Cut | `Resilience` | O(V·E²) | Network weakest point under attack |
| 14 | Tarjan's SCC | `SCC` | O(V + E) | Isolated sub-network detection |
| 15 | Centrality Analysis | `Centrality` | O(V·(V+E) log V) | Strategic base importance ranking |

---

## Build

Requires **g++ (MinGW on Windows / GCC on Linux)** with C++17 support.

```bash
mingw32-make          # build mil_optimizer.exe
mingw32-make run      # build and launch
mingw32-make tests    # build and run the 64-test unit suite
mingw32-make clean    # remove all object files and binaries
```

On Linux/macOS replace `mingw32-make` with `make`.

---

## Usage

```
.\mil_optimizer.exe
```

The program auto-loads `map.txt` on startup. All interaction is through a numbered menu.

```
  [1]   Load / Build Network
  [2]   Visualize Network          (ASCII map · adjacency list/matrix · stats)
  [3]   Minimum Spanning Tree      (Kruskal / Prim + ASCII tree)
  [4]   Network Traversal          (BFS / DFS + traversal tree)
  [5]   Shortest Path Analysis     (Dijkstra / Bellman-Ford / Floyd-Warshall / A*)
  [6]   Maximum Flow Analysis      (Edmonds-Karp)
  [7]   TSP Route Optimization     (Brute Force / Nearest Neighbor)
  [8]   Network Resilience         (Bridges · Min-Cut · Attack Simulation)
  [9]   Mission Planner            (Multi-stop with fuel/distance budget)
  [10]  Dynamic Map Editor
  [11]  Mission Log                (view / export)
  [12]  Run All Algorithms + Summary
  [13]  Strongly Connected Components (Tarjan's)
  [14]  Network Centrality Analysis
  [15]  Algorithm Benchmark
  [0]   Exit
```

---

## Network File Format (`map.txt`)

```
BASES <n>
<name_0>
...
<name_n-1>

DIRECTED <0|1>

EDGES
# from  to  weight  risk(0-10)
0  1  45  2

# Optional node coordinates for A* heuristic
COORDS
# nodeId  x  y
0  12  12
1  18   6
```

| Field | Description |
|-------|-------------|
| `weight` | Transit distance / time in minutes |
| `risk` | Threat level 0–10 (`0-3` SAFE · `4-6` CAUTION · `7-10` DANGER) |
| `COORDS` | Spatial positions; enables A*'s Euclidean heuristic |

The included `map.txt` models 10 strategic bases across a fictional theatre of operations.

---

## Unit Tests

```
============================================================
  DRDO Tactical Network Optimizer  -  Unit Test Suite
============================================================

--- Graph Construction ---   [11 tests]
--- MST ---                  [ 8 tests]
--- Traversal ---            [ 7 tests]
--- Shortest Path ---        [12 tests]
--- Maximum Flow ---         [ 3 tests]
--- TSP ---                  [ 5 tests]
--- Resilience ---           [ 3 tests]
--- SCC ---                  [ 5 tests]
--- Centrality ---           [ 3 tests]
--- A* Search ---            [ 7 tests]

  Results: 64 passed,  0 failed
============================================================
```

Tests cover triangle, path, K4, directed cycle, and star graphs with exact expected values for MST weight, shortest-path distances, max-flow, SCC membership, and centrality scores.

---

## Project Structure

```
Tactical-Network-Optimizer/
│
├── main.cpp                # CLI menus, timing, Run All summary
│
├── Graph.h / .cpp          # Core graph: adjacency list, edge list, coordinates
├── Colors.h                # ANSI colour constants
├── RiskUtil.h              # Risk colouring, labels, path-risk helpers
│
├── MST.h / .cpp            # Kruskal, Prim, ASCII tree, complexity block
├── Traversal.h / .cpp      # BFS, DFS, traversal tree, complexity block
├── ShortestPath.h / .cpp   # Dijkstra, Bellman-Ford, Floyd-Warshall
├── AStar.h / .cpp          # A* with Euclidean heuristic + Dijkstra comparison
├── MaxFlow.h / .cpp        # Edmonds-Karp max flow
├── TSP.h / .cpp            # Brute-force & nearest-neighbor TSP
│
├── Resilience.h / .cpp     # Bridge detection, min-cut, attack simulation
├── SCC.h / .cpp            # Tarjan's SCC (directed) / BFS components (undirected)
├── Centrality.h / .cpp     # Degree, betweenness, closeness centrality
├── Benchmark.h / .cpp      # Random graph generator + runtime vs complexity table
│
├── MissionPlanner.h / .cpp # Multi-stop budget-constrained route planner
├── MapEditor.h / .cpp      # Runtime graph mutation (add/remove bases & routes)
├── MissionLog.h / .cpp     # Persistent operation log (mission_log.txt)
├── Visualization.h / .cpp  # ASCII map, adjacency list/matrix, network stats, banner
├── FileLoader.h / .cpp     # map.txt parser and writer
│
├── tests.cpp               # 64-test unit suite (separate binary)
├── Makefile                # Build system (g++ C++17)
└── map.txt                 # Sample 10-base tactical network with coordinates
```

---

## Tech Stack

| | |
|---|---|
| **Language** | C++17 |
| **Compiler** | g++ (MinGW / GCC) |
| **UI** | ANSI escape codes — no external libraries |
| **Build** | GNU Make |

---

*Developed as part of a DRDO internship project on graph-algorithm applications in military logistics and network analysis.*
