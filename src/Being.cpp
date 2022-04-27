#include <algorithm>
#include "Being.h"
#include "System.h"
#include "Room.h"
#include "Drone.h"
#include "Crew.h"
#include "NameHat.h"
#include "Infect.h"

#define ROLL ((float)((rand()&65535))/65535.0)
extern Being player;

Being::Being()
{
    for (int i=0; i < kNumBodyParts; i++)
        _infectPercent[i] = 0;
    
    _leftEyeNeedsEyepiece = false;
    _rightEyeNeedsEyepiece = false;
    _voiceboxInstalled = false;
    _transceiverInstalled = false;
    _isPlayer = false;
    _delete = false;
    _posessive = "your";
    _noun = "you";
    _name = "You";  // TODO.. context..?
    _health = 1.0;
    _armor = 0.3;
    _penetrateTarget = 0;
    _requestCopulation = false;
    
    _curRoom = 0;
    
    const float genderDestinations[4] = { -1.0, 0, 0, 1.0 };
    _genderDestination = genderDestinations[pick(0, 3)];
    _arousal = 0;
    _resistance = .7;

    _legsBare = false;
    _chestBare = false;
}

Being::~Being()
{}

bool Being::isInRoomWithPlayer()
{
    if (!_curRoom)
        return false;
    
    std::vector<Being*> beings = _curRoom->beings();
    for (int i=0; i < beings.size(); i++)
    {
        if (beings[i]->isPlayer())
            return true;
    }
    
    return false;
}

bool Being::tryInject(Being *target)
{
    // TODO: Something when the crewman recognizes the drone?
    if (target == this)
    {
        iOut("You cannot inject yourself.");
        return false;
    }
    else if (dynamic_cast<Drone*>(target))
    {
        iOut("It has no effect.");
        return false;
    }
    
    std::string s = name() + " lunges at " + target->name() + ", its injection needle fully extended. ";
    bool ret = false;
    
    if (isInRoomWithPlayer())
    {
        iOut(s.c_str());
        s = "";
        suspense();
    }
    
    float roll = (float)(rand() & 65535) / 65535.0;
    float diff = (target->_resistance - roll);
    if (diff >= 0)
    {
        s += "The drone misses.";
    }
    else if (diff < -.2)
    {
        const bodyPartLocation_t parts[8] = { kBPArmL, kBPArmR, kBPNeck, kBPShoulderL, kBPShoulderR, kBPAbdomen, kBPLegL, kBPLegR };
        bodyPartLocation_t loc = parts[pick(0, 7)];
        s += " " + target->name() + " is caught in the " + getBodyPart(loc).name() + "! ";
        if (target->calcTotalInfectPercent() == 0)
            s += " " + target->name() + " staggers back.";
        target->inject(loc);
        ret = true;
    }
    else
    {
        const bodyPartLocation_t parts[4] = { kBPHandL, kBPHandR, kBPArmL, kBPArmR };
        bodyPartLocation_t loc = parts[rand() % 4];
        std::string n;
        s += " " + target->name() + " puts hands up to block, but the needle jabs " + target->posessive() + " " + getBodyPart(loc).name() + ".";
        if (target->calcTotalInfectPercent() == 0)
            s += " " + target->name() + " looks up at the drone, shocked.";
        s += " " + target->name() + " is caught in the " + getBodyPart(loc).name() + " and staggers back";
        target->inject(loc);
        ret = true;
    }
    
    if (isInRoomWithPlayer())
    {
        iOut(s.c_str());
        suspense();
    }
    return ret;
}

bool Being::tryShoot(Being *target)
{
    if (target == this)
    {
        if (isPlayer())
            iOut("You cannot shoot yourself.");
        return false;
    }
    else if (dynamic_cast<Crew*>(target))
    {
        if (isPlayer())
            iOut("You try to pull the trigger, but cannot bring yourself to do it.");
        return false;
    }
    else if (target->health() < 0)
    {
        if (isPlayer())
            iOut("It's already as dead as it's ever going to be...");
        return false;
    }
    
    if (_curRoom->neighborContainsBeing(gPlayer))
    {
        iOut("You hear shots fired from an adjacent room.");
    }
    
    std::string s;
    if (target == gPlayer)
        s = _name + " fires at " + target->name() + "(You). ";
    else
        s = _name + " fires at " + target->name() + ". ";

    if (isInRoomWithPlayer())
    {
        iOut(s.c_str());
        s = "";
        suspense();
    }

    target->attackedBy(this);
    float rol = roll();
    float diff = (target->_armor - rol);
    if (gAdapted)
    {
        int x = pick(0, 2);
        if (x == 0) s = "The weapon has no effect: The drones have adapted.";
        else if (x == 1) s = target->name() + " staggers but is unharmed. The drones have adapted.";
        else s = target->name() + " absorbs the shot, unharmed.";
    }
    else if (diff >= 0)
    {
        if (roll() < .5)
            s += target->posessive() + " shield absorbs the shot. It takes no notice'.";
        else
            s += name() + " misses.";
    }
    else if (diff < -.2)
    {
        target->_health -= .3 * rol;
        if (dynamic_cast<Drone*>(target))
        {
            if (target->_health >= 0)
            {
                s += "The drone staggers backwards momentarily but quickly regains its footing.";
            }
            else
            {
                s += "The drone shudders and begins to malfunction. Its manipulator arm juts out, its knees give way, and it goes down. It will be repaired.";
                if (gPlayer->deadName() != "")
                    iOut("** A voice in your head: Drone %s has been deactivated.", target->name().c_str());
            }
        }
        else
        {
            if (target->_health >= 0)
            {
                s += "The beam catches " + target->_name + " square in the chest.";
            }
            else
            {
                if (target->isPlayer())
                {
                    gAdapted = true;
                    s += "The drones adapt to the weapons. " + target->name() + " is unharmed.";
                    /*
                    if (target->_deadName == "")
                        iOut("** You have died.");
                    else
                        iOut("** You are dimly aware of the new systems in your body glitching and shutting down. End.");
                    
                    while(1);   // TODO: Real ending
                     */
                }
                else
                {
                    s += target->_name + " goes down, dead.";
                    target->markDelete();
                }
            }
        }
    }
    else
    {
        target->_health -= .1 * rol;
        if (dynamic_cast<Drone*>(target))
        {
            if (target->_health >= 0)
            {
                s += "The drone is hit, but this doesn't seem to register in its blank face.";
            }
            else
            {
                s += "The drone malfunctions breifly before shutting down. It is standing up, inert, its head tilted at a downward angle.";
                if (gPlayer->deadName() != "")
                    iOut("** A voice in your head: Drone %s has been deactivated.", target->name().c_str());
            }
        }
        else
        {
            if (target->_health >= 0)
            {
                s += "The beam grazes " + target->_name + ".";
            }
            else
            {
                if (target->isPlayer())
                {
                    if (target->_deadName.empty())
                        iOut("** You have died.");
                    else
                        iOut("** You are dimly aware of your cybernetic systems glitching and shutting down. End.");
                    
                    while(1);   // TODO: Real ending
                }

                s += target->_name + " goes down, dead.";
                target->markDelete();
            }
        }
    }
    
    if (isInRoomWithPlayer())
    {
        iOut(s.c_str());
        if (target->health() >= 0)
            iOut("%s health: %d%%\n", target->posessive().c_str(), (int)(target->health()*100.0));
        suspense();
    }
    
    return true;
}


bool Being::inject(bodyPartLocation_t location)
{
    // TODO: Roll for success against resistance
    infectPart(location, .1);
    return true;
}

void Being::goToRoom(Room *room)
{
    if (_curRoom)
        _curRoom->removeBeing(this);

    room->addBeing(this);
    _curRoom = room;
}

bool Being::goToRoom(exits_t direction)
{
    Room *exit = _curRoom->getExitRoom(direction);
    if (exit)
    {
        goToRoom(exit);
        return true;
    }
    else
    {
        iOut("You can't go that way.");
        return false;
    }
}

bool Being::removePants()
{
    if (_legsBare)
    {
        iOut("You've already removed your pants.");
        return false;
    }
    
    std::string s = name() + " removes its shoes, pants and underwear.";
    iOut(s.c_str());
    _legsBare = true;
    
    return true;
}

bool Being::removeShirt()
{
    if (_chestBare)
    {
        iOut("You've already removed your shirt.");
        return false;
    }
    
    std::string s = name() + " removes its shirt.";
    iOut(s.c_str());
    _chestBare = true;
    return true;
}


void Being::installTransceiver()
{
    std::string s;
    if (_isPlayer)
        s = "\n** " + name() + " begins to hear the thoughts of the collective consciousness. The neural transceiver is online. **";
    else if (isInRoomWithPlayer())
        s = sayString("I can hear them.");
    iOut(s.c_str());
 
    _deadName = _name;
    _noun = makeDroneName(true);
;
    _posessive = _noun + "'s";
    if (isInRoomWithPlayer())
        iOut("** Designation changed from %s to %s\n", _name.c_str(), _noun.c_str());
    
    if (_isPlayer)
        iOut("** Your vision is tinted green and distorted. You feel detached, as if your consciousness has taken a back seat...");
    
    _name = _noun;

    _transceiverInstalled = true;
    
    if (!isPlayer())
    {
        gSpawnList.push_back(new Drone(*this));
        markDelete();
    }
}

void Being::arouseDrone(float amt)
{
    float before = _arousal;
    _arousal += amt;
    
    std::string out;
    if (_arousal >= 1.0 /*&& before < 1.0 */)
    {
        int p = pick(0, 3);
        if (p == 0)
            out = descName() + " malfunctions, its limbs making erratic motions as its actuators receive erroneous instructions. " + sayString("Error! Input failure!");
        else if (p == 1)
            out = descName() + " malfunctions, its eye wide. You hear its servos struggling and the whir of its internal implants in overload. " + sayString("Error!");
        else if (p == 2)
        {
            if (_needsAssimilation)
                out = descName() + " malfunctions, its hips bucking uncontrollably. " + sayString("Error! Error!");
            else
                out = descName() + " malfunctions, its actuators struggling to keep the drone on its feet. " + sayString("Error! Error!");
        }
    }
    else if (_arousal >= .8 && before < .8)
    {
        if (pick(0, 1))
            // TODO: Mouthpiece?
            out = posessive() + " eyes are wide and its mouth hangs open as the drone arches its back.";
        else
            out = posessive() + " judders. It is being overloaded.";
    }
    else if (_arousal >= .6 && before < .6)
    {
        if (_voiceboxInstalled)
            out = descName() + " makes a buzzy monotone sound from its voicebox and bucks its hips once, the servos making a short whir as it does so.";
        else
            out = descName() + " moans and bucks its hips, the servos making a short whir as it does so";
    }
    else if (_arousal >= .2 && before < .2)
    {
        if (pick(0, 1))
        {
            if (_voiceboxInstalled)
                out = descName() + " moans, but it comes out as a robotic modulated voice.";
            else
                out = descName() + " moans.";
        }
        else
        {
            // TODO: Mouthpiece?
            out = posessive() + " lips part; eyes unfocus. You hear a sharp whir that just as quickly spins down as the drone takes in oxygen.";
        }

    }

    if (isInRoomWithPlayer())
        iOut(out.c_str());
}

void Being::setOnPathTo(Room *r)
{
    _path = _curRoom->findPathTo(r);
}

std::string Being::descName()
{
    float p = calcTotalInfectPercent();
    if (_health >= 0)
    {
        if (!_deadName.empty())
        {
            return _name + "(" + _deadName + ")";
        }
        else if (p > 0 && p < 1.0)
        {
            return _name + "(Infected)";
        }
    }
    else
    {
        if (dynamic_cast<Drone*>(this))
            return _name + "(Deactivated)";
    }
    
    return _name;
}

void Being::describe()
{
    float p = calcTotalInfectPercent();
    if (p == 0)
    {
        iOut(std::string(name() + " is of average build.").c_str());
        iOut("Items:");
        listItems();
    }
    else
    {
        std::string s;
        if (_deadName != "")
            s = " This used to be " + _deadName + ".";
        iOut(std::string(name() + " has been infected with nanites. " + noun() + " looks unwell: " + posessive() + " skin glistens and veins are visible beneath the skin, but drained of color." + s).c_str());
        iOut("Items:");
        listItems();
    }
}

void Being::listItems()
{
    for (int i=0; i < _items.size(); i++)
    {
        Implant *implant = dynamic_cast<Implant*>(_items[i]);
        if (implant)    // TODO: list mount point as well
            iOut(_items[i]->name().c_str());
    }
}


bool Being::prosthesisInstalled(bodyPartLocation_t loc)
{
    std::vector<bodyPartLocation_t> list;
    switch (loc)
    {
        case kBPShoulderL:
            std::remove(_socketed.begin(), _socketed.end(), kBPShoulderL);
        case kBPArmL:
            std::remove(_socketed.begin(), _socketed.end(), kBPArmL);
            std::remove(_socketed.begin(), _socketed.end(), kBPHandL);
            return true;

        case kBPShoulderR:
            std::remove(_socketed.begin(), _socketed.end(), kBPShoulderR);
        case kBPArmR:
            std::remove(_socketed.begin(), _socketed.end(), kBPArmR);
            std::remove(_socketed.begin(), _socketed.end(), kBPHandR);
            return true;
            
        case kBPThighL:
            std::remove(_socketed.begin(), _socketed.end(), kBPThighL);
        case kBPLegL:
            std::remove(_socketed.begin(), _socketed.end(), kBPLegL);
            std::remove(_socketed.begin(), _socketed.end(), kBPFootL);
            return true;
            
        case kBPThighR:
            std::remove(_socketed.begin(), _socketed.end(), kBPThighR);
        case kBPLegR:
            std::remove(_socketed.begin(), _socketed.end(), kBPLegR);
            std::remove(_socketed.begin(), _socketed.end(), kBPFootR);
            return true;
            
        default:
            return false;
    }

    return true;
}

bool Being::installPort(bodyPartLocation_t loc)
{
    iOut("%s limb is removed at the %s, exposing a gleaming port ready to accept a fully artificial prosthetic upgrade.", posessive().c_str(), getBodyPart(loc).name().c_str());
    
//    arouseDrone(.2);
    
    switch (loc)
    {
        case kBPShoulderL:
            _socketed.push_back(kBPShoulderL);
        case kBPArmL:
            _socketed.push_back(kBPArmL);
            _socketed.push_back(kBPHandL);
            return true;

        case kBPShoulderR:
            _socketed.push_back(kBPShoulderR);
        case kBPArmR:
            _socketed.push_back(kBPArmR);
            _socketed.push_back(kBPHandR);
            return true;
            
        case kBPThighL:
            _socketed.push_back(kBPThighL);
        case kBPLegL:
            _socketed.push_back(kBPLegL);
            _socketed.push_back(kBPFootL);
            return true;

        case kBPThighR:
            _socketed.push_back(kBPThighR);
        case kBPLegR:
            _socketed.push_back(kBPLegR);
            _socketed.push_back(kBPFootR);
            return true;

        default:
            iOut("You cannot remove the drone's %s.", getBodyPart(loc).name().c_str());
    }

    return "";
}

bool Being::isPartMissing(bodyPartLocation_t loc)
{
    for (int i=0; i < _socketed.size(); i++)
    {
        if (_socketed[i] == loc)
            return true;
    }
    
    return false;
}

std::string Being::describeInfection(bodyPartLocation_t location, int stage)
{
    BodyPartDesc &partDesc = getBodyPart(location);
    
    if (stage == 0)
    {
        switch (location)
        {
            case kBPNeck:
                if (_isPlayer)
                    return posessive() + " throat begins to tingle. You feel a lump in it.";
                else
                    return "";
            case kBPHead:
                if (_isPlayer)
                    return noun() + " feels a crawling sensation beneath the skin on " + posessive() + " face.";
                else
                    return "";
            case kBPEyeL:
            case kBPEyeR:
                if (_isPlayer)
                    return posessive() + " " + partDesc.name() + " begins to feel dry.";
                else
                    return "";
            case kBPNose:
                break;
            case kBPMouth:
                if (_isPlayer)
                    return posessive() + " mouth beings to taste a strange metallic sensation.";
                else
                    return "";
                break;
            case kBPPelvis:
                if (_isPlayer)
                    return name() + " begins to feel something shifting in its crotch.";
                else
                    return "";
                break;
            case kBPChest:
                if (_isPlayer)
                    return posessive() + " breathing becomes erattic as the drone feels a strange sensation in the chest.";
                else
                    return posessive() + " breathing becomes erattic.";
            case kBPAbdomen:
                if (_isPlayer)
                    return posessive() + " " + partDesc.name() + " begins to feel tight, like after a hard workout.";
                else
                    return "";
            default:
            {
                if (_isPlayer)
                {
                    std::string which = (rand() & 1)? "tingle." : "go numb.";
                    return posessive() + " " + partDesc.name() + " begins to " + which;
                }
                else
                {
                    return "";
                }
            }
        }
    }
    else if (stage == 1)
    {
        switch (location)
        {
            case kBPHead:
                if (_isPlayer)
                    return "You feel slight pressure at the base of your skull.";
                break;
            case kBPEyeL:
            case kBPEyeR:
                if (_isPlayer)
                    return "There is a sensation of pressure behind " + posessive() + " " + partDesc.name() + ".";
                else
                    return posessive() + " " + partDesc.name() + " takes on a subtle shift in hue.";
            case kBPChest:
            {
                std::string s;
                if (_isPlayer)
                    s += "The drone can feel a vibration coming from deep inside its chest. ";
//                if (_chestBare)
                    s += "Panels have formed flush against " + posessive() + " pectoral area: connectors, devices, and vents.";
//                else
//                    s += "You notice something shifting beneath " + posessive() + " uniform.";
                return s;
            }
            case kBPPelvis:
//                if (_legsBare)
                {
                    if (_genderRating < 0)
                    {
                        if (_genderDestination >= 0)
                        {
                            arouseDrone(.3);
                            return posessive() + " penis begins to shrink and the testicles are drawn up towards the body.";
                        }
                        else
                        {
                            arouseDrone(.3);
                            return posessive() + " penis is wrapped in a silvery web. A coil forms near the base of the phallis. The testicles are drawn up towards the body.";
                        }
                    }
                    else
                    {
                        if (_genderDestination < 0)
                        {
                            arouseDrone(.3);
                            return "A bulge is visible in " + posessive() + "'s pants. Its probe has begun to take shape.";
                        }
                        else
                        {
                            return "";
                        }
                    }
                }
/*                else
                {
                    return "";
                }
 */
            case kBPMouth:
                if (_isPlayer)
                    return noun() + " feels a tingling sensation in its mouth.";
                else
                    return "";
            case kBPAbdomen:
                return sayString("This unit's bio-synthesizers are online. This drone will no longer require food.");
            case kBPNeck:
            {
                _voiceboxInstalled = true;
                std::string s;
                if (rand() & 1)
                    return name() + " lets out a groan, like when you're not sure if your throat is clear-- but it has a modulated electronic raspiness to it.";
                else
                    return posessive() + " throat feels heavy. The voicebox is being converted to a speech synthesis system.";
            }
            case kBPArmL:
            case kBPArmR:
                return "A web of grey spreads just beneath the skin on " + posessive() + " " + partDesc.name() + ". The muscle definition increases as synthetic fibers and actuators augment the biological muscle tissue.";
            case kBPHandL:
            case kBPHandR:
                return "Metallic webbing covers " + posessive() + " " + partDesc.name() + ".";
            default:
                return "A web of grey spreads just beneath the skin on " + posessive() + " " + partDesc.name() + ".";
        }
    }
    else    // Body part has finished transformation
    {
        switch (location)
        {
            case kBPFootL:
            case kBPFootR:
//                if (!_legsBare)
                    return posessive() + " shoe falls apart, exposing a heavily assimilated " + partDesc.name();
            case kBPHandL:
            case kBPHandR:
                return posessive() + " " + partDesc.name() + " is encased in a silver exoskeletal mesh. The nails have turned a dull grey and have tiny chips and circuitry etched onto them. They are ready for further upgrades.";
            case kBPChest:
                return name() + " ceases breathing. The drone jolts as its uniform shreds, exposing its cyborg body's armored plating. Rhythmic mechanical sounds are audible from " + posessive() + " chest as the nanite pump which replaced its heart comes online. The chest whirs as fans draw oxygen into its intake vent.";
            case kBPPelvis:
//                if (_legsBare)
                {
                    if (_genderRating < 0)
                    {
                        if (_genderDestination > 0)
                        {
                            arouseDrone(.3);
                            return "An artificial vaginal opening forms in " + posessive() + " cyborg crotch. The remains of the penis have been wrapped in technology, transforming it into a clitoral implant.";
                        }
                        else if (_genderDestination == 0)
                        {
                            arouseDrone(.3);
                            return posessive() + " crotch is now genderless. The uniform shreds, exposing an array of implants taking the place of genitalia.";
                        }
                        else
                        {
                            arouseDrone(.3);
                            return posessive() + " penis is wrapped in a silvery web. A coil forms near the base of the phallis. The testicles are drawn up towards the body.";
                        }
                    }
                    else
                    {
                        if (_genderDestination < 0)
                        {
                            arouseDrone(.3);
                            return posessive() + " probe is complete. The uniform shreds, exposing the phallis proudly hanging from the drone's otherwise smooth crotch. There is a coil of tubing around its base.";
                        }
                        else
                        {
                            arouseDrone(.3);
                            return posessive() + " uniform shreds, exposing a perfectly smooth crotch with a slight graceful bulge.";
                        }
                    }
                }
            case kBPMouth:
                return posessive() + " tongue is wrapped in a web of implants and circuits have been etched into the teeth. It is now ready for further enhancements if necessary.";
            case kBPAbdomen:
                return "The power generator and battery system has been constructed in " + posessive() + " abdomen and is online.";
            case kBPNose:
                return "";
            case kBPHead:
                return name() + " can feel memories rearranging, neurons converted to cybernetic data nodes.";
            case kBPEyeL:
            case kBPEyeR:
            {
                int which = rand() & 1;
                if (which)
                {
                    return posessive() + " " + partDesc.name() + " pupil appears lighter than normal. The iris is drained of color. This eye is ready for further augmentation if necessary.";
                }
                else
                {
                    if (location == kBPEyeL)
                        _leftEyeNeedsEyepiece = true;
                    else
                        _rightEyeNeedsEyepiece = true;
                    return "Small implants now completely cover the surface of " + posessive() + " " + partDesc.name() + ". In the center is a jack. This drone must receive an eyepiece.";
                }
            }
            default:
            {
                int which = rand() & 3;
                if (which == 0)
                    return "The skin on " + posessive() + " " + partDesc.name() + " takes on a strange sheen. It is ready for further implants.";
                else if (which == 1)
                    return posessive() + " " + partDesc.name() + " is dotted with implants, connected with circuits etched into its skin like metallic tattoos, light glinting off them.";
                else if (which == 2)
                    return posessive() + " " + partDesc.name() + " has completed stage 1 conversion.";
                else if (which == 3)
                    return posessive() + " " + partDesc.name() + " is encircled in tubes: conduits for energy and fluids both biological and synthetic.";
            }
        }
    }
    
    return "";
}

void Being::infectPart(bodyPartLocation_t loc, float increase)
{
    float before = _infectPercent[loc];
    _infectPercent[loc] += increase;
    if (_infectPercent[loc] >= 1.0)
        _infectPercent[loc] = 1.0;

    if (before == 0)
    {
        std::string s = describeInfection(loc, 0);
        if (isInRoomWithPlayer())
            iOut(s.c_str());
    }
    if (before <= .5 && _infectPercent[loc] >= .5)
    {
        std::string s = describeInfection(loc, 1);
        if (isInRoomWithPlayer())
            iOut(s.c_str());
    }
    if (before < 1.0 && _infectPercent[loc] >= 1.0)
    {
        std::string s = describeInfection(loc, 2);
        if (isInRoomWithPlayer())
            iOut(s.c_str());
    }
}

void Being::tick()
{
    if (_health < 0)
    {
    }
    
    float beforeTotal = calcTotalInfectPercent();
    
    // Calculate nanite reproduction
    for (int i=0; i < kNumBodyParts; i++)
    {
        if (_infectPercent[i] > 0)
            infectPart((bodyPartLocation_t)i, ROLL * (1.0 - _resistance) * .1);    // TODO: exponential?
    }
    
    // Now calculate effect on surrounding body parts
    for (int i=0; i < kNumBodyParts; i++)
    {
        // Look for connected body parts and infect those too
        BodyPartDesc &infectedPart = getBodyPart((bodyPartLocation_t)i);
    
        size_t numConnections = infectedPart.connections().size();
        for (int j=0; j < numConnections; j++)
        {
            bodyPartLocation_t targetPart = infectedPart.connections()[j];
            float naniteImpact = ROLL * _infectPercent[i];
            
            // The 4x scale is so it's easier to break through the initial resistance of that body part; otherwise
            // infection doesn't begin until the nanites reach a huge percentage, and then assimilation is too quick
            if (naniteImpact * 4.0 > _resistance)
            {
//                arouseDrone(.01);
                infectPart(targetPart, naniteImpact * (1.0 - _resistance));
            }
        }
    }
  
    // Do descriptions

/*
    for (int i=0; i < kNumBodyParts; i++)
    {
        BodyPartDesc &infectedPart = getBodyPart((bodyPartLocation_t)i);
        if (_infectPercent[i] > .1)
            printf("Infection of %s is %f\n", infectedPart.name().c_str(), _infectPercent[i]);
    }
 */
    float totalInfect = calcTotalInfectPercent();
    float beforeResistance = _resistance;
    _resistance *= (1.0 - totalInfect);
    
    if (isInRoomWithPlayer())
    {
        std::string resistStr;
        if (beforeResistance > .4 && _resistance <= .4)
            resistStr = sayString("I will not be made into a drone!");
        else if (beforeResistance > .1 && _resistance <= .1)
            resistStr = sayString("We will comply.");
        iOut(resistStr.c_str());
    }
    
    if (totalInfect >= 1.0 && beforeTotal < 1.0)
    {
        installTransceiver();
        if (isInRoomWithPlayer())
            iOut("Phase 1 of %s conversion is complete. Additional implants must be surgically added.", posessive().c_str());
    }
    _arousal *= .4;
//    iOut("The drone's resistance is at %f, arousal at %f, and is %f%% infected.", _resistance, _arousal, totalInfect);
}

std::string Being::sayString(std::string s)
{
    std::string voiceboxStr;
    if (_voiceboxInstalled)
    {
        if (rand() & 1)
            voiceboxStr = "There is a flat electronic timbre to the speech.";
        else
            voiceboxStr = "Its voice has an eerie robotic modulation.";
    }
    else if (!_voiceboxInstalled && calcTotalInfectPercent() > .4)
    {
        voiceboxStr = "The voice still sounds human, albeit monotone.";
    }
    
    std::string t;
    /*
    if (_genderRating == 1.0)
    {
        int x = pick(0, 2);
        if (x == 0) t = "says";
        else if (x == 1) t = "coos";
        else t = "declares";
    }
    else
    {
        int x = pick(0, 2);
        if (x == 0) t = "says";
        else if (x == 1) t = "demands";
        else t = "declares";
    }*/
    
    t = "says";
    
    if (rand() & 1)
        return name() + " " + t + ", \"" + s + "\". " + voiceboxStr;
    else
        return "\"" + s + "\", " + t + " " + name() + ". " + voiceboxStr;
}

float Being::calcTotalInfectPercent()
{
    float sum = 0;
    for (int i=0; i < kNumBodyParts; i++)
    {
        assert(_infectPercent[i] <= 1.0);
        sum += _infectPercent[i];
//        printf("%d, %f\n", i, _infectPercent[i]);
    }
    
    return sum / (float)kNumBodyParts;
}

bool Being::penetrate(Being &other)
{
    if (getGenderDestination() == 0)
    {
        if (other.getGenderDestination() == 0)
        {
            iOut("You have no means of pleasuring yourself; You are permanently sealed within your cybernetic armor as is the other drone. You must find a drone which can interface with your anal implant. Your implants send a copulation request to the collective...");
            _requestCopulation = true;
            return false;
        }
        else if (other.getGenderDestination() == 1.0)
        {
            iOut("You slip a finger into the drone's synthetic vagina. You feel the walls press against it. Then, your finger opens, docking with the machinery inside.");
            other.arouseDrone(.2);
            gPlayer->arouseDrone(.1);
        }
        else
        {
            iOut("You cannot initiate with this drone; Your implants send a copulation request to the collective...");
            _requestCopulation = true;
            return false;
        }
    }
    else if (getGenderDestination() == 1.0)
    {
        if (other.getGenderDestination() == 0)
        {
            iOut("This drone is sealed within its cyborg body. You place your hand on its smooth crotch; the drone is aroused. It cocks its head.");
            other.arouseDrone(.1);
            arouseDrone(.1);
        }
        else if (other.getGenderDestination() == 1.0)
        {
            iOut("You slip a finger into the drone's synthetic vagina.");
            other.arouseDrone(.2);
            arouseDrone(.1);
        }
        else
        {
            iOut("You cannot initiate with this drone; Your implants send a copulation request to the collective.");
            _requestCopulation = true;
//            iOut("You face the wall. The drone's phallus pivots upright and you feel it insert into your synthetic vaginal port, which constricts around the probe. As it interfaces with your electronics, you feel a rush of pleasure.");
//            other.arouseDrone(.4);
//            arouseDrone(.4);
//            iOut("After a few seconds, it disengages.");
        }
    }
    else if (getGenderDestination() == -1.0)
    {
        if (other.getGenderDestination() == 1.0)
        {
            iOut("The other drone turns around. Your phallus plugs into its synthetic vagina.");
            other.arouseDrone(.4);
            arouseDrone(.4);
            iOut("After a few seconds, you disengage.");
        }
        else
        {
            iOut("The other drone turns around and your phallus is inserted into its anal implant, docking with it and interfacing with the drone.");
            other.arouseDrone(.4);
            arouseDrone(.4);
        }
    }

    other.penetrated(this);
    return true;
}

void Being::withdraw(Being &other)
{
    iOut("You disengage from %s.", other.name().c_str());
    other.penetrated(0);
}
