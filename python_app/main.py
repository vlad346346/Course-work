import ctypes, sys, os

lib_name = {"win32": "maze.dll", "darwin": "libmaze.dylib"}.get(sys.platform, "libmaze.so")
lib = ctypes.CDLL(os.path.join(os.path.dirname(__file__), "..", "cpp_core", lib_name))

lib.generate_maze.argtypes = [ctypes.c_int, ctypes.c_int, ctypes.c_uint,
                              ctypes.POINTER(ctypes.c_ubyte)]

N, E, S, W = 1, 2, 4, 8

def generate(w, h, seed=42):
    buf = (ctypes.c_ubyte * (w * h))()
    lib.generate_maze(w, h, seed, buf)
    return list(buf)

def draw(cells, w, h):
    rows = [["#"] * (2 * w + 1) for _ in range(2 * h + 1)]
    for y in range(h):
        for x in range(w):
            c = cells[y * w + x]
            rows[2*y + 1][2*x + 1] = " "
            if c & E: rows[2*y + 1][2*x + 2] = " "
            if c & S: rows[2*y + 2][2*x + 1] = " "
    print("\n".join("".join(r) for r in rows))

if __name__ == "__main__":
    w, h = 15, 8
    draw(generate(w, h), w, h)