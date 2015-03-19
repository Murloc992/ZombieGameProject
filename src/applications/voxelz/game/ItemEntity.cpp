#include "Precomp.h"
#include "ItemEntity.h"

#include "Voxel/VoxelMesh.h"
#include "Voxel/ChunkManager.h"
#include "Scenegraph/Camera.h"
#include "Opengl/MVar.h"

ItemEntity::ItemEntity(ChunkManager* chkmgr,const glm::vec3 &pos,const std::string &filename):Entity("item",pos,glm::vec3(1.f))
{
    _type=EET_ITEM;
    _sprite=(VoxelSprite*)VoxelSprite::LoadFromFile(filename);
    _sprite->Rebuild();
    _chkmgr=chkmgr;
    _pos=pos;
}

ItemEntity::~ItemEntity()
{

}

void ItemEntity::Update(float dt)
{
    float spd=9.f;

    int substeps=glm::ceil(spd*dt)*2;
    glm::vec3 stepsteed=(glm::vec3(0,-spd,0)*dt)/(float)substeps;

    loopi(substeps)
    {
        Translate(stepsteed);
        if(_isCollidingWorld)
        {
            CollideWithWorld(dt,_chkmgr);
        }
    }
}

void ItemEntity::OnCollisionWithWorld(const Block &blk)
{

}

bool ItemEntity::OnCollision(Entity* ent)
{

}

void ItemEntity::Render(float dt,CameraPtr cam)
{
    glm::mat4 Model=glm::mat4(1.0f);
    Model=glm::translate(Model,_colShape.GetCenter()-_colShape.GetHalfSize());
    Model=glm::scale(Model,glm::vec3(1.f/64.f,1.f/64.f,1.f/32.f));
    glm::mat4 MVP=cam->GetViewProjMat()*Model;
    MVar<glm::mat4>(0, "mvp", MVP).Set();
    _sprite->Render(false);
}
