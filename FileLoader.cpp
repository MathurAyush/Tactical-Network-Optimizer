#include "FileLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>

// Strip leading/trailing whitespace
static std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// -----------------------------------------------------------------------------
//  load
// -----------------------------------------------------------------------------

bool FileLoader::load(const std::string& filename, Graph& g) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "  ! Cannot open file: " << filename << "\n";
        return false;
    }

    int  numBases   = 0;
    bool directed   = false;
    bool inBases    = false;
    bool inEdges    = false;
    bool inCoords   = false;
    int  baseIdx    = 0;

    std::string line;
    int lineNum = 0;
    bool graphInit  = false;

    while (std::getline(file, line)) {
        lineNum++;
        line = trim(line);

        // Skip comments and blank lines
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        std::string token;
        iss >> token;

        // -- Section keywords --------------------------------------------------
        if (token == "BASES") {
            iss >> numBases;
            if (numBases <= 0) {
                std::cerr << "  ! Invalid BASES count on line " << lineNum << "\n";
                return false;
            }
            inBases  = true;
            inEdges  = false;
            inCoords = false;
            baseIdx  = 0;
            continue;
        }

        if (token == "DIRECTED") {
            int d; iss >> d;
            directed = (d != 0);
            continue;
        }

        if (token == "EDGES") {
            if (numBases <= 0) {
                std::cerr << "  ! EDGES section reached before BASES defined.\n";
                return false;
            }
            if (!graphInit) {
                g.reset(numBases, directed);
                graphInit = true;
            }
            inBases  = false;
            inEdges  = true;
            inCoords = false;
            continue;
        }

        if (token == "COORDS") {
            inBases  = false;
            inEdges  = false;
            inCoords = true;
            continue;
        }

        // -- Base name lines ----------------------------------------------------
        if (inBases) {
            if (!graphInit && baseIdx == 0) {
                g.reset(numBases, directed);
                graphInit = true;
            }
            if (baseIdx < numBases)
                g.setName(baseIdx++, line);
            continue;
        }

        // -- Coordinate lines --------------------------------------------------
        if (inCoords) {
            // format: nodeId  x  y
            int id;
            double x, y;
            try { id = std::stoi(token); }
            catch (...) { continue; }
            if (!(iss >> x >> y)) continue;
            g.setCoords(id, x, y);
            continue;
        }

        // -- Edge definition lines ---------------------------------------------
        if (inEdges) {
            int from, to, weight;
            int risk = 0; // optional 4th column - threat level 0-10
            if (!(iss >> to >> weight)) {
                // token was already the first number
                try {
                    from   = std::stoi(token);
                    // re-read to and weight
                    if (!(iss >> to >> weight)) {
                        std::cerr << "  ! Malformed edge on line " << lineNum
                                  << ": " << line << "\n";
                        continue;
                    }
                } catch (...) {
                    std::cerr << "  ! Skipping unrecognised line " << lineNum
                              << ": " << line << "\n";
                    continue;
                }
            } else {
                try { from = std::stoi(token); }
                catch (...) {
                    std::cerr << "  ! Skipping unrecognised line " << lineNum
                              << ": " << line << "\n";
                    continue;
                }
            }
            iss >> risk; // optional - leaves risk at 0 if absent
            if (risk < 0) risk = 0;
            if (risk > 10) risk = 10;

            if (from < 0 || from >= numBases || to < 0 || to >= numBases) {
                std::cerr << "  ! Edge on line " << lineNum
                          << " references out-of-range node.\n";
                continue;
            }
            // Undirected graphs: bidirectional = true
            g.addEdge(from, to, weight, risk, !directed);
            continue;
        }
    }

    if (!graphInit) {
        std::cerr << "  ! File contained no valid graph data.\n";
        return false;
    }

    std::cout << "  Loaded: " << g.n << " bases, "
              << g.edgeList.size() << " routes from '" << filename << "'\n";
    return true;
}

// -----------------------------------------------------------------------------
//  save
// -----------------------------------------------------------------------------

bool FileLoader::save(const std::string& filename, const Graph& g) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "  ! Cannot write file: " << filename << "\n";
        return false;
    }

    file << "# Military Tactical Network - exported by optimizer\n\n";
    file << "BASES " << g.n << "\n";
    for (int i = 0; i < g.n; i++)
        file << g.getName(i) << "\n";

    file << "\nDIRECTED " << (g.directed ? 1 : 0) << "\n";
    file << "\nEDGES\n";
    file << "# from  to  weight  risk(0-10)\n";
    for (const Edge& e : g.edgeList)
        file << e.from << "  " << e.to << "  " << e.weight << "  " << e.risk << "\n";

    std::cout << "  Network saved to '" << filename << "'\n";
    return true;
}
