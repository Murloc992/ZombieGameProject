#ifndef ENTITY_H_INCLUDED
#define ENTITY_H_INCLUDED

#include "Physics/CollisionObject.h"

#define DEBUG_ENTITIES

enum E_ENTITY_TYPE :uint32_t
{
	EET_DEFAULT,
	EET_PLAYER,
	EET_ITEM,
	EET_ENEMY,
	EET_BLOCK_ENTITY,

	EET_COUNT
};

class ChunkManager;
class CubeMesh;
class Block;
class Chunk;
typedef std::shared_ptr<Chunk> ChunkPtr;

class Entity :public CollisionObject
{
public:
	Entity(ChunkManager* chkmgr, std::string id, const glm::vec3 &pos, const glm::vec3 &size, bool colliding = true, bool collidingWorld = true, bool dynamic = true);
	virtual ~Entity();

	uint32_t GetType();

	std::string GetId();

	bool IsOnGround()
	{
		return _isOnGround;
	}

	bool IsAlive()
	{
		return _alive;
	}

	void Die()
	{
		_alive = false;
	}

	void CheckCollision(Entity* ent);

	virtual void Update(float dt);

	virtual void OnCollisionWithWorld(const Block &blk) = 0;
	virtual bool OnCollision(Entity* ent);

	void OnEnterChunk(Chunk* chunk);
	void OnExitChunk(Chunk* chunk);
private:
protected:
	ChunkManager* _chunkManager;
	bool _alive, _isDynamic, _isColliding, _isCollidingWorld, _isOnGround, _hitCeiling;
	uint32_t _type;
	std::string _id;
	glm::vec3 _velocity;

	vector<Chunk*> _containingChunks;

	void CollideWithWorld(float dt);
	void TrackChunks(const glm::ivec3 &startCoord, const glm::ivec3 &endCoord);
	void TrackBlocks(const glm::ivec3 &startCoord, const glm::ivec3 &endCoord);

	CubeMesh* _debugAABBMesh;
};

#endif // ENTITY_H_INCLUDED
