// C-міст між C++ ядром і Python (ctypes). Весь API - extern "C" над MazeFacade.
#include <cstring>
#include <exception>
#include "MazeFacade.h"

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif

static MazeFacade* F(void* h) { return static_cast<MazeFacade*>(h); }

static void writeSteps(const StepRecorder& rec, int* out) {
    for (size_t i = 0; i < rec.steps.size(); ++i) {
        out[3 * i]     = static_cast<int>(rec.steps[i].type);
        out[3 * i + 1] = rec.steps[i].a;
        out[3 * i + 2] = rec.steps[i].b;
    }
}

extern "C" {

EXPORT void* maze_create() { return new MazeFacade(); }
EXPORT void  maze_destroy(void* h) { delete F(h); }

// Повертає 0 при успіху, -1 при помилці (текст - у maze_last_error)
EXPORT int maze_generate(void* h, const char* gridType, int w, int hh,
                         const char* algo, unsigned seed) {
    try { F(h)->generate(gridType, w, hh, algo, seed); return 0; }
    catch (const std::exception& e) { F(h)->lastError = e.what(); return -1; }
}
EXPORT int maze_solve(void* h, const char* algo, int start, int goal) {
    try { F(h)->solve(algo, start, goal); return 0; }
    catch (const std::exception& e) { F(h)->lastError = e.what(); return -1; }
}
EXPORT const char* maze_last_error(void* h) { return F(h)->lastError.c_str(); }

EXPORT int maze_cell_count(void* h) { return F(h)->grid ? F(h)->grid->cellCount() : 0; }

// Ребра лабіринту: пари (a, b), a < b
EXPORT int maze_edge_count(void* h) {
    if (!F(h)->grid) return 0;
    int cnt = 0;
    for (int c = 0; c < F(h)->grid->cellCount(); ++c)
        for (int n : F(h)->grid->links(c)) if (c < n) ++cnt;
    return cnt;
}
EXPORT void maze_get_edges(void* h, int* out) {
    int i = 0;
    for (int c = 0; c < F(h)->grid->cellCount(); ++c)
        for (int n : F(h)->grid->links(c)) if (c < n) { out[i++] = c; out[i++] = n; }
}

// Кроки генерації / розв'язання: трійки (type, a, b)
EXPORT int  maze_gen_step_count(void* h) { return (int)F(h)->genSteps.steps.size(); }
EXPORT void maze_get_gen_steps(void* h, int* out) { writeSteps(F(h)->genSteps, out); }
EXPORT int  maze_solve_step_count(void* h) { return (int)F(h)->solveSteps.steps.size(); }
EXPORT void maze_get_solve_steps(void* h, int* out) { writeSteps(F(h)->solveSteps, out); }

EXPORT int  maze_path_length(void* h) { return (int)F(h)->result.path.size(); }
EXPORT void maze_get_path(void* h, int* out) {
    std::memcpy(out, F(h)->result.path.data(), F(h)->result.path.size() * sizeof(int));
}

// out[6]: deadEnds, branchPoints, pathLength, visitedCells, genTimeMs, solveTimeMs
EXPORT void maze_get_stats(void* h, double* out) {
    const Statistics& s = F(h)->stats;
    out[0] = s.deadEnds; out[1] = s.branchPoints; out[2] = s.pathLength;
    out[3] = s.visitedCells; out[4] = s.genTimeMs; out[5] = s.solveTimeMs;
}

}
