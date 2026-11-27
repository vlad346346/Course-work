#pragma once
#include <vector>

// Тип події, яку алгоритм повідомляє спостерігачу (для покрокової візуалізації)
enum class StepType : int {
    Carve = 0,   // генерація: пробито прохід між клітинками a і b
    Visit = 1,   // пошук шляху: алгоритм відвідав клітинку a (b - звідки прийшов, або -1)
    Path  = 2    // фінальний шлях: клітинка a належить знайденому шляху
};

struct Step {
    StepType type;
    int a;
    int b;
};

// Observer: алгоритм нічого не знає про графіку, він лише сповіщає слухача
class StepObserver {
public:
    virtual ~StepObserver() = default;
    virtual void onStep(const Step& s) = 0;
};

// Простий слухач, який запам'ятовує всі кроки (їх потім забирає Python)
class StepRecorder : public StepObserver {
public:
    void onStep(const Step& s) override { steps.push_back(s); }
    void clear() { steps.clear(); }
    std::vector<Step> steps;
};

struct SolveResult {
    std::vector<int> path;   // клітинки шляху від старту до фінішу
    int visited = 0;         // скільки різних клітинок відвідано
    bool found = false;
};

struct Statistics {
    int deadEnds = 0;        // клітинки з одним проходом
    int branchPoints = 0;    // клітинки з 3+ проходами
    int pathLength = 0;
    int visitedCells = 0;
    double genTimeMs = 0;
    double solveTimeMs = 0;
};
