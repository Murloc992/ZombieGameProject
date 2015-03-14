#include "Precomp.h"

#include "Voxel/Block.h"
#include "Application/InputHandler.h"
#include "Scenegraph/Camera.h"

#include "Player.h"

Player::Player(ChunkManager* chunkManager, const glm::vec3 &feetPos):Entity("Player",glm::vec3(feetPos.x,feetPos.y,feetPos.z),glm::vec3(PLAYER_WIDTH,PLAYER_HEIGHT,PLAYER_WIDTH))
{
    _type=EET_PLAYER;
    _chunkManager=chunkManager;
    _tempMesh=new CubeMesh(AABB(glm::vec3(0),_colShape.GetHalfSize()));
    _isJumping=false;
    _isFalling=false;
}

Player::~Player()
{

}

bool Player::OnCollision(Entity* ent)
{
    switch(ent->GetType())
    {
    default:
        break;
    }
}

glm::vec3 Player::GetVelocity()
{
    return _velocity;
}

void Player::OnCollisionWithWorld(const Block &blk)
{
    switch(blk.type)
    {
    case EBT_WATER:
        _isSwimming=true;
        _fallingSpeed=GRAVITY_CONSTANT/9.f;
        break;
    default:
        _isSwimming=false;
        _fallingSpeed=GRAVITY_CONSTANT;
        break;
    }
}

void Player::CalculateSpeed()
{
    if(!_isOnGround&&_velocity.y<GRAVITY_CONSTANT)
    {
        _velocity+=glm::vec3(0,-GRAVITY_CONSTANT,0);
    }
    else if(_isOnGround||_hitCeiling||_velocity.y>0)
    {
        _isJumping=false;
    }
    _velocity*=0.75;
}

void Player::Update(float dt,CameraPtr cam)
{
    glm::vec3 d=cam->GetLook();
    _walkingDir=glm::vec3(d.x,0,d.z);
    if(_isDynamic)
    {
        CalculateSpeed();

        float spd=
        glm::max(glm::abs(_velocity.x),
                 glm::max(glm::abs(_velocity.y),
                          glm::abs(_velocity.z)));

        int substeps=glm::ceil(spd*dt)*2;
        glm::vec3 stepsteed=(_velocity*dt)/(float)substeps;

        loopi(substeps)
        {
            Translate(stepsteed);
            if(_isCollidingWorld)
            {
                CollideWithWorld(dt,_chunkManager);
            }
        }
    }
}

void Player::HandleInput(InputHandler* input)
{
    if(input->IsKeyDown(GLFW_KEY_W))
    {
        _velocity+=_walkingDir*5.f;
    }

    if(input->IsKeyDown(GLFW_KEY_S))
    {
        _velocity+=_walkingDir*-5.f;
    }

    if(input->IsKeyDown(GLFW_KEY_SPACE)&&(_isSwimming||(_isOnGround&&!_isJumping)))
    {
        _isJumping=true;
        _velocity+=glm::vec3(0,GRAVITY_CONSTANT*4.f,0);
    }
}

void Player::Render(float dt)
{
    _tempMesh->Render(false);
}

glm::vec3 Player::GetFeetPos()
{
    return _colShape.GetCenter()-glm::vec3(0,_colShape.GetHalfSize().y,0);
}

glm::vec3 Player::GetEyePos()
{
    return _colShape.GetCenter()+glm::vec3(0,_colShape.GetHalfSize().y,0);
}
