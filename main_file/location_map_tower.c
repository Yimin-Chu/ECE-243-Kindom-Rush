#include "location_map_tower.h"

typedef struct {
    int x;
    int y;
} Coordinate;

Coordinate block_positions[11] = {
    {4, 37},  // Block 1
    {4, 85},  // Block 2
    {4, 133},  // Block 3
    {74, 60},  // Block 4
    {74, 107},  // Block 5
    {74, 154},  // Block 6
    {147, 60}, // Block 7
    {147, 107}, // Block 8
    {220, 60}, // Block 9
    {220, 107}, // Block 10
    {220, 154}  // Block 11
};