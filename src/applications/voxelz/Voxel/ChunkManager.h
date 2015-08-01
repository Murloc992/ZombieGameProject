#ifndef CHUNKMANAGER_H
#define CHUNKMANAGER_H

#define GENERATION_THREAD_COUNT 4

#define WORLD_HEIGHT 128
#define WORLD_HEIGHTF 128.f

#include "opengl/Shader.h"

#include "SuperChunk.h"

class Camera;
enum EBlockType :uint32_t;

class WorldGenerator;

class Player;

class ChunkManager
{
public:
	std::thread generationThreads[GENERATION_THREAD_COUNT];
	std::mutex generationLocks[GENERATION_THREAD_COUNT];
	uint32_t usedThread;
	bool generated, runAsync;

public:
	ChunkManager();
	virtual ~ChunkManager();

	void SetBlock(const glm::ivec3 &pos, EBlockType type, bool active);
	const Block &GetBlock(const glm::ivec3 &pos);

	SuperChunkPtr AddSuperChunk(const glm::ivec3 &pos);
	SuperChunkPtr GetSuperChunk(const glm::ivec3 &pos);
	void RemoveSuperChunk(const glm::ivec3 &pos);
	ChunkPtr GetChunkWorld(const glm::ivec3 &pos);
	ChunkPtr GetChunk(const glm::ivec3 &pos);

	uint32_t GetChunkCount();
	uint32_t GetTotalBlocks()
	{
		uint32_t ret = 0;
		//        for(auto a:_superChunks)
		//        {
		//            ret+=a.second->GetBlockCount();
		//        }
		return ret;
	}

	void FlagGenerated()
	{
		generated = true;
		for (auto sc : _superChunks)
		{
			sc.second->FlagGenerated();
		}
	}

	void Generate();

	void Update(float dt, Player*player);
	void Render(Camera *cam, ShaderPtr vsh, bool wireframe = false);
protected:
private:
	SuperChunkMap _superChunks;

	vector<SuperChunkPtr> _chunksToGenerate;
	vector<glm::ivec3> _removableChunks;

	WorldGenerator *_worldGenerator;

	void AsyncGenerate(vector<SuperChunkPtr> _vec);

	static SuperChunk NULL_SUPERCHUNK;
};

#endif // CHUNKMANAGER_H
