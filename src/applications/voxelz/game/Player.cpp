#include "Precomp.h"

#include "Voxel/Block.h"
#include "Application/InputHandler.h"

#include "Player.h"

Player::Player(ChunkManager* chunkManager, const glm::vec3 &feetPos):Entity("Player",glm::vec3(feetPos.x,feetPos.y,feetPos.z),glm::vec3(PLAYER_WIDTH,PLAYER_HEIGHT,PLAYER_WIDTH))
{
    _type=EET_PLAYER;
    _chunkManager=chunkManager;
    _tempMesh=new CubeMesh(AABB(glm::vec3(0),_colShape.GetHalfSize()));
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

void Player::OnCollisionWithWorld(const Block &blk)
{
    Player* stuff=this;
    _velocity=glm::vec3(0);
}

void Player::CalculateSpeed()
{
    if(_velocity.y<GRAVITY_CONSTANT)
    _velocity+=glm::vec3(0,-GRAVITY_CONSTANT,0);
}

void Player::Update(float dt)
{
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
    if(input->IsKeyDown(GLFW_KEY_UP))
    {
        _velocity+=glm::vec3(0,0,5);
    }
    if(input->IsKeyDown(GLFW_KEY_DOWN))
    {
        _velocity+=glm::vec3(0,0,-5);
    }
    if(input->IsKeyDown(GLFW_KEY_LEFT))
    {
        _velocity+=glm::vec3(5,0,0);
    }
    if(input->IsKeyDown(GLFW_KEY_RIGHT))
    {
        _velocity+=glm::vec3(-5,0,0);
    }
}

void Player::Render(float dt)
{
    _tempMesh->Render(false);
}
