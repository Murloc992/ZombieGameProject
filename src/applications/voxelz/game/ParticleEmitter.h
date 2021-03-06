#ifndef PARTICLEEMITTER_H
#define PARTICLEEMITTER_H

#include "Voxel/VoxelTypes.h"
#include "utility/Vector.h"
#include "Opengl/BufferObject.h"

#define PARTICLE_SUBSTEPS 20

class IParticleAffector;
struct Particle;

class ParticleEmitter
{
public:
    glm::vec3 _pos;
    glm::vec3 _direction;

    float _speed;
    float _spread;

    float _particleLife;
    float _particleSize;

    uint32_t _particleCount,_deadParticleCount,_maxParticles,_lastUsedParticle;

    bool _loop;
    bool _finished;

    vector<Particle> _particleContainer;
private:
    vector<IParticleAffector*> _particleAffectors;
protected:
public:
    ParticleEmitter(glm::vec3 pos,glm::vec3 direction, float speed, float spread,float particleLife,uint32_t maxParticles=1024,bool loop=true);

    virtual ~ParticleEmitter();

    void Update(float dt,uint32_t &particleCount, BufferObject<glm::vec3> *pos, BufferObject<u8vec4> *col, BufferObject<glm::vec4> *rot);

    void AddParticleAffector(IParticleAffector* affector);

    void SetSize(const uint32_t &newsize);

    void SetParticleLife(const uint32_t &newLife);

    void Emit(Particle& p);

    int32_t FindUnused();
private:

protected:
};

#endif // PARTICLEEMITTER_H
