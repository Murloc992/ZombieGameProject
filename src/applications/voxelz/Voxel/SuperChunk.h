#ifndef SUPERCHUNK_H_INCLUDED
#define SUPERCHUNK_H_INCLUDED

#define SUPERCHUNK_SIZE 8
#define SUPERCHUNK_SIZEF 8.f
#define SUPERCHUNK_SIZE_BLOCKS (SUPERCHUNK_SIZE*CHUNK_SIZE)
#define SUPERCHUNK_SIZE_BLOCKSF (SUPERCHUNK_SIZEF*CHUNK_SIZEF)
#define CHUNK_BLOCK_SIZE ((CHUNK_SIZE+1)*(CHUNK_SIZE+1)*(CHUNK_SIZE+1))
#define VRAM_BLOCK_SIZE (CHUNK_BLOCK_SIZE*(SUPERCHUNK_SIZE*SUPERCHUNK_SIZE*SUPERCHUNK_SIZE))
#define CHUNK_UPDATES_PER_FRAME 16

#include "OpenGL/Mesh.h"
#include "GreedyMeshBuilder.h"
#include "Chunk.h"

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

class SuperChunk:public Mesh
{
public:
private:
    ChunkManager* _chunkManager;
    ChunkMap _chunks;
    uint32_t _occlusion;
    uint32_t _offsetTrack;
    glm::ivec3 _pos;
    float noises[SUPERCHUNK_SIZE_BLOCKS][SUPERCHUNK_SIZE_BLOCKS];
    vector<Chunk> _genList;
    bool built;
public:
    typedef std::shared_ptr<SuperChunk> _SuperChunkPtr;
    SuperChunk(ChunkManager* chkmgr,const glm::ivec3 &pos)
    {
        this->_chunkManager=chkmgr;
        this->_pos=pos;
        this->_offsetTrack=0;
        built=false;

        BufferObject<glm::ivec3> *vert=new BufferObject<glm::ivec3>();
        BufferObject<u8vec4> *col=new BufferObject<u8vec4>();
        IndexBufferObject<uint32_t> *inds = new IndexBufferObject<uint32_t>();

        printf("RESERVED RAM BLOCK: %f Mb\n",(float)(VRAM_BLOCK_SIZE*sizeof(glm::ivec3)+VRAM_BLOCK_SIZE*sizeof(glm::ivec3))/1000000.f);
        vert->data.resize(VRAM_BLOCK_SIZE);
        col->data.resize(VRAM_BLOCK_SIZE);
        inds->data.resize(0);

        buffers[Mesh::POSITION]=vert;
        buffers[Mesh::COLOR]=col;
        buffers[Mesh::INDICES]=inds;

        loopi(x,SUPERCHUNK_SIZE_BLOCKS)
        loopi(y,SUPERCHUNK_SIZE_BLOCKS)
        {
            noises[x][y]=scaled_raw_noise_2d(0,WORLD_HEIGHT,(x+_pos.x)/WORLD_HEIGHTF/2,(y+_pos.z)/WORLD_HEIGHTF/2);
        }

        Init();
    }

    ~SuperChunk()
    {
    }

    void Fill()
    {
        loopi(x,SUPERCHUNK_SIZE)
        {
            loopi(z,SUPERCHUNK_SIZE)
            {
                loopi(y,SUPERCHUNK_SIZE)
                {
                    AddChunk(glm::ivec3(x,y,z));
                    //_chunks[glm::ivec3(x,y,z)]->Fill();
                }
            }
        }
    }

    void Update()
    {
        int32_t chunksPerFrame=0;
        if(!built)
        {
            for(auto a:_chunks)
            {
                if(!a.second->generated)
                {
                    //printf("Generate\n");
                    Generate(a.second);
                    chunksPerFrame++;
                }
            }
            built=true;
            for(auto a:_chunks)
            {
                glm::ivec3 pos=a.first;
                a.second->leftN=GetChunk(glm::ivec3(pos.x-1,pos.y,pos.z));
                a.second->rightN=GetChunk(glm::ivec3(pos.x+1,pos.y,pos.z));
                a.second->botN=GetChunk(glm::ivec3(pos.x,pos.y-1,pos.z));
                a.second->topN=GetChunk(glm::ivec3(pos.x,pos.y+1,pos.z));
                a.second->frontN=GetChunk(glm::ivec3(pos.x,pos.y,pos.z+1));
                a.second->backN=GetChunk(glm::ivec3(pos.x,pos.y,pos.z-1));
            }
        }
        else{
        for(auto a:_chunks)
        {
            if(chunksPerFrame!=CHUNK_UPDATES_PER_FRAME)
            {
                if(a.second->generated&&!a.second->built)
                {
                    //printf("Build\n");
                    GreedyMeshBuilder::GreedyBuild(a.second);
                    chunksPerFrame++;
                }
                else if(a.second->generated&&a.second->built&&!a.second->uploaded)
                {
                    //printf("Upload\n");
                    UpdateChunkData(a.second);
                    chunksPerFrame++;
                }
            }
            else
            {
                break;
            }
        }
        }
    }

    void Generate(ChunkPtr chunk)
    {
        loopi(x,CHUNK_SIZE)
        {
            loopi(z,CHUNK_SIZE)
            {
                float noiseval=noises[x+chunk->position.x][z+chunk->position.z];

                loopi(y,CHUNK_SIZE)
                {
                    float absoluteY=y+chunk->position.y+this->_pos.y;

                    if(absoluteY==0)
                    {
                        chunk->SetBlock(x,y,z,EBT_VOIDROCK,true);
                        continue;
                    }
                    else if(absoluteY==(int)noiseval)
                    {
                        chunk->SetBlock(x,y,z,EBT_GRASS,true);
                        continue;
                    }
                    else if(absoluteY>noiseval)
                    {
                        if(absoluteY<64)
                        {
                            chunk->SetBlock(x,y,z,EBT_WATER,true);
                        }
                        else
                        {
                            chunk->SetBlock(x,y,z,EBT_AIR,false);
                            continue;
                        }
                        //trees

                    }
                    else
                    {
                        chunk->SetBlock(x,y,z,EBT_STONE,true);
                    }
                }
            }
        }
        chunk->generated=true;
    }

    void Set(uint32_t x,uint32_t y,uint32_t z,EBlockType type,bool active)
    {
        glm::ivec3 pos(x,y,z);
        glm::ivec3 chunkCoords=WorldToChunkCoords(pos),voxelCoords=ChunkSpaceCoords(pos);

        if(_chunks.count(chunkCoords)!=0)
        {
            _chunks[chunkCoords]->SetBlock(voxelCoords.x,voxelCoords.y,voxelCoords.z,type,active);
            _chunks[chunkCoords]->built=false;
            _chunks[chunkCoords]->uploaded=false;
        }
        else
        {
            AddChunk(chunkCoords,_offsetTrack);
            _offsetTrack=_offsetTrack+CHUNK_BLOCK_SIZE;
            _chunks[chunkCoords]->SetBlock(voxelCoords.x,voxelCoords.y,voxelCoords.z,type,active);
        }
    }

    const Block &Get(uint32_t x,uint32_t y,uint32_t z)
    {
        glm::ivec3 pos(x,y,z);
        glm::ivec3 chunkCoords=WorldToChunkCoords(pos),voxelCoords=ChunkSpaceCoords(pos);
        if(_chunks.count(chunkCoords)!=0)
            return _chunks[chunkCoords]->GetBlock(voxelCoords.x,voxelCoords.y,voxelCoords.z);
        else
            return Chunk::EMPTY_BLOCK;
    }

    void AddChunk(const glm::ivec3 &chunkCoords,const uint32_t & offsetTrack)
    {
        _chunks[chunkCoords]=share(new Chunk(this->_chunkManager,ChunkToWorldCoords(chunkCoords),offsetTrack));
    }

    void AddChunk(const glm::ivec3 &chunkCoords)
    {
        AddChunk(chunkCoords,_offsetTrack);
        _offsetTrack=_offsetTrack+CHUNK_BLOCK_SIZE;
    }

    ChunkPtr GetChunk(const glm::ivec3 &chunkCoords)
    {
        if(chunkCoords.x<SUPERCHUNK_SIZE&&chunkCoords.x>=0&&
                chunkCoords.y<SUPERCHUNK_SIZE&&chunkCoords.y>=0&&
                chunkCoords.z<SUPERCHUNK_SIZE&&chunkCoords.z>=0)
        {
            if(_chunks.count(chunkCoords)!=0)
                return _chunks[chunkCoords];
            else
            {
                AddChunk(chunkCoords);
                return _chunks[chunkCoords];
            }
        }
        return nullptr;
    }

    void UpdateChunkData(ChunkPtr chunk)
    {
        this->UploadBufferSubData(Mesh::POSITION,chunk->meshData.positions,chunk->offset);
        this->UploadBufferSubData(Mesh::COLOR,chunk->meshData.colors,chunk->offset);

        RebuildIndices();
        chunk->uploaded=true;
    }

    void RebuildIndices()
    {
        IndexBufferObject<uint32_t> *inds = (IndexBufferObject<uint32_t>*)buffers[Mesh::INDICES];
        inds->data.clear();

        for(auto a:_chunks)
        {
            appendVectors<uint32_t>(inds->data,a.second->meshData.indices);
        }

        glBindVertexArray(vao);
        inds->Upload();
        glBindVertexArray(0);
    }
protected:
};
typedef SuperChunk::_SuperChunkPtr SuperChunkPtr;
typedef boost::unordered_map<glm::ivec3, SuperChunkPtr, ivec3hash, ivec3equal> SuperChunkMap;
#endif // SUPERCHUNK_H_INCLUDED
