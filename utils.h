#ifndef ROGUE_UTILS_H
#define ROGUE_UTILS_H

#include "rogue.h" // For coord type

/* Function declarations for common utilities */
int rnd(int range);
int roll(int number, int sides);
int sign(int nm);
int spread(int nm);
int dist(int y1, int x1, int y2, int x2);
int dist_cp(coord *c1, coord *c2);

#endif // ROGUE_UTILS_H
