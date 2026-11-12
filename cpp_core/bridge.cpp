#include <vector>
#include <stack>
#include <random>
#include <algorithm>

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

extern "C" {

    // Заповнює масив out (розмір w*h) бітами відкритих проходів
    EXPORT void generate_maze(int w, int h, unsigned seed, unsigned char* out) {
        std::fill(out, out + w * h, 0);
        std::vector<bool> visited(w * h, false);
        std::mt19937 rng(seed);

        const int dx[4] = { 0, 1, 0, -1 };   // N, E, S, W
        const int dy[4] = { -1, 0, 1, 0 };
        const int bit[4] = { 1, 2, 4, 8 };

        std::stack<int> st;
        st.push(0);
        visited[0] = true;

        while (!st.empty()) {
            int cur = st.top();
            int x = cur % w, y = cur / w;

            std::vector<int> options;                 // куди можна піти
            for (int d = 0; d < 4; ++d) {
                int nx = x + dx[d], ny = y + dy[d];
                if (nx >= 0 && nx < w && ny >= 0 && ny < h && !visited[ny * w + nx])
                    options.push_back(d);
            }

            if (options.empty()) { st.pop(); continue; }   // тупик: повертаємось

            int d = options[rng() % options.size()];
            int next = (y + dy[d]) * w + (x + dx[d]);
            out[cur] |= bit[d];                 // пробиваємо стінку в обидва боки
            out[next] |= bit[(d + 2) % 4];
            visited[next] = true;
            st.push(next);
        }
    }

}