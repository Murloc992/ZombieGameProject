#ifndef SUPERCHUNK_H_INCLUDED
#define SUPERCHUNK_H_INCLUDED

#include "Chunk.h"
#include "OpenGL/Mesh.h"

#define SUPERCHUNK_SIZE 8
#define SUPERCHUNK_SIZEF 8.f
#define SUPERCHUNK_SIZE_BLOCKS (SUPERCHUNK_SIZE*CHUNK_SIZE)
#define SUPERCHUNK_SIZE_BLOCKSF (SUPERCHUNK_SIZEF*CHUNK_SIZEF)
#define VRAM_BLOCK_SIZE (CHUNK_MESH_SIZE*(SUPERCHUNK_SIZE*SUPERCHUNK_SIZE*SUPERCHUNK_SIZE))
#define CHUNK_UPDATES_PER_FRAME 16

inline glm::ivec3 WorldToSuperChunkCoords(const glm::ivec3 &other)
{
    int cx = glm::floor(other.x / SUPERCHUNK_SIZE_BLOCKSF);
    int cy = glm::floor(other.y / SUPERCHUNK_SIZE_BLOCKSF);
    int cz = glm::floor(other.z / SUPERCHUNK_SIZE_BLOCKSF);

    return glm::ivec3(cx,cy,cz);
}

inline glm::ivec3 SuperChunkToWorldCoords(const glm::ivec3 &other)
{
    int cx = other.x * SUPERCHUNK_SIZE_BLOCKS;
    int cy = other.y * SUPERCHUNK_SIZE_BLOCKS;
    int cz = other.z * SUPERCHUNK_SIZE_BLOCKS;

    return glm::ivec3(cx,cy,cz);
}

inline glm::ivec3 ChunkToSuperChunkCoords(const glm::ivec3 &pos)
{
    int cx = glm::floor(pos.x / SUPERCHUNK_SIZEF);
    int cy = glm::floor(pos.y / SUPERCHUNK_SIZEF);
    int cz = glm::floor(pos.z / SUPERCHUNK_SIZEF);

    return glm::ivec3(cx,cy,cz);
}

inline glm::ivec3 SuperChunkSpaceChunkCoords(const glm::ivec3 &pos)
{
    glm::ivec3 ats;

    ats.x = pos.x % SUPERCHUNK_SIZE;
    if(pos.x<0&&ats.x!=0)
        ats.x= SUPERCHUNK_SIZE+ats.x;

    ats.y = pos.y % SUPERCHUNK_SIZE;
    if(pos.y<0&&ats.y!=0)
        ats.y=SUPERCHUNK_SIZE+ats.y;

    ats.z = pos.z % SUPERCHUNK_SIZE;
    if(pos.z<0&&ats.z!=0)
        ats.z=SUPERCHUNK_SIZE+ats.z;

    return ats;
}

inline glm::ivec3 SuperChunkSpaceCoords(const glm::ivec3 &pos)
{
    glm::ivec3 ats;

    ats.x = pos.x % SUPERCHUNK_SIZE_BLOCKS;
    if(pos.x<0&&ats.x!=0)
        ats.x= SUPERCHUNK_SIZE_BLOCKS+ats.x;

    ats.y = pos.y % SUPERCHUNK_SIZE_BLOCKS;
    if(pos.y<0&&ats.y!=0)
        ats.y=SUPERCHUNK_SIZE_BLOCKS+ats.y;

    ats.z = pos.z % SUPERCHUNK_SIZE_BLOCKS;
    if(pos.z<0&&ats.z!=0)
        ats.z=SUPERCHUNK_SIZE_BLOCKS+ats.z;

    return ats;
}

class ChunkManager;

class SuperChunk:public Mesh
{
public:
    ChunkMap _chunks;
    bool built,generated,generating;
private:
    ChunkManager* _chunkManager;
    bool alive;
    uint32_t _occlusion;
    uint32_t _offsetTrack;
    glm::ivec3 _pos;
    float noises[SUPERCHUNK_SIZE_BLOCKS][SUPERCHUNK_SIZE_BLOCKS];
public:
    typedef std::shared_ptr<SuperChunk> _SuperChunkPtr;

    SuperChunk(ChunkManager* chkmgr,const glm::ivec3 &pos);

    virtual ~SuperChunk();

    void SaveToFile();

    void LoadFromFile();

    void Fill();

    void FlagGenerated();

    void SetChunkNeighbours(glm::ivec3 chunkIndex,ChunkPtr chunk);

    void Update(float dt);

    void Set(uint32_t x,uint32_t y,uint32_t z,EBlockType type,bool active);

    const Block &Get(uint32_t x,uint32_t y,uint32_t z);

    uint32_t GetBlockCount();

    void AddChunk(const glm::ivec3 &chunkCoords,const uint32_t & offsetTrack);

    void AddChunk(const glm::ivec3 &chunkCoords);

    ChunkPtr GetChunk(const glm::ivec3 &chunkCoords);

    void UpdateChunkData(ChunkPtr chunk);

    void RebuildIndices();


    void GenerationLoop();

    void BuildingLoop();

    const glm::ivec3 &GetPosition() const
    {
        return _pos;
    }
protected:
};
typedef SuperChunk::_SuperChunkPtr SuperChunkPtr;
typedef boost::unordered_map<glm::ivec3, SuperChunkPtr, ivec3hash, ivec3equal> SuperChunkMap;
#endif // SUPERCHUNK_H_INCLUDED
