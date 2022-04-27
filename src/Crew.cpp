#include "Crew.h"
#include "System.h"
#include "Infect.h"
#include "Drone.h"
#include "NameHat.h"

Crew::Crew()
{
    if (rand()&1)
    {
        _genderRating = 1.0;
        _name = getHumanName(false);
        _noun = "she";
        // The balance has changed...
        const float genderDestinations[4] = { -1.0, 1.0, 0, 0 };
        _genderDestination = genderDestinations[pick(0, 3)];
    }
    else
    {
        _genderRating = -1.0;
        _name = getHumanName(true);
        _noun = "he";
    }

    if (_name[_name.size()-1] == 's')
        _posessive = _name + "'";
    else
        _posessive = _name + "'s";

    _resistance = .5;
    _armor = .1;

    for (int i=0; i < kNumBodyParts; i++)
        _infectPercent[i] = 0;
}

void Crew::tick()
{
    // If a crewman is being targeted by a drone and they are in the same room, they will attack. If they are in a neighboring room,
    // they will run in
    if (calcTotalInfectPercent() > .7)
    {
        // Just stay there, transforming
        if (isInRoomWithPlayer())
            iOut(std::string(_name + " stares dead ahead, as if in a trance.").c_str());
        Being::tick();
        return;
    }
    
    // Random chance that they will skip a turn
    if (roll() < .2)
        return;
    
    std::vector<Being*> beings = _curRoom->beings();
    for (int i=0; i < beings.size(); i++)
    {
        Being *be = beings[i];
        Drone *d = dynamic_cast<Drone*>(be);
        if (d)
        {
            if (d->target() && d->health() >= 0)
            {
                tryShoot(d);
                Being::tick();
                return;
            }
        }
        
        // Target the player
        if (beings[i] == gPlayer)
        {
            if (gPlayer->deadName() != "")
            {
                tryShoot(gPlayer);
                Being::tick();
                return;
            }
        }
    }
    for (int j=0; j < kMaxExits; j++)
    {
        Room *r = _curRoom->getExitRoom((exits_t)j);
        if (r)
        {
            std::vector<Being*> beings = r->beings();
            for (int i=0; i < beings.size(); i++)
            {
                Being *be = beings[i];
                Drone *d = dynamic_cast<Drone*>(be);
                if ((d && d->target()) || (beings[i]->isPlayer() && beings[i]->deadName() != ""))
                {
//                    printf("%s RUNS TO %s\n", _name.c_str(), r->shortTitle().c_str());
                    goToRoom(r);
                    Being::tick();
                    return;
                }
            }
        }
    }
    
    // Wander mode
    if (roll() < .5)
    {
        std::vector<Room*> exits;
        for (int j=0; j < kMaxExits; j++)
        {
            Room *r = _curRoom->getExitRoom((exits_t)j);
            if (r)
                exits.push_back(r);
        }
        
        Room *r = exits[rand() % exits.size()];
        if (isInRoomWithPlayer())
            iOut(std::string(name() + " heads to " + r->shortTitle() + ".").c_str());
        goToRoom(r);
        if (isInRoomWithPlayer())
            iOut(std::string(descName() + " enters the room.").c_str());
    }
    
    if (isInRoomWithPlayer())
    {
        int c = countDrones();
        if ((rand() & 7) == 0)
        {
            if (rand() & 1)
            {
                if (c == 1)
                    iOut(sayString("C'mon. We've gotta find that damn drone before things get any worse.").c_str());
                else if (c > 1)
                    iOut(sayString("C'mon. We've gotta find those damn drones before things get any worse.").c_str());
            }
            else
            {
                iOut(sayString("They're probably after our technology.").c_str());
            }
        }
    }
    
    Being::tick();
    
/*    else
    {
        Drone::tick();
    }
 */
}
