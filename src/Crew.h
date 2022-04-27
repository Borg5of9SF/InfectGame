#ifndef CREW_H
#define CREW_H

#include "Being.h"

class Crew : public Being
{
public:
    Crew();
    virtual ~Crew() {}
    
    virtual void tick();
    bool isPanicked() { return (_panic != 0); }
    
private:
    enum crewMode_t
    {
        kModeWorking,
        kModeWandering,
        kModePanic
    };
    
    float _panic;
    
};

#endif
