"""Швидка перевірка C++ ядра без GUI. Запуск: python check_core.py (з папки cpp_core)."""
import ctypes as C
import os
import sys

name = "maze.dll" if sys.platform.startswith("win") else "libmaze.so"
path = os.path.join(os.path.dirname(os.path.abspath(__file__)), name)
if not os.path.exists(path):
    sys.exit("FAIL: " + name + " not found. Build it first.")

L = C.CDLL(path)
vp, ci, pi = C.c_void_p, C.c_int, C.POINTER(C.c_int)
L.maze_create.restype = vp
L.maze_generate.argtypes = [vp, C.c_char_p, ci, ci, C.c_char_p, C.c_uint]
L.maze_solve.argtypes = [vp, C.c_char_p, ci, ci]
L.maze_last_error.argtypes = [vp]
L.maze_last_error.restype = C.c_char_p
for f in ("maze_edge_count", "maze_path_length", "maze_gen_step_count", "maze_solve_step_count"):
    getattr(L, f).argtypes = [vp]
L.maze_get_edges.argtypes = [vp, pi]
L.maze_get_path.argtypes = [vp, pi]
L.maze_get_stats.argtypes = [vp, C.POINTER(C.c_double)]

h = C.c_void_p(L.maze_create())
W, H = 30, 20
n = W * H
failed = False


def check(cond, text):
    global failed
    print(("  OK   " if cond else "  FAIL ") + text)
    failed = failed or not cond


for gen in ("backtracker", "prim", "kruskal"):
    print("Generator:", gen)
    check(L.maze_generate(h, b"rect", W, H, gen.encode(), 7) == 0, "generate returned 0")
    ne = L.maze_edge_count(h)
    check(ne == n - 1, "edges = %d (expected %d)" % (ne, n - 1))
    buf = (C.c_int * (2 * ne))()
    L.maze_get_edges(h, buf)
    adj = {i: [] for i in range(n)}
    for i in range(ne):
        a, b = buf[2 * i], buf[2 * i + 1]
        adj[a].append(b)
        adj[b].append(a)
    seen, stack = {0}, [0]
    while stack:
        for x in adj[stack.pop()]:
            if x not in seen:
                seen.add(x)
                stack.append(x)
    check(len(seen) == n, "maze is connected (%d of %d cells reachable)" % (len(seen), n))
    lengths = []
    for s in ("bfs", "astar", "wallfollower"):
        L.maze_solve(h, s.encode(), 0, n - 1)
        pl = L.maze_path_length(h)
        p = (C.c_int * pl)()
        L.maze_get_path(h, p)
        valid = pl > 0 and p[0] == 0 and p[pl - 1] == n - 1 and \
            all(p[i + 1] in adj[p[i]] for i in range(pl - 1))
        st = (C.c_double * 6)()
        L.maze_get_stats(h, st)
        check(valid, "%-12s path=%3d visited=%3d" % (s, pl, st[3]))
        lengths.append(pl)
    check(len(set(lengths)) == 1, "all solvers found the same path length")

print("Error handling:")
check(L.maze_generate(h, b"hex", 5, 5, b"prim", 1) == -1, "unknown grid -> -1 (" +
      L.maze_last_error(h).decode() + ")")
check(L.maze_solve(h, b"bfs", 0, 99999) == -1, "bad goal -> -1 (" +
      L.maze_last_error(h).decode() + ")")

print()
print("RESULT:", "SOME CHECKS FAILED" if failed else "ALL CHECKS PASSED")
sys.exit(1 if failed else 0)
