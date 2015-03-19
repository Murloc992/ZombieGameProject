#ifndef PLAYER_H
#define PLAYER_H

#include "Entity.h"

#define PLAYER_HEIGHT 1.75f
#define PLAYER_WIDTH 0.75f


#include "Opengl/CubeMesh.h"

class ChunkManager;
class VoxelMesh;
class InputHandler;
class Camera;
typedef std::shared_ptr<Camera> CameraPtr;

class Player:public Entity
{
public:
    Player(ChunkManager* chunkManager, const glm::vec3 &feetPos);
    virtual ~Player();

    void Update(float dt,CameraPtr cam);
    void Render(float dt);

    bool OnCollision(Entity* ent);
    void OnCollisionWithWorld(const Block &blk);

    void HandleInput(InputHandler* input);

    glm::vec3 GetFeetPos();
    glm::vec3 GetEyePos();
    glm::vec3 GetVelocity();
protected:
private:
    bool _isJumping,_isFalling,_isSwimming;
    float _fallingSpeed,_jumpHeight,_isFlying;

    VoxelMesh* _playerMesh;
    CubeMesh* _tempMesh;
    glm::vec3 _walkingDir,_jumpStartPos,_strafingDir;

    void CalculateSpeed();
};

#endif // PLAYER_H
