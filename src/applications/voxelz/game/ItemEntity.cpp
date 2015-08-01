#include "Precomp.h"
#include "ItemEntity.h"

#include "Voxel/VoxelMesh.h"
#include "Voxel/ChunkManager.h"
#include "Scenegraph/Camera.h"
#include "Opengl/MVar.h"
#include "opengl/CubeMesh.h"

ItemEntity::ItemEntity(ChunkManager* chkmgr,const glm::vec3 &pos,const std::string &filename):Entity(chkmgr,"item",pos,glm::vec3(1.f))
{
    _type=EET_ITEM;
    _sprite=(VoxelSprite*)VoxelSprite::LoadFromFile(filename);
    _sprite->Rebuild();

    _velocity=glm::vec3(0.f,-GRAVITY_CONSTANT,0.f);
    _debugAABBMesh=new CubeMesh(AABB(glm::vec3(0),_colShape.GetHalfSize()));
}

ItemEntity::~ItemEntity()
{

}

void ItemEntity::Update(float dt)
{
    if(!_isOnGround)
    {
        _velocity=glm::vec3(0.f,-GRAVITY_CONSTANT,0.f);
    }
    else
    {
        _velocity=glm::vec3(0.f);
    }
    Entity::Update(dt);
}

void ItemEntity::OnCollisionWithWorld(const Block &blk)
{

}

bool ItemEntity::OnCollision(Entity* ent)
{
	return true;
}

void ItemEntity::Render(float dt,CameraPtr cam)
{
    if(_alive)
    {
        glm::mat4 Model=glm::mat4(1.0f);
        Model=glm::translate(Model,_colShape.GetCenter()-_sprite->aabb.GetHalfSize()/64.f);
        Model=glm::scale(Model,glm::vec3(1.f/64.f,1.f/64.f,1.f/32.f));
        glm::mat4 MVP=cam->GetViewProjMat()*Model;
        MVar<glm::mat4>(0, "mvp", MVP).Set();
        _sprite->Render(false);
        #ifdef DEBUG_ENTITIES
        Model=glm::mat4(1.0f);
        Model=glm::translate(Model,_colShape.GetCenter());
        MVP=cam->GetViewProjMat()*Model;
        MVar<glm::mat4>(0, "mvp", MVP).Set();
        _debugAABBMesh->Render(true);
        #endif
    }
}
