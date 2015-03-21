#ifndef CHUNKMANAGER_H
#define CHUNKMANAGER_H

#define GENERATION_THREAD_COUNT 2

#define WORLD_HEIGHT 128
#define WORLD_HEIGHTF 128.f

#include "opengl/Shader.h"

#include "SuperChunk.h"

class Camera;
enum EBlockType;

class ChunkManager
{
public:
    std::thread generationThreads[GENERATION_THREAD_COUNT];
    uint32_t usedThreads;
    bool generated;

public:
    ChunkManager();
    virtual ~ChunkManager();

    void SetBlock(const glm::ivec3 &pos,EBlockType type,bool active);
    const Block &GetBlock(const glm::ivec3 &pos);

    SuperChunkPtr AddSuperChunk(const glm::ivec3 &pos);
    SuperChunkPtr GetSuperChunk(const glm::ivec3 &pos);
    void RemoveSuperChunk(const glm::ivec3 &pos);
    ChunkPtr GetChunkWorld(const glm::ivec3 &pos);
    ChunkPtr GetChunk(const glm::ivec3 &pos);

    uint32_t GetChunkCount();
    uint32_t GetTotalBlocks()
    {
        uint32_t ret=0;
        for(auto a:_superChunks)
        {
            ret+=a.second->GetBlockCount();
        }
        return ret;
    }

    void FlagGenerated()
    {
        for(auto sc:_superChunks)
        {
            sc.second->FlagGenerated();
        }
    }

    void Generate()
    {
        if(!generated)
        {
            for(int x=0; x<2; x++)
        for(int z=0; z<2; z++)
        {
            AddSuperChunk(glm::ivec3(x,0,z));
            _superChunks[glm::ivec3(x,0,z)]->Fill();
        }
        generated=true;
        }
    }

    void Update(float dt);
    void Render(Camera *cam,ShaderPtr vsh,bool wireframe=false);
protected:
private:
    SuperChunkMap _superChunks;
    vector<vector<SuperChunkPtr>> _generationPools;
    vector<SuperChunkPtr> GetLeastBusyGenerationPool();
    void AsyncGeneration(int id);

    static SuperChunk NULL_SUPERCHUNK;
};

#endif // CHUNKMANAGER_H
