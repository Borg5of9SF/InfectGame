#ifndef BODYPARTDESC_H
#define BODYPARTDESC_H

#include <vector>
#include <string>

enum bodyPartLocation_t
{
    kBPFootL,
    kBPFootR,
    kBPLegL,
    kBPLegR,
    kBPThighL,
    kBPThighR,
    kBPHipL,
    kBPHipR,
    kBPPelvis,
    kBPAbdomen,
    kBPChest,
    kBPShoulderL,
    kBPShoulderR,
    kBPArmL,
    kBPArmR,
    kBPHandL,
    kBPHandR,
    kBPNeck,
    kBPHead,
    kBPEyeL,
    kBPEyeR,
    kBPMouth,
    kBPNose,
    kBPEarL,
    kBPEarR,
    kNumBodyParts
};
// will add individual organs in next version (voicebox, heart, digestive, spine, etc)

class BodyPartDesc
{
public:
    BodyPartDesc(bodyPartLocation_t location, bool isInjectable, const char *name);
    virtual ~BodyPartDesc() {}

    void connectTo(bodyPartLocation_t loc);
    std::string name() { return _name; }
    
    std::vector<bodyPartLocation_t> &connections() { return _connections; }
    
private:
    bodyPartLocation_t _location;
    bool _injectable;
    
    // Each body part can be connected to others to form a map
    std::vector<bodyPartLocation_t> _connections;
    std::string _name;
};

void initBodyParts();
void uninitBodyParts();
BodyPartDesc &getBodyPart(bodyPartLocation_t location);


#endif

