#pragma once
#include <cstdlib>
#include "Grid.h"

// Прямокутна сітка. Слоти сусідів: 0=N, 1=E, 2=S, 3=W (за годинниковою стрілкою)
class RectGrid : public Grid {
public:
    RectGrid(int w, int h) : Grid(w, h) {
        initCells(w * h);
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x) {
                int c = y * w + x;
                slots_[c] = {
                    y > 0     ? c - w : -1,
                    x < w - 1 ? c + 1 : -1,
                    y < h - 1 ? c + w : -1,
                    x > 0     ? c - 1 : -1
                };
            }
    }
    double distance(int a, int b) const override {   // манхеттенська відстань
        return std::abs(a % width_ - b % width_) + std::abs(a / width_ - b / width_);
    }
    const char* typeName() const override { return "rect"; }
};
