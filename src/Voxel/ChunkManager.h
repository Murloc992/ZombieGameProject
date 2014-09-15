#ifndef CHUNKMANAGER_H
#define CHUNKMANAGER_H

#include "OpenGL/Shader.h"
#include <boost/unordered_map.hpp>
#include <boost/functional/hash.hpp>

class Camera;
class Block;
class Chunk;
enum EBlockType;

struct chunk_hash : std::unary_function<glm::vec3, std::size_t>
{
    std::size_t operator()(glm::vec3 const& v) const
    {
        std::size_t seed = 0;
        boost::hash_combine(seed, v.x);
        boost::hash_combine(seed, v.y);
        boost::hash_combine(seed, v.z);
        return seed;
    }
};

inline glm::vec3 WorldToChunkCoords(const glm::vec3 &other)
{
    int cx = glm::floor(other.x / CHUNK_SIZEF);
    int cy = glm::floor(other.y / CHUNK_SIZEF);
    int cz = glm::floor(other.z / CHUNK_SIZEF);

    return glm::vec3(cx,cy,cz);
}

inline glm::vec3 ChunkToWorldCoords(const glm::vec3 &other)
{
    int cx = glm::floor(other.x * CHUNK_SIZEF);
    int cy = glm::floor(other.y * CHUNK_SIZEF);
    int cz = glm::floor(other.z * CHUNK_SIZEF);

    return glm::vec3(cx,cy,cz);
}

inline glm::vec3 ChunkSpaceCoords(const glm::vec3 &pos)
{
	glm::vec3 ats;

	ats.x = (int)pos.x % CHUNK_SIZE;
	if(pos.x<0&&ats.x!=0)
        ats.x= (CHUNK_SIZE)+ats.x;

	ats.y = (int)pos.y % CHUNK_SIZE;
	if(pos.y<0&&ats.y!=0)
        ats.y=(CHUNK_SIZE)+ats.y;

	ats.z = (int)pos.z % CHUNK_SIZE;
	if(pos.z<0&&ats.z!=0)
        ats.z=(CHUNK_SIZE)+ats.z;

	return ats;
}

typedef boost::unordered_map<glm::vec3, Chunk*, chunk_hash> ChunkMap;

class ChunkManager
{
public:
    ChunkManager();
    virtual ~ChunkManager();

    void Explode(glm::vec3 pos,float power);

    void SetBlock(glm::vec3 pos,EBlockType type,bool active);
    Block GetBlock(glm::vec3 pos);

    Chunk *GetChunk(glm::vec3 pos);
    Chunk *GetChunkWorld(glm::vec3 pos);
    void SetChunkNeighbours(Chunk* chunk,glm::vec3 pos);

    Chunk *AddChunk(glm::vec3 pos);
    Chunk *AddChunkWorld(glm::vec3 pos);

    void Render(Camera *cam,ShaderPtr vsh);

    void Clear();

    uint32_t GetChunkCount();

    uint32_t GetTotalBlocks();
protected:
private:
    ChunkMap m_chunks;
};

#endif // CHUNKMANAGER_H
