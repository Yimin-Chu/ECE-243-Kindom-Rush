#ifndef COIN_H
#define COIN_H

#include <stdbool.h>

// Coin module for managing in-game currency.
// Initial coin count is 100. When a monster is killed, add 20 coins.
// Placing a tower costs 40 coins. The coin count is displayed on HEX0 (ones),
// HEX1 (tens) and HEX2 (hundreds).

void display_3digits_on_HEX(int value);
// Initializes the coin counter (sets it to 100) and displays it.
void init_coin(void);

// Adds 20 coins and updates the display.
void add_coin(void);

// Attempts to spend 40 coins for a tower.
// Returns true if the purchase is successful (coins >= 40),
// otherwise returns false (insufficient coins).
bool spend_coin(void);

// Displays the current coin count on HEX0, HEX1, and HEX2.
void display_coin(void);

// Returns the current coin count.
int get_coin(void);

// Returns true if there are enough coins (>=40) to place a tower.
bool can_place_tower(void);

#endif // COIN_H
