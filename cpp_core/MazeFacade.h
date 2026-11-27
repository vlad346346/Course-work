#pragma once
#include <chrono>
#include <memory>
#include <string>
#include "MazeFactory.h"

// Facade: єдина проста точка входу до ядра (generate, solve, статистика, кроки)
class MazeFacade {
public:
    void generate(const std::string& gridType, int w, int h,
                  const std::string& algo, unsigned seed) {
        auto newGrid = MazeFactory::createGrid(gridType, w, h);
        auto gen = MazeFactory::createGenerator(algo);
        genSteps.clear(); solveSteps.clear();
        result = SolveResult();
        stats = Statistics();

        auto t0 = Clock::now();
        gen->generate(*newGrid, seed, &genSteps);
        stats.genTimeMs = elapsedMs(t0);

        grid = std::move(newGrid);
        for (int c = 0; c < grid->cellCount(); ++c) {
            size_t d = grid->links(c).size();
            if (d == 1) ++stats.deadEnds;
            if (d >= 3) ++stats.branchPoints;
        }
    }

    void solve(const std::string& algo, int start, int goal) {
        if (!grid) throw std::logic_error("Maze is not generated yet");
        int n = grid->cellCount();
        if (start < 0 || start >= n || goal < 0 || goal >= n)
            throw std::out_of_range("start/goal out of range");
        auto solver = MazeFactory::createSolver(algo);
        solveSteps.clear();

        auto t0 = Clock::now();
        result = solver->solve(*grid, start, goal, &solveSteps);
        stats.solveTimeMs = elapsedMs(t0);
        stats.visitedCells = result.visited;
        stats.pathLength = static_cast<int>(result.path.size());
    }

    std::unique_ptr<Grid> grid;
    StepRecorder genSteps, solveSteps;
    SolveResult result;
    Statistics stats;
    std::string lastError;

private:
    using Clock = std::chrono::steady_clock;
    static double elapsedMs(Clock::time_point t0) {
        return std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
    }
};
