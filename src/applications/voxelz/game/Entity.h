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

class Entity:public CollisionObject
{
public:
    Entity(std::string id,const glm::vec3 &pos)
    {
        CollisionObject(pos,glm::vec3(1));
        _type=EET_DEFAULT;
        _id=id;
    }
    ~Entity(){}

    std::string GetId()
    {
        return id;
    }

    glm::vec3 GetPosition()
    {
        return pos;
    }

    void SetPosition(const glm::vec3 &other)
    {
        pos=other;
    }

    void CheckCollision(Entity* ent)
    {
        if(MPRCollide(ent, this))
        {
            OnCollision(ent);
            ent->OnCollision(this);
        }
    }

    virtual bool OnCollision(Entity* ent);
private:
protected:
    uint32_t _type;
    std::string _id;
    glm::vec3 _pos;
};

#endif // ENTITY_H_INCLUDED
