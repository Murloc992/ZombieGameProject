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
    _isFlying=false;
    _isSwimming=false;
    _fallingSpeed=GRAVITY_CONSTANT;
    _jumpHeight=1.f;
}

Player::~Player()
{

}

bool Player::OnCollision(Entity* ent)
{
    switch(ent->GetType())
    {
    default:
        printf("entity.\n");
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
    if(!_isFlying)
    {
        if(_isJumping)
        {
            if(GetFeetPos().y < _jumpStartPos.y+_jumpHeight && !_hitCeiling)
            {
                _velocity+=glm::vec3(0,_fallingSpeed,0);
            }
            else
            {
                _isJumping=false;
                _isFalling=true;
            }
        }
        else
        {
            _isFalling=!_isOnGround;

            if(_isFalling)
            {
                if(_velocity.y<_fallingSpeed)
                {
                    _velocity-=glm::vec3(0,_fallingSpeed,0);
                }
            }
        }

        _velocity*=0.75f;
    }
    else
    {
        _velocity*=0.95f;
    }
}

void Player::Update(float dt,CameraPtr cam)
{
    glm::vec3 d=cam->GetLook();

    _walkingDir=_isFlying||_isSwimming?d:glm::vec3(d.x,0,d.z);
    _strafingDir=cam->GetRight();

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
    float forward=1.f;
    float backward=-1.f;
    float direction=forward;

    if(input->IsKeyDown(GLFW_KEY_W))
    {
        direction=forward;
        _velocity+=_walkingDir*2.f*direction;
    }

    if(input->IsKeyDown(GLFW_KEY_S))
    {
        direction=backward;
        _velocity+=_walkingDir*2.f*direction;
    }

    if(input->IsKeyDown(GLFW_KEY_A))
    {
        direction=backward;
        _velocity+=_strafingDir*2.f*direction;
    }

    if(input->IsKeyDown(GLFW_KEY_D))
    {
        direction=forward;
        _velocity+=_strafingDir*2.f*direction;
    }

    if(input->IsKeyDown(GLFW_KEY_LEFT_SHIFT)&&direction==forward&&(!_isSwimming&&_isOnGround&&!_isJumping))
    {
        _velocity+=_walkingDir*2.f;
    }

    if(input->IsKeyDown(GLFW_KEY_SPACE)&&(_isSwimming||(_isOnGround&&!_isJumping)))
    {
        //_velocity+=_walkingDir*_jumpHeight*2.f;
        _jumpStartPos=GetFeetPos();
        _isJumping=true;
    }

    if(input->IsKeyDown(GLFW_KEY_O))
    {
        _isFlying=!_isFlying;
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
