#pragma once
#include <queue>
#include <limits>
#include <algorithm>
#include "Grid.h"
#include "Types.h"

// Strategy: спільний інтерфейс для всіх алгоритмів пошуку шляху
class SolverAlgorithm {
public:
    virtual ~SolverAlgorithm() = default;
    virtual SolveResult solve(const Grid& g, int start, int goal, StepObserver* obs) = 0;
    virtual const char* name() const = 0;
protected:
    static void notify(StepObserver* obs, StepType t, int a, int b) {
        if (obs) obs->onStep({ t, a, b });
    }
    // Відновлення шляху за масивом "батьків" + повідомлення про клітинки шляху
    static std::vector<int> buildPath(const std::vector<int>& parent, int start, int goal,
                                      StepObserver* obs) {
        std::vector<int> path;
        for (int c = goal; c != -1; c = parent[c]) {
            path.push_back(c);
            if (c == start) break;
        }
        std::reverse(path.begin(), path.end());
        for (int c : path) notify(obs, StepType::Path, c, -1);
        return path;
    }
};

// 1. BFS: завжди знаходить найкоротший шлях, але відвідує багато клітинок
class BFSSolver : public SolverAlgorithm {
public:
    const char* name() const override { return "bfs"; }
    SolveResult solve(const Grid& g, int start, int goal, StepObserver* obs) override {
        SolveResult r;
        std::vector<int> parent(g.cellCount(), -1);
        std::vector<bool> seen(g.cellCount(), false);
        std::queue<int> q;
        q.push(start);
        seen[start] = true;
        notify(obs, StepType::Visit, start, -1);
        r.visited = 1;
        while (!q.empty()) {
            int c = q.front(); q.pop();
            if (c == goal) { r.found = true; break; }
            for (int n : g.links(c)) {
                if (seen[n]) continue;
                seen[n] = true;
                parent[n] = c;
                ++r.visited;
                notify(obs, StepType::Visit, n, c);
                q.push(n);
            }
        }
        if (r.found) r.path = buildPath(parent, start, goal, obs);
        return r;
    }
};

// 2. A*: найкоротший шлях з евристикою, зазвичай відвідує менше клітинок за BFS
class AStarSolver : public SolverAlgorithm {
public:
    const char* name() const override { return "astar"; }
    SolveResult solve(const Grid& g, int start, int goal, StepObserver* obs) override {
        SolveResult r;
        int n = g.cellCount();
        const double INF = std::numeric_limits<double>::infinity();
        std::vector<double> gScore(n, INF);
        std::vector<int> parent(n, -1);
        std::vector<bool> closed(n, false);
        using Item = std::pair<double, int>;                 // (f, клітинка)
        std::priority_queue<Item, std::vector<Item>, std::greater<Item>> open;
        gScore[start] = 0;
        open.push({ g.distance(start, goal), start });
        while (!open.empty()) {
            int c = open.top().second; open.pop();
            if (closed[c]) continue;
            closed[c] = true;
            ++r.visited;
            notify(obs, StepType::Visit, c, parent[c]);
            if (c == goal) { r.found = true; break; }
            for (int nb : g.links(c)) {
                double ng = gScore[c] + 1;
                if (ng < gScore[nb]) {
                    gScore[nb] = ng;
                    parent[nb] = c;
                    open.push({ ng + g.distance(nb, goal), nb });
                }
            }
        }
        if (r.found) r.path = buildPath(parent, start, goal, obs);
        return r;
    }
};

// 3. Правило правої руки: не знаходить найкоротший шлях, працює лише в "ідеальних"
// лабіринтах (без циклів), майже не використовує пам'ять
class WallFollowerSolver : public SolverAlgorithm {
public:
    const char* name() const override { return "wallfollower"; }
    SolveResult solve(const Grid& g, int start, int goal, StepObserver* obs) override {
        SolveResult r;
        int n = g.cellCount();
        std::vector<bool> seen(n, false);
        std::vector<int> pos(n, -1);          // позиція клітинки в поточному шляху
        std::vector<int> path;
        int cur = start, heading = 0;
        seen[start] = true; r.visited = 1;
        path.push_back(start); pos[start] = 0;
        notify(obs, StepType::Visit, start, -1);

        long long limit = 8LL * n + 16;       // захист від зациклення
        while (cur != goal && limit-- > 0) {
            const auto& slots = g.neighbors(cur);
            int deg = static_cast<int>(slots.size());
            int moved = -1;
            // спочатку праворуч, потім прямо, потім ліворуч, нарешті назад
            for (int i = 0; i < deg && moved < 0; ++i) {
                int s = ((heading + 1 - i) % deg + deg) % deg;
                int nb = slots[s];
                if (nb >= 0 && g.isLinked(cur, nb)) { moved = s; }
            }
            if (moved < 0) break;             // ізольована клітинка
            int next = slots[moved];
            heading = moved;
            notify(obs, StepType::Visit, next, cur);
            if (!seen[next]) { seen[next] = true; ++r.visited; }
            // підтримуємо шлях без петель: повернулись у клітинку - відрізаємо хвіст
            if (pos[next] >= 0) {
                while (path.back() != next) { pos[path.back()] = -1; path.pop_back(); }
            } else {
                pos[next] = static_cast<int>(path.size());
                path.push_back(next);
            }
            cur = next;
        }
        if (cur == goal) {
            r.found = true;
            r.path = path;
            for (int c : path) notify(obs, StepType::Path, c, -1);
        }
        return r;
    }
};
