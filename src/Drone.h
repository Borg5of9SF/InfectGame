#ifndef DRONE_H
#define DRONE_H

#include "Being.h"

class Machine;

class Drone : public Being
{
public:
    enum droneState_t
    {
        kStateAttack,   // on initial vessel attacking
        kStateConvertMachines,  // on initial vessel converting machinery
        kStateAssimilate,   // drone is assimilating (use _target)
        kStateInterface, // on cyb ship interfacing with machinery or other drones
        kStateWander,    // on cyb ship wandering
        kStateCopulation
    };
    
    enum droneType_t
    {
        kTypeAssimilation,
        kTypeEngineering,
        kTypeRepair,
        kTypeTactical
    };
    
    Drone();
    Drone(Being &from);
    virtual ~Drone() {}
    
    Being *target() { return _target; }
    Machine *machinery() { return _machinery; }
    static Room *findTeleporterRoom();

    virtual void attackedBy(Being *b);
    virtual void describe();
    virtual void tick();
    virtual std::string getHereString();
    void setState(droneState_t state);
    void setType(droneType_t type) { _type = type; }
    droneType_t type() { return _type; }
    
private:
    // If target is not null, the drone is in chase/attack mode. Though, it may also
    // camp in narrow corridors. Otherwise, it wanders and is drawn to technology.
    // The pathfinding might be intentionally bad (check neighboring rooms)...? Only follow if spotted.
    void doAssimilateString();
    void pickAssimilateTarget();
    void pickLimbToRemove();
    
    Being *_target;
    Machine *_machinery;
    int _interfaceCountDn;
    int _assimilationStage;
    
    droneState_t _state;
    droneType_t _type;
    
    void tickAttackMode();
    void tickMachinery();
    void tickWander();
    void tickInterface();
    void tickAssimilate();
    void followPath();
    
    // Conversion mode variables:
    int _portsToInstall;
    int _installCountDn;
    bool _installFaceplate;
    bodyPartLocation_t _workingOn;
    
    int _copulationCountDn;
};

#endif
