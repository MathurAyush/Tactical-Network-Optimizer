#pragma once
#include <string>

// -----------------------------------------------------------------------------
//  MissionLog - Historical Log of every algorithm run
//  Appends one line per run to mission_log.txt:
//    [DATE TIME] OPERATION | Input | Result | Time taken (ms)
// -----------------------------------------------------------------------------
class MissionLog {
public:
    static const char* LOG_FILE;

    // Append one entry to mission_log.txt
    static void record(const std::string& operation, const std::string& input,
                        const std::string& result, long long timeTakenMs);

    // Print the last 'count' entries inside the CLI
    static void viewLast(int count);

    // Export the full log as a formatted report .txt file
    static bool exportReport(const std::string& filename);
};
