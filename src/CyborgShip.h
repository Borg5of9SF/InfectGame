#ifndef CYBSHIP_H
#define CYBSHIP_H

#define kCybDecks 2
#define kCybArmLen 4
// We'll have to bump the iterations up a lot once we put it on a grid!
// Also, iterations = longest path you can have
#define kCybIterations 4

#include <vector>

class Room;

typedef std::vector<Room*> cybDeck_t;
extern std::vector<cybDeck_t> gCybShipRooms;

void initCyborgShip();

#endif
