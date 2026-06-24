# ─────────────────────────────────────────────────────────────────────────────
#  Makefile – Military Logistics & Tactical Network Optimizer
#  Targets: all (default), clean, run
# ─────────────────────────────────────────────────────────────────────────────

CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2
TARGET   := mil_optimizer

SRCS := main.cpp       \
        Graph.cpp      \
        MST.cpp        \
        Traversal.cpp  \
        ShortestPath.cpp \
        MaxFlow.cpp    \
        TSP.cpp        \
        Visualization.cpp \
        FileLoader.cpp \
        Resilience.cpp \
        MissionPlanner.cpp \
        MapEditor.cpp  \
        MissionLog.cpp \
        AStar.cpp      \
        SCC.cpp        \
        Centrality.cpp \
        Benchmark.cpp

OBJS := $(SRCS:.cpp=.o)

# Library objects shared between main binary and test binary
LIB_SRCS := Graph.cpp MST.cpp Traversal.cpp ShortestPath.cpp MaxFlow.cpp \
            TSP.cpp Visualization.cpp FileLoader.cpp Resilience.cpp      \
            MissionPlanner.cpp MapEditor.cpp MissionLog.cpp              \
            AStar.cpp SCC.cpp Centrality.cpp Benchmark.cpp
LIB_OBJS := $(LIB_SRCS:.cpp=.o)

TEST_TARGET := run_tests

# ── Default target ─────────────────────────────────────────────────────────────
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^
	@echo "Build successful: $(TARGET)"

# Pattern rule for object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# ── Convenience targets ────────────────────────────────────────────────────────
run: all
	./$(TARGET)

# ── Unit test target ──────────────────────────────────────────────────────────
$(TEST_TARGET): tests.cpp $(LIB_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ tests.cpp $(LIB_OBJS)
	@echo "Test binary built: $(TEST_TARGET)"

tests: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -f $(OBJS) tests.o $(TARGET) $(TARGET).exe $(TEST_TARGET) $(TEST_TARGET).exe

# ── Windows (MinGW) variant ────────────────────────────────────────────────────
win: CXXFLAGS += -static-libgcc -static-libstdc++
win: all

.PHONY: all run clean win tests
