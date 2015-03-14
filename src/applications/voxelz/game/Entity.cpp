#include "Precomp.h"

#include "Voxel/VoxelTypes.h"
#include "Voxel/ChunkManager.h"
#include "Voxel/Block.h"

#include "Entity.h"

Entity::Entity(std::string id,const glm::vec3 &pos,const glm::vec3 &size,bool colliding,bool collidingWorld,bool dynamic):CollisionObject(pos,size/2.f)
{
    _type=EET_DEFAULT;
    _id=id;
    _isColliding=colliding;
    _isCollidingWorld=collidingWorld;
    _isDynamic=dynamic;
    _isOnGround=false;
}

Entity::~Entity()
{

}

uint32_t Entity::GetType()
{
    return _type;
}

std::string Entity::GetId()
{
    return _id;
}

void Entity::CheckCollision(Entity* ent)
{
    if(_isColliding)
    {
        if(MPRCollide(ent, this))
        {
            OnCollision(ent);
            ent->OnCollision(this);
        }
    }
}

void Entity::CollideWithWorld(float dt,ChunkManager* chkmgr)
{
    glm::ivec3 amn=(glm::ivec3)glm::floor((_colShape.GetMin()));
    glm::ivec3 amx=(glm::ivec3)glm::ceil((_colShape.GetMax()));

    int32_t sx,ex,sy,ey,sz,ez;

    sx=amn.x;
    sy=amn.y;
    sz=amn.z;

    ex=amx.x;
    ey=amx.y;
    ez=amx.z;

    if(ex<sx)
    {
        int32_t tmp=sx;
        sx=ex;
        ex=tmp;
    }

    if(ey<sy)
    {
        int32_t tmp=sy;
        sy=ey;
        ey=tmp;
    }

    if(ez<sz)
    {
        int32_t tmp=sz;
        sz=ez;
        ez=tmp;
    }

    const Block &blockBelow=chkmgr->GetBlock(glm::ivec3(sx,sy-1,sz));
    const Block &blockAbove=chkmgr->GetBlock(glm::ivec3(sx,ey+1,sz));
    _isOnGround=blockBelow!=Chunk::EMPTY_BLOCK&&blockBelow.type!=EBT_WATER;

    for(int32_t x=sx; x<ex; x++)
    {
        for(int32_t y=sy; y<ey; y++)
        {
            for(int32_t z=sz; z<ez; z++)
            {
                const Block &blk=chkmgr->GetBlock(glm::ivec3(x,y,z));
                if(blk.active&&blk.type!=EBT_WATER)
                {
                    CollisionObject b=CollisionObject(glm::vec3(x,y,z)+glm::vec3(0.5f),glm::vec3(0.5f));

                    if(MPRCollide(this,&b))
                    {
                        CollisionInfo cinf=MPRPenetration(this,&b);
                        if(cinf.colliding&&cinf.depth==cinf.depth&&ccdVec3Eq(&cinf.dir,&cinf.dir))
                        {
                            glm::vec3 direction=CCDtoGLM(cinf.dir);
                            this->Translate(-direction*cinf.depth);

                            OnCollisionWithWorld(blk);
                        }
                    }
                }
                else if(blk.active&&blk.type==EBT_WATER)
                {
                    CollisionObject b=CollisionObject(glm::vec3(x,y,z)+glm::vec3(0.5f),glm::vec3(0.5f));

                    if(MPRCollide(this,&b))
                    {
                        OnCollisionWithWorld(blk);
                    }
                }
                else if(blk.type==EBT_AIR)
                {
                    OnCollisionWithWorld(blk);
                }
                else
                    continue;
            }
        }
    }
}
