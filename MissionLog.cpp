#include "MissionLog.h"
#include "Colors.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <ctime>
#include <algorithm>

const char* MissionLog::LOG_FILE = "mission_log.txt";

// -----------------------------------------------------------------------------
//  Timestamp helper - "YYYY-MM-DD HH:MM:SS"
// -----------------------------------------------------------------------------

static std::string timestamp() {
    std::time_t t = std::time(nullptr);
    std::tm tmBuf{};
#ifdef _WIN32
    localtime_s(&tmBuf, &t);
#else
    localtime_r(&t, &tmBuf);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tmBuf, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

// -----------------------------------------------------------------------------
//  record - append one log line
// -----------------------------------------------------------------------------

void MissionLog::record(const std::string& operation, const std::string& input,
                        const std::string& result, long long timeTakenMs) {
    std::ofstream f(LOG_FILE, std::ios::app);
    if (!f.is_open()) return;

    f << "[" << timestamp() << "] " << operation
      << " | " << input
      << " | " << result
      << " | " << timeTakenMs << "ms\n";
}

// -----------------------------------------------------------------------------
//  viewLast - show the most recent N entries inside the CLI
// -----------------------------------------------------------------------------

void MissionLog::viewLast(int count) {
    std::ifstream f(LOG_FILE);
    std::cout << "\n  " << CLR_RESULT << "[Mission Log - last " << count << " entries]" << CLR_RESET << "\n";
    std::cout << "  " << std::string(70, '-') << "\n";

    if (!f.is_open()) {
        std::cout << "  " << CLR_WARN << "No log entries yet - run an algorithm first." << CLR_RESET << "\n";
        return;
    }

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(f, line))
        if (!line.empty()) lines.push_back(line);

    if (lines.empty()) {
        std::cout << "  " << CLR_WARN << "Log file is empty." << CLR_RESET << "\n";
        return;
    }

    int start = std::max(0, static_cast<int>(lines.size()) - count);
    for (int i = start; i < static_cast<int>(lines.size()); i++)
        std::cout << "  " << CLR_RESULT << lines[i] << CLR_RESET << "\n";

    std::cout << "  " << std::string(70, '-') << "\n";
    std::cout << "  Showing " << (lines.size() - start) << " of " << lines.size()
              << " total entries.\n";
}

// -----------------------------------------------------------------------------
//  exportReport - write a formatted report file from the full log
// -----------------------------------------------------------------------------

bool MissionLog::exportReport(const std::string& filename) {
    std::ifstream in(LOG_FILE);
    if (!in.is_open()) {
        std::cout << "  " << CLR_WARN << "No log file found - nothing to export." << CLR_RESET << "\n";
        return false;
    }

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(in, line))
        if (!line.empty()) lines.push_back(line);

    std::ofstream out(filename);
    if (!out.is_open()) {
        std::cout << "  " << CLR_WARN << "Cannot write report file: " << filename << CLR_RESET << "\n";
        return false;
    }

    out << "==============================================================\n";
    out << "   MILITARY LOGISTICS & TACTICAL NETWORK OPTIMIZER\n";
    out << "   MISSION LOG - FORMATTED REPORT\n";
    out << "   Generated : " << timestamp() << "\n";
    out << "   Total runs: " << lines.size() << "\n";
    out << "==============================================================\n\n";

    for (size_t i = 0; i < lines.size(); i++)
        out << "  " << std::setw(4) << (i + 1) << ". " << lines[i] << "\n";

    out << "\n==============================================================\n";
    out << "   End of report - " << lines.size() << " operation(s) logged.\n";
    out << "==============================================================\n";

    std::cout << "  " << CLR_BORDER << "Report exported to '" << filename << "' ("
              << lines.size() << " entries)." << CLR_RESET << "\n";
    return true;
}
