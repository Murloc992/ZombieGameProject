#ifndef PARTICLEAFFECTOR_H
#define PARTICLEAFFECTOR_H

#include "Particle.h"

class ParticleEmitter;

class IParticleAffector
{
    public:
        virtual ~IParticleAffector(){};
        virtual void Affect(float dt,Particle & particleToAffect, ParticleEmitter *emitter)=0;
    protected:
    private:
};

#endif // PARTICLEAFFECTOR_H
