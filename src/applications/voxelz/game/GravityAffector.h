#ifndef GRAVITYAFFECTOR_H
#define GRAVITYAFFECTOR_H

#include "IParticleAffector.h"

class GravityAffector:public IParticleAffector
{
    public:
        virtual ~GravityAffector();
        void Affect(float dt,Particle & particleToAffect, ParticleEmitter *emitter);
    protected:
    private:
};

#endif // GRAVITYAFFECTOR_H
