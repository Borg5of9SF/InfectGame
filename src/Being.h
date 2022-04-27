#ifndef BEING_H
#define BEING_H

#include "BodyPartDesc.h"
#include "Implant.h"
#include "Item.h"
#include "Room.h"
#include <stack>

class Being
{
public:
    // Needs bidirectional graph of how body parts connect.
    // Infection might work a little like game of life. Neighboring
    // parts are infected.
    // Implants also contain more nanites.
    
    Being();
    virtual ~Being();
    
    void goToRoom(Room *room);
    bool goToRoom(exits_t direction);
    Room *room() { return _curRoom; }
    
    void setGender(bool male) { male? _genderRating = -1.0 : 1.0; }
    float getGenderRating() { return _genderRating; }
    float getGenderDestination() { return _genderDestination; }
    void setNeedsAssimilation(bool a=true) { _needsAssimilation = a; }
    bool needsAssimilation() { return _needsAssimilation; }
    // Functions returning booleans are true on success

    bool tryInject(Being *target);
    bool tryShoot(Being *target);
    
    // Inject with nanoprobes. If total infect percent is 1.0, it does nothing.
    bool inject(bodyPartLocation_t location);
    
    // This removes the limb from that location downward
    bool installPort(bodyPartLocation_t loc);
    bool isPartMissing(bodyPartLocation_t loc);
    // Unsocket the port
    bool prosthesisInstalled(bodyPartLocation_t loc);

    // Print out the description of the assimilatee so far
    virtual void describe();
    void listItems();
    
    std::string posessive() { return _posessive; }
    std::string noun() { return _noun; }
    std::string name() { return _name; }
    std::string deadName() { return _deadName; }
    virtual std::string descName();
    virtual std::string getHereString() { return descName() + " is here."; }
    float health() { return _health; }
    
    void setIsPlayer() { _isPlayer = true; }
    bool isPlayer() { return _isPlayer; }
    float calcTotalInfectPercent();

    // Pass time
    virtual void tick();
    
    bool removePants();
    bool removeShirt();
    void giveIn() { _resistance = 0; }
    void markDelete() { _delete = true; }
    bool markedDelete() { return _delete; }
    float infectPercent(bodyPartLocation_t loc) { return _infectPercent[loc]; }
    bool isInRoomWithPlayer();
    void setOnPathTo(Room *r);
    void arouseDrone(float amt);

    virtual void attackedBy(Being *b) {}
    
    void addItem(Item *item)
    {
        _items.push_back(item);
    }
    
    // TODO: removeItem by class type and by instance
    
    
    
    virtual bool penetrate(Being &other);
    virtual void withdraw(Being &other);
    
    virtual void penetrated(Being *other) { _penetrateTarget = other; _requestCopulation = false; }
    bool copulationRequested() { return _requestCopulation; }

    void installTransceiver();

protected:
    // This is not a pure output function; it can set variables
    std::string describeInfection(bodyPartLocation_t location, int stage);
    void infectPart(bodyPartLocation_t loc, float increase);
    // If not empty, the being is following a path
    std::stack<Room*> _path;
    std::string sayString(std::string s);

    Room *_curRoom;
    float _resistance;
    float _infectPercent[kNumBodyParts];
    float _armor;
    float _health;
    
    bool _needsAssimilation;
    bool _leftEyeNeedsEyepiece;
    bool _rightEyeNeedsEyepiece;
    bool _voiceboxInstalled;
    bool _transceiverInstalled;
    bool _isPlayer;
    bool _delete;
    
    // These could be inventory later
    bool _legsBare;
    bool _chestBare;
    
    float _arousal;
    std::vector<bodyPartLocation_t> _socketed;
    std::vector<Implant> _implants;
    std::vector<Item*> _items;
    std::string _posessive;
    std::string _noun;
    std::string _name;
    std::string _deadName;

    Being *_penetrateTarget;
    bool _requestCopulation;
    
    float _genderRating;    // -male, +female, 0=neutral
    // These will be in the nanoprobe class later
    float _genderDestination;
};

#endif
