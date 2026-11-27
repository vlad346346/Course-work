#pragma once
#include <memory>
#include <string>
#include <stdexcept>
#include "RectGrid.h"
#include "Generators.h"
#include "Solvers.h"

// Factory Method: створює об'єкти за назвою, решта програми не знає конкретних класів
class MazeFactory {
public:
    static std::unique_ptr<Grid> createGrid(const std::string& type, int w, int h) {
        if (type == "rect") return std::make_unique<RectGrid>(w, h);
        // сюди додається друга сітка: if (type == "hex") return std::make_unique<HexGrid>(w, h);
        throw std::invalid_argument("Unknown grid type: " + type);
    }
    static std::unique_ptr<GeneratorAlgorithm> createGenerator(const std::string& name) {
        if (name == "backtracker") return std::make_unique<RecursiveBacktracker>();
        if (name == "prim")        return std::make_unique<PrimGenerator>();
        if (name == "kruskal")     return std::make_unique<KruskalGenerator>();
        throw std::invalid_argument("Unknown generator: " + name);
    }
    static std::unique_ptr<SolverAlgorithm> createSolver(const std::string& name) {
        if (name == "bfs")          return std::make_unique<BFSSolver>();
        if (name == "astar")        return std::make_unique<AStarSolver>();
        if (name == "wallfollower") return std::make_unique<WallFollowerSolver>();
        throw std::invalid_argument("Unknown solver: " + name);
    }
};
