#include <vector>
#include <assert.h>
#include "BodyPartDesc.h"

BodyPartDesc *_bodyPartDescs[kNumBodyParts];

BodyPartDesc &getBodyPart(bodyPartLocation_t location)
{
    return *_bodyPartDescs[location];
}

void initBodyParts()
{
    for (int i=0; i < kNumBodyParts; i++)
        _bodyPartDescs[i] = 0;
    
    // Basic init
    // Arms
    _bodyPartDescs[kBPArmL] = new BodyPartDesc(kBPArmL, true, "left arm");
    _bodyPartDescs[kBPArmR] = new BodyPartDesc(kBPArmR, true, "right arm");
    _bodyPartDescs[kBPHandL] = new BodyPartDesc(kBPHandL, true, "left hand");
    _bodyPartDescs[kBPHandR] = new BodyPartDesc(kBPHandR, true, "right hand");
    _bodyPartDescs[kBPShoulderL] = new BodyPartDesc(kBPShoulderL, true, "left shoulder");
    _bodyPartDescs[kBPShoulderR] = new BodyPartDesc(kBPShoulderR, true, "right shoulder");

    // Legs
    _bodyPartDescs[kBPFootL] = new BodyPartDesc(kBPFootL, true, "left foot");
    _bodyPartDescs[kBPFootR] = new BodyPartDesc(kBPFootR, true, "right foot");
    _bodyPartDescs[kBPLegL] = new BodyPartDesc(kBPLegL, true, "left leg");
    _bodyPartDescs[kBPLegR] = new BodyPartDesc(kBPLegR, true, "right leg");
    _bodyPartDescs[kBPHipL] = new BodyPartDesc(kBPHipL, true, "left hip");
    _bodyPartDescs[kBPHipR] = new BodyPartDesc(kBPHipR, true, "right hip");
    _bodyPartDescs[kBPThighL] = new BodyPartDesc(kBPThighL, true, "left thigh");
    _bodyPartDescs[kBPThighR] = new BodyPartDesc(kBPThighR, true, "right thigh");

    // Trunk
    _bodyPartDescs[kBPChest] = new BodyPartDesc(kBPChest, true, "chest");
    _bodyPartDescs[kBPAbdomen] = new BodyPartDesc(kBPAbdomen, true, "abdomen");
    _bodyPartDescs[kBPPelvis] = new BodyPartDesc(kBPPelvis, true, "pelvis");

    // Neck and head
    _bodyPartDescs[kBPHead] = new BodyPartDesc(kBPHead, true, "head");
    _bodyPartDescs[kBPEarL] = new BodyPartDesc(kBPEarL, false, "left ear");
    _bodyPartDescs[kBPEarR] = new BodyPartDesc(kBPEarR, false, "right ear");
    _bodyPartDescs[kBPEyeL] = new BodyPartDesc(kBPEyeL, false, "left eye");
    _bodyPartDescs[kBPEyeR] = new BodyPartDesc(kBPEyeR, false, "right eye");
    _bodyPartDescs[kBPNose] = new BodyPartDesc(kBPNose, false, "nose");
    _bodyPartDescs[kBPMouth] = new BodyPartDesc(kBPMouth, false, "mouth");
    _bodyPartDescs[kBPNeck] = new BodyPartDesc(kBPNeck, true, "neck");

    // Sanity checks
    for (int i=0; i < kNumBodyParts; i++)
        assert(_bodyPartDescs[i]);

    
    // Create graph
    // Arms
    _bodyPartDescs[kBPHandL]->connectTo(kBPArmL);
    _bodyPartDescs[kBPArmL]->connectTo(kBPShoulderL);

    _bodyPartDescs[kBPHandR]->connectTo(kBPArmR);
    _bodyPartDescs[kBPArmR]->connectTo(kBPShoulderR);
    
    // Legs
    _bodyPartDescs[kBPFootL]->connectTo(kBPLegL);
    _bodyPartDescs[kBPLegL]->connectTo(kBPThighL);
    _bodyPartDescs[kBPThighL]->connectTo(kBPHipL);

    _bodyPartDescs[kBPFootR]->connectTo(kBPLegR);
    _bodyPartDescs[kBPLegR]->connectTo(kBPThighR);
    _bodyPartDescs[kBPThighR]->connectTo(kBPHipR);

    // Trunk
    _bodyPartDescs[kBPShoulderL]->connectTo(kBPChest);
    _bodyPartDescs[kBPShoulderR]->connectTo(kBPChest);
    _bodyPartDescs[kBPHipL]->connectTo(kBPChest);
    _bodyPartDescs[kBPHipR]->connectTo(kBPChest);
    _bodyPartDescs[kBPAbdomen]->connectTo(kBPChest);
    _bodyPartDescs[kBPNeck]->connectTo(kBPChest);
    _bodyPartDescs[kBPPelvis]->connectTo(kBPAbdomen);
    
    // Head
    _bodyPartDescs[kBPNeck]->connectTo(kBPHead);
    _bodyPartDescs[kBPEarL]->connectTo(kBPHead);
    _bodyPartDescs[kBPEarR]->connectTo(kBPHead);
    _bodyPartDescs[kBPEyeL]->connectTo(kBPHead);
    _bodyPartDescs[kBPEyeR]->connectTo(kBPHead);
    _bodyPartDescs[kBPNose]->connectTo(kBPHead);
    _bodyPartDescs[kBPMouth]->connectTo(kBPHead);
    
}

BodyPartDesc::BodyPartDesc(bodyPartLocation_t location, bool isInjectable, const char *name) :
    _location(location),
    _injectable(isInjectable),
    _name(name)
{
}

void BodyPartDesc::connectTo(bodyPartLocation_t loc)
{
    _bodyPartDescs[_location]->_connections.push_back(loc);
    _bodyPartDescs[loc]->_connections.push_back(_location);
}

