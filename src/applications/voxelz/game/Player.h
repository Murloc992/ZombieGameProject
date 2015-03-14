#ifndef PLAYER_H
#define PLAYER_H

#include "Entity.h"

#define PLAYER_HEIGHT 2.f
#define PLAYER_WIDTH 1.f
#define GRAVITY_CONSTANT 9.807f

#include "Opengl/CubeMesh.h"

class ChunkManager;
class VoxelMesh;
class InputHandler;

class Player:public Entity
{
public:
    Player(ChunkManager* chunkManager, const glm::vec3 &feetPos);
    virtual ~Player();

    void Update(float dt);
    void Render(float dt);

    bool OnCollision(Entity* ent);
    void OnCollisionWithWorld(const Block &blk);

    void HandleInput(InputHandler* input);
protected:
private:
    bool _isJumping,_isFalling,_isSwimming;
    float _fallingSpeed;

    ChunkManager* _chunkManager;
    VoxelMesh* _playerMesh;
    CubeMesh* _tempMesh;
    glm::vec3 _velocity;

    void CalculateSpeed();
};

#endif // PLAYER_H
