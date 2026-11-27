#pragma once
#include <vector>
#include <algorithm>

// Абстрактна сітка. Лабіринт = граф: клітинки - вершини, пробиті проходи - ребра.
// Усі алгоритми працюють лише з цим інтерфейсом, тому нова форма сітки
// (гекс, коло...) вимагає тільки нового нащадка, без змін в алгоритмах (OCP, LSP).
class Grid {
public:
    Grid(int w, int h) : width_(w), height_(h) {}
    virtual ~Grid() = default;

    int width() const { return width_; }
    int height() const { return height_; }
    int cellCount() const { return static_cast<int>(slots_.size()); }

    // Можливі сусіди в циклічному порядку (за годинниковою стрілкою); -1 = межа сітки
    const std::vector<int>& neighbors(int c) const { return slots_[c]; }

    // Лише існуючі сусіди
    std::vector<int> existingNeighbors(int c) const {
        std::vector<int> r;
        for (int n : slots_[c]) if (n >= 0) r.push_back(n);
        return r;
    }

    // Проходи (пробиті стіни)
    void link(int a, int b) {
        if (isLinked(a, b)) return;
        links_[a].push_back(b);
        links_[b].push_back(a);
    }
    bool isLinked(int a, int b) const {
        return std::find(links_[a].begin(), links_[a].end(), b) != links_[a].end();
    }
    const std::vector<int>& links(int c) const { return links_[c]; }
    void clearLinks() { for (auto& l : links_) l.clear(); }

    // Евристика для A*: оцінка відстані між клітинками
    virtual double distance(int a, int b) const = 0;
    virtual const char* typeName() const = 0;

protected:
    void initCells(int count) {
        slots_.assign(count, {});
        links_.assign(count, {});
    }
    int width_, height_;
    std::vector<std::vector<int>> slots_;
    std::vector<std::vector<int>> links_;
};
