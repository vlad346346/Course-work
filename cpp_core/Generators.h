#pragma once
#include <random>
#include <stack>
#include <numeric>
#include <algorithm>
#include "Grid.h"
#include "Types.h"

// Strategy: спільний інтерфейс для всіх алгоритмів генерації
class GeneratorAlgorithm {
public:
    virtual ~GeneratorAlgorithm() = default;
    virtual void generate(Grid& grid, unsigned seed, StepObserver* obs) = 0;
    virtual const char* name() const = 0;
protected:
    static void carve(Grid& g, int a, int b, StepObserver* obs) {
        g.link(a, b);
        if (obs) obs->onStep({ StepType::Carve, a, b });
    }
};

// 1. Рекурсивний пошук з поверненням: довгі звивисті коридори, мало тупиків
class RecursiveBacktracker : public GeneratorAlgorithm {
public:
    const char* name() const override { return "backtracker"; }
    void generate(Grid& g, unsigned seed, StepObserver* obs) override {
        std::mt19937 rng(seed);
        std::vector<bool> visited(g.cellCount(), false);
        std::stack<int> st;
        st.push(0);
        visited[0] = true;
        while (!st.empty()) {
            int cur = st.top();
            std::vector<int> options;
            for (int n : g.existingNeighbors(cur))
                if (!visited[n]) options.push_back(n);
            if (options.empty()) { st.pop(); continue; }
            int next = options[rng() % options.size()];
            carve(g, cur, next, obs);
            visited[next] = true;
            st.push(next);
        }
    }
};

// 2. Випадковий алгоритм Прима: багато коротких тупиків, "ялинкова" структура
class PrimGenerator : public GeneratorAlgorithm {
public:
    const char* name() const override { return "prim"; }
    void generate(Grid& g, unsigned seed, StepObserver* obs) override {
        std::mt19937 rng(seed);
        std::vector<bool> inTree(g.cellCount(), false);
        std::vector<std::pair<int, int>> frontier;   // ребра (в дереві -> поза деревом)
        auto addCell = [&](int c) {
            inTree[c] = true;
            for (int n : g.existingNeighbors(c))
                if (!inTree[n]) frontier.push_back({ c, n });
        };
        addCell(0);
        while (!frontier.empty()) {
            size_t i = rng() % frontier.size();
            auto e = frontier[i];
            frontier[i] = frontier.back();
            frontier.pop_back();
            if (inTree[e.second]) continue;
            carve(g, e.first, e.second, obs);
            addCell(e.second);
        }
    }
};

// 3. Алгоритм Краскала (система неперетинних множин): рівномірна, "зерниста" структура
class KruskalGenerator : public GeneratorAlgorithm {
public:
    const char* name() const override { return "kruskal"; }
    void generate(Grid& g, unsigned seed, StepObserver* obs) override {
        std::mt19937 rng(seed);
        int n = g.cellCount();
        std::vector<int> parent(n);
        std::iota(parent.begin(), parent.end(), 0);

        std::vector<std::pair<int, int>> edges;
        for (int c = 0; c < n; ++c)
            for (int nb : g.existingNeighbors(c))
                if (c < nb) edges.push_back({ c, nb });
        std::shuffle(edges.begin(), edges.end(), rng);

        for (auto& e : edges) {
            int ra = find(parent, e.first), rb = find(parent, e.second);
            if (ra == rb) continue;                 // уже з'єднані, цикл не робимо
            parent[ra] = rb;
            carve(g, e.first, e.second, obs);
        }
    }
private:
    static int find(std::vector<int>& p, int x) {
        while (p[x] != x) { p[x] = p[p[x]]; x = p[x]; }
        return x;
    }
};
