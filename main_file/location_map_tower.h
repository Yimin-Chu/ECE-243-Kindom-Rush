#ifndef COORDINATES_H
#define COORDINATES_H

// Define a struct for (x, y) coordinates
typedef struct {
    int x;
    int y;
} Coordinate;

// Declare the array of block positions (extern since it's defined in a .c file)
extern Coordinate block_positions[11];

#endif // COORDINATES_H
