#include "Precomp.h"

#include "Particle.h"
#include "ParticleEmitter.h"
#include "IParticleAffector.h"

ParticleEmitter::ParticleEmitter(glm::vec3 pos,glm::vec3 direction, float speed, float spread,float particleLife,uint32_t maxParticles,bool loop)
{
    _pos=pos;
    _direction=direction;

    _speed=speed;
    _spread=spread;

    _particleLife=particleLife;
    _particleSize=15.f;

    _maxParticles=maxParticles;
    _lastUsedParticle=0;
    _particleCount=0;
    _deadParticleCount=0;

    _loop=loop;
    _finished=false;

    _particleContainer.resize(_maxParticles);
}

ParticleEmitter::~ParticleEmitter()
{
    //dtor
}

void ParticleEmitter::AddParticleAffector(IParticleAffector* affector)
{
    _particleAffectors.push_back(affector);
}

int32_t ParticleEmitter::FindUnused()
{
    loopr(i, _lastUsedParticle, _maxParticles)
    {
        if(_particleContainer[i].life<=0.f)
        {
            _lastUsedParticle=i;
            return i;
        }
    }
    loopi(_maxParticles/2)
    {
        if(_particleContainer[i].life<=0.f)
        {
            _lastUsedParticle=i;
            return i;
        }
    }
    return -1;
}

void ParticleEmitter::Emit(Particle& p)
{
    p.life=_particleLife;
    p.mass = 100;

    p.rot.x=(rand()%360);
    p.rot.y=(rand()%360);
    p.rot.z=(rand()%360);

    p.rot.w = rand()%(int)_particleSize;

    p.pos=_pos;
    p.col=u8vec4(150+rand()%106,rand()%50,rand()%1,255);

    glm::vec3 mainDir=_direction;
    glm::vec3 randomDir((rand()%2000-1000.f)/1000.f,(rand()%2000-1000.f)/1000.f,(rand()%2000-1000.f)/1000.f);
    p.speed=mainDir*_speed+randomDir*_spread;
}

void ParticleEmitter::Update(float dt,uint32_t &particleCount, BufferObject<glm::vec3> *pos, BufferObject<u8vec4> *col, BufferObject<glm::vec4> *rot)
{
    for(auto &p:_particleContainer)
    {
        if(p.life>0.f)
        {
            p.life-=dt;
            if(p.life>0.f)
            {
                for(auto &affector:_particleAffectors)
                {
                    loopi(PARTICLE_SUBSTEPS)
                    affector->Affect(dt,p,this);
                }

                p.pos+=p.speed*dt;

                pos->data[particleCount]=p.pos;
                col->data[particleCount]=p.col;
                rot->data[particleCount]=p.rot;

                particleCount++;
            }
        }
    }

    for(auto &p:_particleContainer)
    {
        if(p.life<0.f)
        {
            p.life=0;
            _deadParticleCount++;
        }
    }

    //printf("deads %u\n",_deadParticleCount);
}

void ParticleEmitter::SetSize(const uint32_t &newsize)
{
    _maxParticles=newsize;
    _particleCount=0;
    _deadParticleCount=0;
    _finished=false;
    for(auto &p:_particleContainer)
    {
        p.life=0.f;
    }
    _particleContainer.resize(newsize);
}

void ParticleEmitter::SetParticleLife(const uint32_t &newLife)
{
    _particleLife=newLife;
}
