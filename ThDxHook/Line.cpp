#include "Line.h"
#include "stdafx.h"
void swap(int &a, int &b) {
    int tmp = a;
    a = b;
    b = tmp;
}
// bresenham algorithm to improve movement
void Line(int x1, int y1, int x2, int y2, int &rx, int &ry) {
    const bool steep = (abs(y2 - y1) > abs(x2 - x1));
    if (steep == true) {
        swap(x1, y1);
        swap(x2, y2);
    }

    if (x1 > x2) {
        swap(x1, x2);
        swap(y1, y2);
    }

    int dx = x2 - x1;
    int dy = abs(y2 - y1);

    float error = dx / 2.0f;
    const int ystep = (y1 < y2) ? 1 : -1;
    int y = y1;

    const int maxX = x2;
    int x = x1;
    for (; x == x1; x++) {
        error -= dy;
        if (error < 0) {
            y += ystep;
            // error += dx;
        }
    }
    rx = 0;
    if (y != y1)
        ry = 0;
    if (steep)
        swap(rx, ry);
}