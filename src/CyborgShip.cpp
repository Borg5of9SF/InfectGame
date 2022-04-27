#include <math.h>
#include <string.h>
#include "CyborgShip.h"
#include "System.h"
#include "Infect.h"
#include "Room.h"
#include "Drone.h"

// Replace the grid generation system with a recursive walk, like a reverse path find
// that makes rooms:
// Roll for # of exits, then call make room on them. Do this until the # of desired rooms is filled

#define kCybShipH 8
#define kCybShipW 8

bool gCybShipGrid[kCybShipH][kCybShipW];
int gCybEngineering = 0;
int gCybConversion = 0;
int gCybRepair = 0;
int gCybStorage = 0;
int gSectorNumber=0;

std::vector<cybDeck_t> gCybShipRooms;

std::string cybAddrString(int x, int y, int z)
{
    char ret[16];
    snprintf(ret, 16, "Node0x%x%x%x", gSectorNumber, z, y*16 + x);
    return std::string(ret);
}

class Connective : public Room
{
public:
    Connective(std::string title, std::string shortTitle, std::string desc):
        Room(title, shortTitle, desc)
        {
            _x = _y = _z = 0;
        }
    virtual ~Connective() {}
    void setCoordinates(int x, int y, int z) {_x=x; _y=y; _z=z;}
    
protected:
    virtual void connectedTo(Room *r)
    {
        int con=0;

        std::string s = cybAddrString(_x, _y, _z);
        _shortTitle = s;

        for (int i=0; i < kMaxExits; i++)
        {
            if (_exits[i])
                con++;
        }
        if (con==1)
        {
            _title = "Access Point (" + s + ")";
            _description = "A small nook only large enough for two drones. An access terminal is mounted on the wall.";
        }
        else if (con==2)
        {
            _title = "Connective (" + s + ")";
            _description = "A corridor connecting two cyborg ship nodes. It narrow and winding with a low ceiling, and only wide enough for two drones.";
        }
        else
        {
            std::string s = cybAddrString(_x, _y, _z);
            _title = "Junction (" + s + ")";
            _description = "A junction where multiple rooms meet. There are some regen stations along the walls and a terminal in the center of the room.";
        }
    }
    float _x;
    float _y;
    float _z;
};


void getExitVector(exits_t e, int &x, int &y)
{
    if (e == kExitWest)
    {
        x = -1;
        y = 0;
    }
    else if (e == kExitEast)
    {
        x = 1;
        y = 0;
    }
    else if (e == kExitNorth)
    {
        x = 0;
        y = -1;
    }
    else if (e == kExitSouth)
    {
        x = 0;
        y = 1;
    }
}

void addExitVector(exits_t e, int &x, int &y)
{
    int vx;
    int vy;
    getExitVector(e, vx, vy);
    x += vx;
    y += vy;
}

std::string appendOuterXY()
{
    return " You are at the edge of your cluster's permitted sector, though you can sense many sectors surrounding this one for miles.";
}

Room *initCybEngineering(int x, int y, int z)
{
    std::string s = cybAddrString(x, y, z);
    std::string title = "Engineering (" + s + ")";
    std::string desc = "A cramped, maze-like engineering room not too dissimilar from a corridor. Drones are working here, their manipulators engaging and disengaging from various terminals and other machinery. ";
 
    if (x==0 || x == kCybShipW-1 || y == 0 || y == kCybShipH-1)
        desc += appendOuterXY();
    
        
    Room *r = new Room(title, s, desc);

    gCybEngineering++;
    
    return r;
}

Room *initCybEngineering2(int x, int y, int z)
{
    std::string s = cybAddrString(x, y, z);
    std::string title = "Engineering (" + s + ")";
    std::string desc = "A support room for engineering drones. It is dense with drone regen stations. In the center of the room is a terminal.";

//    if (x==0 || x == kCybShipW-1 || y == 0 || y == kCybShipH-1)
//        desc += appendOuterXY();

    Room *r = new Room(title, s, desc);

    gCybEngineering++;
    
    return r;
}

Room *initCybConversion(int x, int y, int z)
{
    std::string s = cybAddrString(x, y, z);
    std::string title = "Conversion (" + s + ")";
    std::string desc = "A conversion chamber. The space is open, and there are several conversion racks here. Along the wall are conversion drone regen stations.";

    if (x==0 || x == kCybShipW-1 || y == 0 || y == kCybShipH-1)
        desc += appendOuterXY();

    Room *r = new Room(title, s, desc);

    gCybConversion++;
    
    return r;
}

Room *initCybRepair(int x, int y, int z)
{
    std::string s = cybAddrString(x, y, z);
    std::string title = "Repair (" + s + ")";
    std::string desc = "A cyborg repair room. The space is more open, and there are several repair racks here. Some are occupied by drones in various states of disassembly. Along the wall are repair drone regen stations.";
    
    if (x==0 || x == kCybShipW-1 || y == 0 || y == kCybShipH-1)
        desc += appendOuterXY();

    Room *r = new Room(title, s, desc);

    gCybRepair++;
    
    return r;
}

Room *initCybStorage(int x, int y, int z)
{
    std::string s = cybAddrString(x, y, z);
    std::string title = "Repair (" + s + ")";
    std::string desc = "A cyborg storage room. The ceiling is higher than the other rooms, and there are two rows of drone regen stations, one stacked upon the other.";

    if (x==0 || x == kCybShipW-1 || y == 0 || y == kCybShipH-1)
        desc += appendOuterXY();

    Room *r = new Room(title, s, desc);

    gCybStorage++;
    
    return r;
}

Room *initCybCorridor(int x, int y, int z)
{
    Connective *c = new Connective("", "", "");
    c->setCoordinates(x, y, z);
    
    return c;
}

bool dirInBounds(exits_t dir, int x, int y)
{
    int vx = x;
    int vy = y;
    addExitVector(dir, vx, vy);

    if (x < 0)
        return false;
    if (x >= kCybShipW)
        return false;
    if (y < 0)
        return false;
    if (y >= kCybShipH)
        return false;
    
    return true;
}

Room *rollRoom(int x, int y, int z)
{
    Room *r;
    float rol = roll();
    
    if (rol < .2)
    {
        if (roll() < .5)
            r = initCybEngineering(x, y, z);
        else
            r = initCybEngineering2(x, y, z);
    }
    else if (rol < .4)
    {
        r = initCybConversion(x, y, z);
    }
    else if (rol < .6)
    {
        r = initCybRepair(x, y, z);
    }
    else if (rol < .7)
    {
        r = initCybStorage(x, y, z);
    }
    else
    {
        r = initCybCorridor(x, y, z);
    }
    
    if (roll() < .4)
    {
        Drone *d = new Drone();
        d->setState(Drone::kStateWander);
        d->goToRoom(r);
        gBeings.push_back(d);
    }
    
    return r;
}

bool rollUnusedExit(int x, int y, exits_t &ret)
{
    std::vector<int> validExits;
    for (int i=0; i < 4; i++)    // no up/dn
    {
        int vx = x;
        int vy = y;
        addExitVector((exits_t)i, vx, vy);

        // Check if we're on the grid and the x/y isn't used
        if (vx >= 0 && vx < kCybShipW
            && vy >= 0 && vy < kCybShipH
            && !gCybShipGrid[vy][vx])
        {
            validExits.push_back(i);
        }
    }

    if (validExits.empty())
        return false;

    ret = (exits_t)validExits[pick(0, (int)validExits.size()-1)];
    return true;
}

void addRooms(cybDeck_t &deck, Room *newRoom, int x, int y, int iteration)
{
    // Find how many rooms to add
    // The # of rooms to add must be a repeating pattern to get the same number of
    // rooms on each randomization. This will appear pseudorandom because each time
    // the # of rooms also increases by this amount!
    int roomsToAdd = (deck.size() % 3) + 1; //pick(1, 3);

    for (int i=0; i < roomsToAdd; i++)
    {
        // For each one, add a new Room*, connect it, then call addRoomToDeck for it to add its own children

        // TODO: Build a 2d grid so we don't have non-euclidean layouts!
        exits_t e;
        if (rollUnusedExit(x, y, e))
        {
            int vx = x;
            int vy = y;
            addExitVector(e, vx, vy);
            Room *r = rollRoom(vx, vy, 0);
            deck.push_back(r);
            gCybShipGrid[vy][vx] = true; // mark space used BEFORE addRooms
            newRoom->connectTo(*r, e);
            addRooms(deck, r, vx, vy, iteration+1);
        }
    }
}

void initCyborgShip()
{
    gSectorNumber = rand() & 255;
    
    memset(gCybShipGrid, 0, sizeof(bool)*kCybShipW*kCybShipH);
    
    gCybShipRooms.push_back(cybDeck_t());
    int x = kCybShipW>>1;
    int y = kCybShipH>>1;
    Room *r = initCybConversion(x, y, 0);
    // 2 assimilation drones
    for (int i=0; i < 2; i++)
    {
        Drone *d = new Drone();
        d->setType(Drone::kTypeAssimilation);
        d->setState(Drone::kStateAssimilate);
        gBeings.push_back(d);
        d->goToRoom(r);
    }
    gCybShipRooms[0].push_back(r);
    gCybShipGrid[y][x] = true;
    addRooms(gCybShipRooms[0], gCybShipRooms[0][0], y, x, 0);
//    printf("Ship has %d rooms\n", (int)gCybShipRooms[0].size());
}
