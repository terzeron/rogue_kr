#include "utils.h"
#include "rogue.h" // For coord type and RN macro
#include <stdlib.h> // For abs

int rnd(int range) {
    return range == 0 ? 0 : abs((int) RN) % range;
}

int roll(int number, int sides) {
    int dtotal = 0;
    while (number--)
        dtotal += rnd(sides) + 1;
    return dtotal;
}

int sign(int nm) {
    if (nm < 0)
        return -1;
    else if (nm > 0)
        return 1;
    return 0;
}

int spread(int nm) {
    return nm - nm / 2 + rnd(nm);
}

int dist(int y1, int x1, int y2, int x2) {
    int dx = abs(x1 - x2);
    int dy = abs(y1 - y2);
    return (dx > dy ? dx : dy);
}

int dist_cp(coord *c1, coord *c2) {
    return dist(c1->y, c1->x, c2->y, c2->x);
}
