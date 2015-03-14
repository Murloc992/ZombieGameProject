#ifndef ENTITY_H_INCLUDED
#define ENTITY_H_INCLUDED

#include "Physics/CollisionObject.h"

enum E_ENTITY_TYPE:uint32_t
{
    EET_DEFAULT,
    EET_PLAYER,
    EET_ITEM,
    EET_ENEMY,

    EET_COUNT
};

class ChunkManager;
class Block;

class Entity:public CollisionObject
{
public:
    Entity(std::string id,const glm::vec3 &pos,const glm::vec3 &size,bool colliding=true,bool collidingWorld=true,bool dynamic=true);
    virtual ~Entity();

    uint32_t GetType();

    std::string GetId();

    bool IsOnGround()
    {
        return _isOnGround;
    }

    void CheckCollision(Entity* ent);

    virtual void Update(float dt) = 0;

    virtual void OnCollisionWithWorld(const Block &blk) = 0;
    virtual bool OnCollision(Entity* ent) = 0;
private:
protected:
    bool _isDynamic,_isColliding,_isCollidingWorld,_isOnGround;
    uint32_t _type;
    std::string _id;

    void CollideWithWorld(float dt,ChunkManager* chkmgr);
};

#endif // ENTITY_H_INCLUDED
