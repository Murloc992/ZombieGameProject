#ifndef CHUNKMANAGER_H
#define CHUNKMANAGER_H
#define WORLD_HEIGHT 128
#define WORLD_HEIGHTF 128.f

#include "opengl/Shader.h"

#include "SuperChunk.h"

class Camera;
enum EBlockType;

class ChunkManager
{
public:
    ChunkManager();
    virtual ~ChunkManager();

    void SetBlock(const glm::ivec3 &pos,EBlockType type,bool active);
    const Block &GetBlock(const glm::ivec3 &pos);

    SuperChunkPtr AddSuperChunk(const glm::ivec3 &pos);
    SuperChunkPtr GetSuperChunk(const glm::ivec3 &pos);
    void RemoveSuperChunk(const glm::ivec3 &pos);

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

    void Render(Camera *cam,ShaderPtr vsh,bool wireframe=false);
protected:
private:
    SuperChunkMap _superChunks;
    static SuperChunk NULL_SUPERCHUNK;
};

#endif // CHUNKMANAGER_H
