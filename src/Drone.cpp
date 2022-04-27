#include "Drone.h"
#include "System.h"
#include "ImplantDesc.h"
#include "Infect.h"
#include "NameHat.h"
#include "Machine.h"
#include "CyborgShip.h"

Drone::Drone()
{
    _name = makeDroneName(false);
    _noun = "it";
    _posessive = "its";
    _target = 0;
    _machinery = 0;
    _voiceboxInstalled = true;
    _transceiverInstalled = true;
    _state = kStateConvertMachines;
    _genderRating = pick(-1, 1);
    _interfaceCountDn = 0;
    _needsAssimilation = false;
    _installCountDn = 0;
    _copulationCountDn = 0;

    for (int i=0; i < kNumBodyParts; i++)
    {
        _infectPercent[i] = 1.0;
    }
    
    // Implants
    StandardEyepiece *se = new StandardEyepiece();
    int eye = rand() & 1;
    if (eye)
        se->install(*this, kBPEyeL);
    else
        se->install(*this, kBPEyeR);
    
    EngineerManipulator *em = new EngineerManipulator();
    if (rand()&1)
        em->install(*this, kBPArmL);
    else
        em->install(*this, kBPArmR);
    
    if ((rand()&3) == 0)
    {
        SupplementaryEyepiece *se = new SupplementaryEyepiece();
        if (eye)
            se->install(*this, kBPEyeR);
        else
            se->install(*this, kBPEyeL);
    }
    
    _type = (droneType_t)pick(0, 3);
}

Drone::Drone(Being &from)
{
    if (from.deadName().empty())
    {
        _deadName = from.name();
        _name = makeDroneName(true);
        _posessive = descName() + "'s";
    }
    else
    {
        _deadName = from.deadName();
        _name = from.name();
        _posessive = from.name() + "'s";
    }
    
    _noun = "it";
    _target = 0;
    _machinery = 0;
    _curRoom = from.room();
    _voiceboxInstalled = true;
    _transceiverInstalled = true;
    _state = kStateConvertMachines;
    _interfaceCountDn = 0;
    _genderRating = from.getGenderRating();
    _genderDestination = from.getGenderDestination();
    _type = (droneType_t)pick(0, 3);
    _needsAssimilation = true;
    _installCountDn = 0;
    _copulationCountDn = 0;

    for (int i=0; i < kNumBodyParts; i++)
    {
        _infectPercent[i] = 1.0;
    }

    from.room()->addBeing(this);
    for (int i=0; i < kNumBodyParts; i++)
    {
        _infectPercent[i] = from.infectPercent((bodyPartLocation_t)i);
    }
}

void Drone::tickAttackMode()
{
    // If you kill a target, this might crash :) but that is disallowed, so...
    if (_target && _target->deadName() != "")
        _target = 0;
    
    if (_curRoom)
    {
        std::vector<Being*> beings = _curRoom->beings();
        
        if (_target && _target->calcTotalInfectPercent() > .4)
            _target = NULL; // pick a new one
        
        if (!_target)
        {
            // Pick a new target if possible
            std::vector<Being*> targets;
            for (int i=0; i < beings.size(); i++)
            {
                Being *be = beings[i];
                Drone *d = dynamic_cast<Drone*>(be);
                if (beings[i]->deadName().empty() && !dynamic_cast<Drone*>(d))
                    targets.push_back(beings[i]);
            }
            
            if (targets.size())
            {
                // Maybe wait a turn before injection if the player is spotted
                int i = rand() % targets.size();
                _target = targets[i];
                if (isInRoomWithPlayer())
                {
                    std::string s;
                    int x = rand()&3;
                    if (x == 0)
                        s = sayString("Biological organism designation " + _target->name() + ". You will be brought into perfection.");
                    else if (x == 1)
                        s = sayString("Biological organism designation " + _target->name() + ". You must comply.");
                    else if (x == 2)
                        s = sayString("Biological organism designation " + _target->name() + ". You will be converted.");
                    else if (x == 3)
                        s = sayString("Biological organism designation " + _target->name() + ". You will serve us.");
                    
                    iOut(s.c_str());
                }
            }
        }
        else
        {
            bool targetInRoom = false;
            for (int i=0; i < beings.size(); i++)
            {
                if (beings[i] == _target)
                {
                    // TODO: Move descriptors into the inject function. Make inject fn take a Being.
                    if (tryInject(_target))
                    {
                        if (_curRoom->neighborContainsBeing(gPlayer))
                        {
                            if (_target->calcTotalInfectPercent() == 0)
                                iOut("You hear a startled yelp from the next room.. %s?", _target->name().c_str());
                        }
                    }
                    
                    break;
                }
            }
            
            if (!targetInRoom)
            {
                // check surrounding rooms for target and move there if spotted. Otherwise, wait.
                for (int i=0; i < kMaxExits; i++)
                {
                    Room *r = _curRoom->getExitRoom((exits_t)i);
                    if (r)
                    {
                        // TODO: a containsBeing function
                        beings = r->beings();
                        
                        for (int j=0; j < beings.size(); j++)
                        {
                            if (beings[j] == _target)
                            {
                                // Sometimes the drone will wait a turn, even though it will
                                // mostly pursue. Waiting allows it to feel smarter than it is,
                                // and gives it a chance to inject rather than walk during some turns
                                if ((rand() & 3) != 0)
                                {
                                    // TODO: name for direction fn would help
                                    //                                   std::string s = "You hear servo noises as " + name() + " follows " + _target->noun() + " into " + r->shortTitle() + ".";
                                    goToRoom(r);
                                    if (isInRoomWithPlayer())
                                    {
                                        std::string s = "You hear servo noises as " + descName() + " comes into view.";
                                        iOut(s.c_str());
                                    }
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void Drone::followPath()
{
    Room *r = _path.top();
    _path.pop();
    if (isInRoomWithPlayer())
        iOut(std::string(_name + " moves into " + r->shortTitle() + ". It takes no notice of you.").c_str());
    goToRoom(r);
    if (isInRoomWithPlayer())
        iOut(std::string(_name + " comes into view, expression blank, marching towards some destination...").c_str());
}

void Drone::tickMachinery()
{
    if (!_machinery)
    {
        if (_path.empty())
        {
            // Build list of possible target rooms
            std::vector<Room*> targetRooms;
            for (int i=0; i < gRooms.size(); i++)
            {
                Room *r = gRooms[i];
                std::vector<Item*> items = r->items();
                for (int j=0; j < items.size(); j++)
                {
                    // Contains an assimilatable machine?
                    Item *candidate = items[j];
                    Machine *m = dynamic_cast<Machine*>(candidate);
                    if (m && m->converted() < 1.0)
                    {
                        targetRooms.push_back(gRooms[i]);
                        break;
                    }
                }
            }
            
 //           printf("%s: CANDIDATE ROOMS SIZE %d.\n", _name.c_str(), (int)targetRooms.size());
            
            // Choose one
            if (targetRooms.size())
            {
                Room *r = targetRooms[pick(0, (int)targetRooms.size()-1)];
                _path = _curRoom->findPathTo(r);
            }
            else
            {
                if (isInRoomWithPlayer())
                    iOut("%s head moves slightly, its eyes unfocused. It appears to be receiving new instructions...", name().c_str());
                _state = kStateAttack;
            }
        }
        else
        {
            followPath();
            
            if (_path.empty()) // arrived
            {
                for (int i=0; i < _curRoom->items().size(); i++)
                {
                    // Contains an assimilatable machine?
                    Item *item = _curRoom->items()[i];
                    Machine *m = dynamic_cast<Machine*>(item);
                    if (m && m->converted() < 1.0)
                    {
                        if (isInRoomWithPlayer())
                        {
                            iOut(std::string(_name + " stops in front of the " + m->name() + ". It seems to have found what it's looking for. It then raises its manipulator and jacks into it, its expression perfectly calm and unchanging.").c_str());
                        }

    //                    printf("%s: ARRIVED AT DEST. ASSIGN MACHINERY %p.\n", _name.c_str(), m);
                        _machinery = m;
                    }
                }
            }
        }
    }
    else // if _machinery
    {
  //      printf("%s: MACHINERY %p\n", _name.c_str(), _machinery);
        if (_machinery->converted() < 1.0)
        {
            if (isInRoomWithPlayer())
            {
                iOut(std::string(_name + " continues to interface with the " + _machinery->name() + ". It seems to be adapting it for its own purposes.").c_str());
            }
            _machinery->convert(this);
        }
        else
        {
            if (isInRoomWithPlayer())
            {
                iOut(std::string(_name + " disengages from the " + _machinery->name() + " and pivots away from it. It is standing, its posture stiff.").c_str());
                iOut(_machinery->description().c_str());
            }
            _machinery = 0;
        }
    }
}

void Drone::describe()
{
    std::string s;
    if (_deadName != "")
        s = " This used to be " + _deadName + ".";
    else
        s = " Nobody you know.";
    
    if (_genderRating == -1)
        s += " Its original gender appears to have been male.";
    else if (_genderRating == 1)
        s += " Its original gender appears to have been female.";
    else
        s += " Its original gender is hard to discern.";
    
    if (_genderDestination == -1)
        s += " The drone's phallus hangs from its smooth crotch.";
    else if (_genderDestination == 0)
        s += " The drone's codpiece is smooth with just a small graceful bulge.";
    else if (_genderDestination == 1)
        s += " The drone is equipped with a synthetic vaginal opening.";
    
    std::string typeStr;
    if (_type == kTypeRepair)
        typeStr = "repair";
    else if (_type == kTypeTactical)
        typeStr = "tactical";
    else if (_type == kTypeEngineering)
        typeStr = "engineering";
    else if (_type == kTypeAssimilation)
        typeStr = "assimilation";
    
    if (_health >= 0)
        iOut(std::string(name() + " is a cyborg " + typeStr + " drone. You can hear the whirring and clicking of its machinery and electronics." + s).c_str());
    else
        iOut(std::string(name() + " is a cyborg " + typeStr + " drone. It is deactivated, but you can still hear some electronics functioning and a couple lights on its armor are slowly blinking. Some of its systems are quite clearly still online, even if it is in shutdown.." + s).c_str());
    iOut("Implants:");
    listItems();
}

void Drone::attackedBy(Being *b)
{
    if (_target)
        return;
    
    _state = kStateAttack;
    _target = b;
    if (isInRoomWithPlayer())
    {
        if (_machinery)
            iOut(std::string(name() + " disengates from the equipment and turns to face " + b->name() + ", its manipulator outstretched.").c_str());
        else
            iOut(std::string(name() + " pivots to face " + b->name() + ".").c_str());
        _machinery = 0;
    }
}

Room *Drone::findTeleporterRoom()
{
    for (int i=0; i < gRooms.size(); i++)
    {
        Room *r = gRooms[i];
        for (int j=0; j < r->items().size(); j++)
        {
            Item *i = r->items()[j];
            if (dynamic_cast<Teleporter*>(i))
            {
                return r;
            }
        }
    }
    
    return NULL;
}

std::string Drone::getHereString()
{
    if (_state == kStateInterface)
        return descName() + " is here, interfacing with a terminal.";
    else
        return Being::getHereString();
}

void Drone::tickInterface()
{
    if (--_interfaceCountDn <= 0)
    {
        if (isInRoomWithPlayer())
        {
            iOut("%s disengages from the terminal, lowering its manipulator arm and pivoting away from it.", descName().c_str());
            _interfaceCountDn = 0;
            _state = kStateWander;
        }
    }
}

void Drone::tickWander()
{
    float rol = roll();
    
    if (!_path.empty())
    {
        followPath();
    }
    else
    {
        // Aimless
        if (rol < .4)
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
                iOut(std::string(descName() + " comes into view.").c_str());
        }
        else if (rol < .5)
        {
            cybDeck_t deck = gCybShipRooms[0];
            int dest = pick(0, (int)deck.size()-1);
            if (deck[dest] != _curRoom)
                _path = _curRoom->findPathTo(deck[dest]);
        }
    }
    
    // If in room with player or other drone, chance it'll interface for a couple turns and then resume
    // If in room with machinery, chance it'll interface for a few turns
    if (roll() < .1)
    {
        if (isInRoomWithPlayer())
        {
            iOut("%s turns towards a terminal on the wall. With a whir, it raises its manipulator and plugs in.", descName().c_str());
            _state = kStateInterface;
            _interfaceCountDn = pick(5, 20);
        }
    }
}

void Drone::setState(droneState_t state)
{
    _state = state;
}

void Drone::doAssimilateString()
{
    if (!isInRoomWithPlayer())
        return;
    
    int desc;
    if (_target->isPlayer())
        desc = pick(0, 3);
    else
        desc = pick(0, 2);
    
    if (desc == 0)
    {
        iOut("%s continues to work on %s %s, its manipulator pivoting, injecting chemicals, and interfacing with implants.",
             name().c_str(),
             _target->posessive().c_str(),
             getBodyPart(_workingOn).name().c_str()
             );
    }
    else if (desc == 1)
    {
        iOut("%s continues to work on %s %s. There is something strangely tender about its movements.",
             name().c_str(),
             _target->posessive().c_str(),
             getBodyPart(_workingOn).name().c_str()
             );
    }
    else if (desc == 2)
    {
        iOut("%s continues to work on %s %s.",
             name().c_str(),
             _target->posessive().c_str(),
             getBodyPart(_workingOn).name().c_str()
             );
    }
    else
    {
        iOut("%s continues working on you. There is a dull sensation of pressure as your %s is moved and pulled.",
             name().c_str(),
             getBodyPart(_workingOn).name().c_str()
             );
    }

}

void Drone::pickAssimilateTarget()
{
    std::vector<Being*> candidates;
    const std::vector<Being*> &beings = _curRoom->beings();
    for (int i=0; i < beings.size(); i++)
    {
        Being *b = _curRoom->beings()[i];
        if (b->needsAssimilation())
        {
            bool droneIsTaken = false;
            // Make sure another drone isn't already working on this one
            for (int j=0; j < beings.size(); j++)
            {
                Drone *d = dynamic_cast<Drone*>(beings[j]);
                if (d)
                {
                    if (d->target() == b)
                    {
                        droneIsTaken = true;
                        break;
                    }
                }
            }
            
            if (!droneIsTaken)
                candidates.push_back(b);
        }
    }
    
    if (!candidates.empty())
    {
        _target = candidates[pick(0, (int)candidates.size()-1)];
        
        if (isInRoomWithPlayer())
            iOut(sayString("Drone " + _target->name() + ": Prepare to receive further modification.").c_str());
    }
}

void Drone::pickLimbToRemove()
{
    // Phase 1: Port the drone. If ports is 3 or 4, start with the legs.
    // If one leg is off, the other has to come off (so, 3 means 2 legs + 1 manipulator).
    std::vector<bodyPartLocation_t> portCandidates;
    if (_portsToInstall == 4)
    {
        //                portCandidates.push_back(kBPThighL);
        portCandidates.push_back(kBPLegL);
        //                portCandidates.push_back(kBPThighR);
        portCandidates.push_back(kBPLegR);
    }
    else if (_portsToInstall == 3)
    {
        if (_target->isPartMissing(kBPLegL))
        {
            portCandidates.push_back(kBPLegR);
        }
        else if (_target->isPartMissing(kBPLegR))
        {
            portCandidates.push_back(kBPLegL);
        }
        else
        {
            //                    portCandidates.push_back(kBPThighL);
            portCandidates.push_back(kBPLegL);
            //                    portCandidates.push_back(kBPThighR);
            portCandidates.push_back(kBPLegR);
        }
    }
    else if (_portsToInstall == 2)
    {
        if (_target->isPartMissing(kBPLegL) && !_target->isPartMissing(kBPLegR))
        {
            portCandidates.push_back(kBPLegR);
        }
        else if (_target->isPartMissing(kBPLegR) && !_target->isPartMissing(kBPLegL))
        {
            portCandidates.push_back(kBPLegL);
        }
        else
        {
            portCandidates.push_back(kBPArmL);
            portCandidates.push_back(kBPArmR);
        }
    }
    else // 1 port left
    {
        if (_target->isPartMissing(kBPArmL))
        {
            portCandidates.push_back(kBPArmR);
        }
        else if (_target->isPartMissing(kBPArmR))
        {
            portCandidates.push_back(kBPArmL);
        }
        else
        {
            portCandidates.push_back(kBPArmR);
            portCandidates.push_back(kBPArmL);
        }
    }
    
    _workingOn = portCandidates[pick(0, (int)portCandidates.size()-1)];
    if (isInRoomWithPlayer())
    {
        iOut("You hear the drone's actuators as %s walks to %s. It stops at %s %s and raises its manipulator.",
             name().c_str(),
             _target->name().c_str(),
             _target->posessive().c_str(),
             getBodyPart(_workingOn).name().c_str()
             );
    }
}

void Drone::tickAssimilate()
{
    if (!_target)
    {
        pickAssimilateTarget();
        _portsToInstall = pick(3, 4);
        _installFaceplate = roll() < .25;
        _assimilationStage = 0;
        _installCountDn = 0;
    }
    else if (_assimilationStage == 0)
    {
        if (_installCountDn == 0)
        {
            pickLimbToRemove();
            _installCountDn = pick(2, 3);
        }
        else
        {
            if (_installCountDn > 1)
            {
                doAssimilateString();
            }
            else // ==1
            {
                _target->installPort(_workingOn);
                _portsToInstall--;
                if (_portsToInstall == 0)
                    _assimilationStage++;
            }
            
            _installCountDn--;
        }
    }
    else if (_assimilationStage == 1) // Attach implants.
    {
        // Phase 2: Attach implants
        // Only one manipulator and one bionic hand if both arms come off.
        // If all implants are installed, attach exoplating.
        bool pickedTarget = false;
        if (_installCountDn == 0)
        {
            // Pick next target
            if (_target->isPartMissing(kBPLegL))
                _workingOn = kBPLegL;
            else if (_target->isPartMissing(kBPLegR))
                _workingOn = kBPLegR;
            else if (_target->isPartMissing(kBPArmL))
                _workingOn = kBPArmL;
            else if (_target->isPartMissing(kBPArmR))
                _workingOn = kBPArmR;
            else
                _assimilationStage++;

            _installCountDn = pick(2, 3);
            pickedTarget = true;
        }

        if (pickedTarget)
        {
            iOut("%s walks out of sight with %s old half-converted %s. It comes back after a moment with a fully cybernetic limb. You suspect it contains some of the original biology, but cannot be certain as it appears solidly robotic.",
                 name().c_str(),
                 _target->posessive().c_str(),
                 getBodyPart(_workingOn).name().c_str());
        }
        else
        {
            if (_installCountDn > 1)
            {
                doAssimilateString();
            }
            else // ==1
            {
                if (_workingOn == kBPArmR)
                {
                    Implant *manipulator = new EngineerManipulator();
                    manipulator->install(*_target, kBPArmR);
                    iOut(manipulator->getDescInstall(*_target, getBodyPart(kBPArmR), 2).c_str());
                }
                else
                {
                    iOut("%s attaches the new cyborg limb to %s %s. You hear servos working as the prosthesis performs a self-test\n",
                         name().c_str(),
                         _target->posessive().c_str(),
                         getBodyPart(_workingOn).name().c_str());
                    _target->arouseDrone(.3);
                }

                _target->prosthesisInstalled(_workingOn);
            }
            
            _installCountDn--;
        }
    }
    else if (_assimilationStage == 2)
    {
        if (_target->isPlayer())
            iOut("The rack pivots upright. You feel something push into you as the anal implant slides in and docks with your internal machinery. It is now a cybernetic conduit.");
        else
            iOut("%s rack pivots upright. Its anal implant is installed.", _target->posessive().c_str());

        _assimilationStage++;
        _target->arouseDrone(.5);
    }
    else if (_assimilationStage == 3)
    {
        // TODO: Multiple eyepieces, use Implant class
        if (_target->isPlayer())
            iOut("%s comes into view and pivots to face you. It raises an eyepiece, which clicks into your socket. Your vision distorts and then, clarity. You realize you can now see far into the electromagnetic spectrum.", name().c_str());
        else
            iOut("%s installs %s eyepiece. The drone jolts slightly as it clicks into place.", name().c_str(), _target->posessive().c_str());

        _assimilationStage++;
        _target->arouseDrone(.2);
    }
    else if (_assimilationStage == 4)
    {
        // TODO: Multiple armor types, use Implant class
        if (_target->isPlayer())
            iOut("Chest and back pieces are lowered from the ceiling. Simultaneously, they clamp around you and interface with your existing layer of implants. The armor is permanently bonded with you. The bulk of the chestplate contrasts with your sleek muscular abdomen and crotch.");
        else
            iOut("Chest and back pieces are lowered from the ceiling and clamp around %s, permanently bonding with the drone.", _target->name().c_str());
        
        _assimilationStage++;
        _target->arouseDrone(.4);
    }
    else
    {
        if (_target->isPlayer())
        {
            iOut("The restaints unclamp, and hoses detatch. You are free to move. If you still required breathing, you would need to catch your breath. As it is, the pumps in your chest are pulsating at a high frequency but you feel a vibration as they wind down to a more moderate rate.");
            iOut("\"This unit's designation is %s. We will serve the cyborg race\", you hear yourself utter. The voice is raspy and vocoded.", _target->name().c_str());
        }
        else
        {
            iOut("The restaints unclamp, and hoses detatch. %s is free to move. The life-sustaining machinery in its chest spins back down to nominal levels. For a second, it stands motionless, with soft clicking and whirring eminating from its body. After its laser scans the room, its eyes and sensors focus on the drone that converted it and it steps forward. \"This unit's designation is %s. We will serve the cyborg race\", it utters in its robotic monotone. The drone then makes a sharp head turn. It is receiving new instructions.", _target->descName().c_str(),
                 _target->name().c_str());
            cybDeck_t &deck = gCybShipRooms[0];
            Room *destRoom = deck[pick(5, (int)deck.size()-1)];
            _target->setOnPathTo(destRoom);
        }
        
        _target->setNeedsAssimilation(false);
        _target = 0;
    }
}

void Drone::tick()
{
    if (_health < 0)
    {
        _target = 0;
        return;
    }
    
    if (_penetrateTarget)
    {
        if (_genderDestination == -1.0)
        {
            if (roll() < .2)
            {
                iOut("With a whir, %s withdraws, the machinery in its chest spinning back down to nominal levels.", descName().c_str());
                _penetrateTarget->penetrated(0);
                _penetrateTarget = 0;
            }
            else if (roll() < .6)
            {
              //  if (_genderDestination == -1.0)
                {
                    iOut("%s continues to interface with you.", descName().c_str());
                    arouseDrone(.3);
                    gPlayer->arouseDrone(.3);
                }
                /*
                 else if (_penetrateTarget->getGenderDestination() == 1.0)
                 {
                 
                 }
                 */
            }
            else
            {
                iOut("%s is still inside you, but stands motionless, processing...", descName().c_str());
            }
        }
        
        return;
    }
    else if (isInRoomWithPlayer() && gPlayer->copulationRequested())
    {
        // Only a drone with a phallus can initiate copulation
        if (roll() < .3)
        {
            if (getGenderDestination() == -1.0)
            {
                iOut("** Copulation will begin.");
                
                if (gPlayer->getGenderDestination() == 1.0)
                    iOut("You face the wall. The drone's phallus pivots upright and you feel it insert into your synthetic vaginal port, which constricts around the probe. As it interfaces with your electronics, you feel a rush of pleasure.");
                else
                    iOut("You face the wall with anticipation. With a whir, %s gets into position. Its phallus raises and pushes into your anal implant, interfacing with the electronics and machinery embedded deep inside you.", descName().c_str());
         
                this->penetrated(gPlayer);
                gPlayer->arouseDrone(.4);
                arouseDrone(.4);
            }
            else
            {
                iOut("%s is unable to initiate copulation.", name().c_str());
            }
        }
        
        return;
    }

    if (gGameState != kGSTeleportOut)
    {
        if (_state == kStateAttack)
            tickAttackMode();
        else if (_state == kStateConvertMachines)
            tickMachinery();
        else if (_state == kStateWander)
            tickWander();
        else if (_state == kStateInterface)
            tickInterface();
        else if (_state == kStateAssimilate)
        {
            int rounds = pick(3, 6);
            for (int i=0; i < rounds; i++)
                tickAssimilate();
            iOut(" ");
        }
    }
    else
    {
        Room *teleporter = findTeleporterRoom();
        if (_curRoom != teleporter)
        {
            if (_path.empty())
            {
                iOut("** A voice in your head: Drone %s: Initial conversion cycle complete. Return to vessel.", _name.c_str());
//                _path = _curRoom->findPathTo(teleporter);
                // TODO: Beam out?
                doCybShipSequence();
            }
            else
            {
                followPath();
            }
        }
        else
        {
            iOut("** A voice in your head: Drone %s has teleported out.", _name.c_str());
            markDelete();
        }
    }
}
