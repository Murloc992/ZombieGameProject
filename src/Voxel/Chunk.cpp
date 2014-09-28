#include "precomp.h"

#include "Chunk.h"

#include "ChunkManager.h"

#include "opengl/Mesh.h"
#include "resources/ResourceCache.h"
#include <boost/foreach.hpp>

static u8vec4 getTypeCol(uint32_t typ)
{
    switch(typ)
    {
    case EBT_VOIDROCK:
        return u8vec4(0,0,0,255);
        break;
    case EBT_STONE:
        return u8vec4(128,128,128,255);
        break;
    case EBT_SAND:
        return u8vec4(192,192,64,255);
        break;
    case EBT_DIRT:
        return u8vec4(64,64,0,255);
        break;
    case EBT_GRASS:
        return u8vec4(0,128,0,255);
        break;
    case EBT_LEAF:
        return u8vec4(0,192,0,255);
        break;
    case EBT_WOOD:
        return u8vec4(128,128,0,255);
        break;
    case EBT_WATER:
        return u8vec4(0,0,128,128);
        break;
    default:
        return u8vec4(255,255,255,255);
        break;
    }
}

Chunk::Chunk(ChunkManager *chunkManager, glm::vec3 chunkPos):VoxelMesh(CHUNK_SIZE),m_pBlocks(boost::extents[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE])
{
    // Create the blocks
    m_chunkManager = chunkManager;
    m_chunkPos = chunkPos;

    leftN=rightN=botN=topN=backN=frontN=nullptr;
}

Chunk::~Chunk()
{
    Cleanup();
}

void Chunk::UpdateNeighbours()
{
    if(leftN) leftN->Rebuild();
    if(rightN) rightN->Rebuild();
    if(botN) botN->Rebuild();
    if(topN) topN->Rebuild();
    if(backN) backN->Rebuild();
    if(frontN) frontN->Rebuild();
}

void Chunk::Rebuild()
{
    Cleanup();

    for (int x = 0; x < CHUNK_SIZE; x++)
    {
        for (int z = 0; z < CHUNK_SIZE; z++)
        {
            for (int y = 0; y < CHUNK_SIZE; y++)
            {
                if(!m_pBlocks[x][y][z].IsActive()) continue;
                    CreateVox(x,y,z);
            }
        }
    }

    UpdateMesh();

    m_dirty=false;
}

void Chunk::Set(uint32_t x,uint32_t y,uint32_t z,EBlockType type,bool active)
{
    m_pBlocks[x][y][z].SetActive(active);
    m_pBlocks[x][y][z].SetBlockType(type);
    m_dirty=true;
}

const Block &Chunk::Get(uint32_t x,uint32_t y,uint32_t z)
{
    if((x>CHUNK_SIZE-1||x<0)||(y>CHUNK_SIZE-1||y<0)||(z>CHUNK_SIZE-1||z<0))
        return EMPTY_BLOCK;
    return m_pBlocks[x][y][z];
}

void Chunk::Fill()
{
    for (int z = 0; z < CHUNK_SIZE; z++)
    {
        for (int y = 0; y < CHUNK_SIZE; y++)
        {
            for (int x = 0; x < CHUNK_SIZE; x++)
            {
                m_pBlocks[x][y][z].SetBlockType(EBT_STONE);
                m_pBlocks[x][y][z].SetActive(true);
            }
        }
    }
    m_dirty=true;
}

uint32_t Chunk::GetBlockCount()
{
    uint32_t ret=0;

    for (int x = 0; x < CHUNK_SIZE; x++)
    {
        for (int z = 0; z < CHUNK_SIZE; z++)
        {
            for (int y = 0; y < CHUNK_SIZE; y++)
            {
                if(m_pBlocks[x][y][z].IsActive()) ret++;
            }
        }
    }

    return ret;
}

