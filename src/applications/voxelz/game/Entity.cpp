#include "Precomp.h"

#include "Voxel/VoxelTypes.h"
#include "Voxel/ChunkManager.h"
#include "Voxel/Block.h"

#include "Entity.h"

Entity::Entity(ChunkManager* chkmgr,std::string id,const glm::vec3 &pos,const glm::vec3 &size,bool colliding,bool collidingWorld,bool dynamic):CollisionObject(pos,size/2.f)
{
    _chunkManager=chkmgr;
    _type=EET_DEFAULT;
    _id=id;
    _isColliding=colliding;
    _isCollidingWorld=collidingWorld;
    _isDynamic=dynamic;
    _isOnGround=false;
    _alive=true;
    _containingChunks.reserve(16);
    _velocity=glm::vec3(0.f);
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

void Entity::CollideWithWorld(float dt)
{
    glm::ivec3 amn=(glm::ivec3)_colShape.GetMin();
    glm::ivec3 amx=(glm::ivec3)_colShape.GetMax();

    glm::ivec3 center=(glm::ivec3)_colShape.GetCenter();
    glm::ivec3 hs=(glm::ivec3)glm::ceil(_colShape.GetHalfSize());

    glm::ivec3  startCoord=center-hs,
                endCoord=center+hs;

    glm::ivec3 epos=(glm::ivec3)_colShape.GetCenter();
    //printf("Collision ranges: PLAYERPOS: %d %d %d START: %d %d %d END: %d %d %d\n",epos.x,epos.y,epos.z,sx,sy,sz,ex,ey,ez);

    /// CHUNK TRACKING (can use some kind of range for broadphasing later on)
    glm::ivec3 startChunkCoords=WorldToChunkCoords(startCoord);
    glm::ivec3 endChunkCoords=WorldToChunkCoords(endCoord);

    // find chunks we are in
    for(uint32_t x=startChunkCoords.x; x<=endChunkCoords.x; x++)
    {
        for(uint32_t y=startChunkCoords.y; y<=endChunkCoords.y; y++)
        {
            for(uint32_t z=startChunkCoords.z; z<=endChunkCoords.z; z++)
            {
                /// Chunk coordinates
                glm::ivec3 pos=glm::ivec3(x,y,z);

                ChunkPtr chunk=_chunkManager->GetChunk(pos);

                if(chunk!=nullptr)
                {
                    auto chunkpointer=chunk.get();

                    /// skip already added ones
                    if(std::find(_containingChunks.begin(),_containingChunks.end(),chunkpointer)!=_containingChunks.end()) continue;
                    /// if collide/are inside chunk box, add and enter chunk
                    CollisionObject b=CollisionObject((glm::vec3)(chunkpointer->positionWorld+glm::ivec3(CHUNK_SIZE/2)),glm::vec3(CHUNK_SIZEF/2.f));
                    if(MPRCollide(this,&b))
                    {
                        _containingChunks.push_back(chunkpointer);
                        OnEnterChunk(chunkpointer);
                    }
                }
            }
        }
    }

    // handle addition/removal events
    for(auto it=_containingChunks.begin(); it!=_containingChunks.end();)
    {
        /// if doesn't collide, erase and leave
        CollisionObject b=CollisionObject((glm::vec3)((*it)->positionWorld+glm::ivec3(CHUNK_SIZE/2)),glm::vec3(CHUNK_SIZEF/2.f));
        if(!MPRCollide(&b,this))
        {
            OnExitChunk(*it);
            it=_containingChunks.erase(it);
            continue;
        }
        it++;
    }

    /// BLOCK TRACKING
    for(int32_t x=startCoord.x; x<=endCoord.x; x++)
    {
        for(int32_t y=startCoord.y; y<=endCoord.y; y++)
        {
            for(int32_t z=startCoord.z; z<=endCoord.z; z++)
            {
                const Block &blk=_chunkManager->GetBlock(glm::ivec3(x,y,z));
                if(blk!=Chunk::EMPTY_BLOCK&&blk.active&&blk.type!=EBT_WATER)
                {
                    CollisionObject b=CollisionObject(glm::vec3(x,y,z)+glm::vec3(0.5f),glm::vec3(0.5f));

                    bool collision=MPRCollide(this,&b);

                    if(collision)
                    {
                        CollisionInfo cinf=MPRPenetration(this,&b);
                        if(cinf.colliding&&cinf.depth==cinf.depth&&ccdVec3Eq(&cinf.dir,&cinf.dir))
                        {
                            glm::vec3 direction=CCDtoGLM(cinf.dir);
                            this->Translate(-direction*cinf.depth);

                            if(y==startCoord.y&&x==center.x&&z==center.z)
                                _isOnGround=true;
                            if(y==endCoord.y&&x==center.x&&z==center.z)
                                _hitCeiling=true;

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
                    if(y==startCoord.y&&x==center.x&&z==center.z)
                        _isOnGround=false;
                    if(y==endCoord.y&&x==center.x&&z==center.z)
                        _hitCeiling=false;

                    OnCollisionWithWorld(blk);
                }
                else
                    continue;
            }
        }
    }

//    const Block &blockBelow=_chunkManager->GetBlock(glm::ivec3(center.x,startCoord.y,center.z));
//    const Block &blockAbove=_chunkManager->GetBlock(glm::ivec3(center.x,endCoord.y,center.z));
//    _isOnGround=blockBelow!=Chunk::EMPTY_BLOCK&&blockBelow.active&&blockBelow.type!=EBT_WATER;
//    _hitCeiling=blockAbove!=Chunk::EMPTY_BLOCK&&blockAbove.active&&blockAbove.type!=EBT_WATER;
}

void Entity::OnEnterChunk(Chunk* chunk)
{
    chunk->AddEntity(this);
}

void Entity::OnExitChunk(Chunk* chunk)
{
    chunk->RemoveEntity(this);
}

void Entity::Update(float dt)
{
    if(_isDynamic)
    {
        float maxAxisSpeed = glm::max(glm::abs(_velocity.x),glm::max(glm::abs(_velocity.y),glm::abs(_velocity.z)));

        int substeps=glm::ceil(maxAxisSpeed*dt)*2;
        glm::vec3 stepsteed=(_velocity*dt)/(float)substeps;

        loopi(substeps)
        {
            Translate(stepsteed);
            if(_isCollidingWorld)
            {
                CollideWithWorld(dt);
            }
        }
    }
}

bool Entity::OnCollision(Entity* ent)
{
    if(ent->_alive&&ent->_isDynamic&&ent->_isColliding)
    {
        bool collision=MPRCollide(this,ent);

        if(collision)
        {
            CollisionInfo cinf=MPRPenetration(this,ent);
            if(cinf.colliding&&cinf.depth==cinf.depth&&ccdVec3Eq(&cinf.dir,&cinf.dir))
            {
                glm::vec3 direction=CCDtoGLM(cinf.dir);
                this->Translate(-direction*cinf.depth);
            }
        }
        if(ent->GetTopPosition().y<=this->GetBottomPosition().y) _isOnGround=true;
        if(ent->GetBottomPosition().y>=this->GetTopPosition().y) _hitCeiling=true;
    }
}
