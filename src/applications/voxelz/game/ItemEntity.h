#ifndef ITEMENTITY_H
#define ITEMENTITY_H

#include "Entity.h"
#include "Voxel/VoxelSprite.h"

class VoxelSprite;
class ChunkManager;
class Camera;
typedef std::shared_ptr<Camera> CameraPtr;

class ItemEntity :public Entity
{
public:

public:
	ItemEntity(ChunkManager* chkmgr, const glm::vec3 &pos, const std::string &filename);
	virtual ~ItemEntity();

	void Update(float dt);

	void OnCollisionWithWorld(const Block &blk);
	bool OnCollision(Entity* ent);

	void Render(float dt, CameraPtr cam);
protected:
	ChunkManager* _chkmgr;
	VoxelSprite* _sprite;
private:
};

#endif // ITEMENTITY_H
