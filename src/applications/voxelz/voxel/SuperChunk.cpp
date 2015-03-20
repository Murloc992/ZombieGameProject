#include "Precomp.h"

#include "ChunkManager.h"

#include "SuperChunk.h"

#include "GreedyMeshBuilder.h"


SuperChunk::SuperChunk(ChunkManager* chkmgr,const glm::ivec3 &pos)
{
    this->_chunkManager=chkmgr;
    this->_pos=pos;
    this->_offsetTrack=0;
    built=false;
    _offsetTrack=0;

    BufferObject<u8vec3> *vert=new BufferObject<u8vec3>();
    BufferObject<u8vec4> *col=new BufferObject<u8vec4>();
    IndexBufferObject<uint32_t> *inds = new IndexBufferObject<uint32_t>();

    vert->data.resize(VRAM_BLOCK_SIZE);
    col->data.resize(VRAM_BLOCK_SIZE);
    inds->data.resize(0);

    printf("RESERVED RAM BLOCK: %f Mb\n",(float)(vert->data.size()*sizeof(vert->data[0])+col->data.size()*sizeof(col->data[0]))/1000000.f);

    buffers[Mesh::POSITION]=vert;
    buffers[Mesh::COLOR]=col;
    buffers[Mesh::INDICES]=inds;

    loop(x,SUPERCHUNK_SIZE_BLOCKS)
    loop(y,SUPERCHUNK_SIZE_BLOCKS)
    {
        noises[x][y]=scaled_raw_noise_2d(0,WORLD_HEIGHT,(x+_pos.x)/WORLD_HEIGHTF/2,(y+_pos.z)/WORLD_HEIGHTF/2);
    }

    Init();
    freeVector(((BufferObject<u8vec3>*)buffers[Mesh::POSITION])->data);
    freeVector(((BufferObject<u8vec4>*)buffers[Mesh::COLOR])->data);
}

SuperChunk::~SuperChunk()
{

}

void SuperChunk::SaveToFile()
{
    char buf[256];
    sprintf(buf,"sc_%d%d%d.dat",_pos.x,_pos.y,_pos.z);
    PHYSFS_file* asd=PHYSFS_openWrite(buf);
    PHYSFS_write(asd,(void*)&noises,sizeof(noises),1);
    PHYSFS_close(asd);
}

void SuperChunk::LoadFromFile()
{

}

void SuperChunk::Fill()
{
    loop(x,SUPERCHUNK_SIZE)
    {
        loop(y,SUPERCHUNK_SIZE)
        {
            loop(z,SUPERCHUNK_SIZE)
            {
                AddChunk(glm::ivec3(x,y,z));
                //_chunks[glm::ivec3(x,y,z)]->FillCheckerboard();
            }
        }
    }
}

void SuperChunk::FlagGenerated()
{
    for(auto ch:_chunks)
    {
        ch.second->generated=true;
    }
}

void SuperChunk::SetChunkNeighbours(glm::ivec3 chunkIndex,ChunkPtr chunk)
{
    chunk->leftN=GetChunk(chunkIndex+glm::ivec3(-1,0,0));
    chunk->rightN=GetChunk(chunkIndex+glm::ivec3(1,0,0));

    chunk->botN=GetChunk(chunkIndex+glm::ivec3(0,-1,0));
    chunk->topN=GetChunk(chunkIndex+glm::ivec3(0,1,0));

    chunk->backN=GetChunk(chunkIndex+glm::ivec3(0,0,-1));
    chunk->frontN=GetChunk(chunkIndex+glm::ivec3(0,0,1));
}

void SuperChunk::Update(float dt)
{
    int32_t chunksPerFrame=0;
    if(!built)
    {
        for(auto a:_chunks)
        {
            if(a.second->empty&&!a.second->generated)
            {
                Generate(a.second);
                chunksPerFrame++;
            }
        }
        built=true;
    }
    else
    {
        loop(x,SUPERCHUNK_SIZE)
        loop(y,SUPERCHUNK_SIZE)
        loop(z,SUPERCHUNK_SIZE)
        {
            if(chunksPerFrame!=CHUNK_UPDATES_PER_FRAME)
            {
                if(_chunks.count(glm::ivec3(x,y,z))!=0)
                {
                    glm::ivec3 chunkIndex=glm::ivec3(x,y,z);
                    ChunkPtr currentChunk=_chunks[chunkIndex];
                    if(!currentChunk->empty)
                    {
                        if(currentChunk->generated&&!currentChunk->built)
                        {
                            SetChunkNeighbours(chunkIndex,currentChunk);
                            GreedyMeshBuilder::GreedyBuild(currentChunk);
                            chunksPerFrame++;
                        }
                        else if(currentChunk->generated&&currentChunk->built&&!currentChunk->uploaded)
                        {
                            UpdateChunkData(currentChunk);
                            chunksPerFrame++;
                        }
                    }

                }
            }
            else
            {
                break;
            }
        }
    }
    loop(x,SUPERCHUNK_SIZE)
    loop(y,SUPERCHUNK_SIZE)
    loop(z,SUPERCHUNK_SIZE)
    {
        if(_chunks.count(glm::ivec3(x,y,z))!=0)
        {
            auto chk=_chunks[glm::ivec3(x,y,z)];
            if(chk->built&&chk->generated)
            {
                chk->Update(dt);
            }
        }
    }
}

void SuperChunk::Generate(ChunkPtr chunk)
{
    loop(x,CHUNK_SIZE)
    {
        loop(z,CHUNK_SIZE)
        {
            float noiseval=80;//noises[x+chunk->position.x][z+chunk->position.z];

            loop(y,CHUNK_SIZE)
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

void SuperChunk::Set(uint32_t x,uint32_t y,uint32_t z,EBlockType type,bool active)
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
        _offsetTrack=_offsetTrack+CHUNK_MESH_SIZE;
        _chunks[chunkCoords]->generated=true;
        _chunks[chunkCoords]->SetBlock(voxelCoords.x,voxelCoords.y,voxelCoords.z,type,active);
    }
}

const Block &SuperChunk::Get(uint32_t x,uint32_t y,uint32_t z)
{
    glm::ivec3 pos(x,y,z);
    glm::ivec3 chunkCoords=WorldToChunkCoords(pos),voxelCoords=ChunkSpaceCoords(pos);
    if(_chunks.count(chunkCoords)!=0)
        return _chunks[chunkCoords]->GetBlock(voxelCoords.x,voxelCoords.y,voxelCoords.z);
    else
        return Chunk::EMPTY_BLOCK;
}

uint32_t SuperChunk::GetBlockCount()
{
    uint32_t ret=0;
    for(auto a:_chunks)
    {
        if(!a.second->empty)
            ret+=a.second->GetBlockCount();
    }
    return ret;
}

void SuperChunk::AddChunk(const glm::ivec3 &chunkCoords,const uint32_t & offsetTrack)
{
    _chunks[chunkCoords]=share(new Chunk(this->_chunkManager,ChunkToWorldCoords(chunkCoords),ChunkToWorldCoords(chunkCoords)+_pos,offsetTrack));
}

void SuperChunk::AddChunk(const glm::ivec3 &chunkCoords)
{
    AddChunk(chunkCoords,_offsetTrack);
    _offsetTrack=_offsetTrack+CHUNK_MESH_SIZE;
}

ChunkPtr SuperChunk::GetChunk(const glm::ivec3 &chunkCoords)
{
    if(chunkCoords.x<SUPERCHUNK_SIZE&&chunkCoords.x>=0&&
            chunkCoords.y<SUPERCHUNK_SIZE&&chunkCoords.y>=0&&
            chunkCoords.z<SUPERCHUNK_SIZE&&chunkCoords.z>=0)
    {
        if(_chunks.count(chunkCoords)!=0)
        {
            auto chk=_chunks[chunkCoords];
            if(chk->generated)
            {
                return chk;
            }
        }

    }
    return nullptr;
}

void SuperChunk::UpdateChunkData(ChunkPtr chunk)
{
    if(!chunk->meshData.empty)
    {
        this->UploadBufferSubData(Mesh::POSITION,chunk->meshData.positions,chunk->offset);
        this->UploadBufferSubData(Mesh::COLOR,chunk->meshData.colors,chunk->offset);
        RebuildIndices();
    }
    chunk->uploaded=true;
}

void SuperChunk::RebuildIndices()
{
    IndexBufferObject<uint32_t> *inds = (IndexBufferObject<uint32_t>*)buffers[Mesh::INDICES];
    inds->data.clear();

    uint32_t finalsize=0;

//        for(auto a:_chunks)
//        {
//            finalsize+=a.second->meshData.indices.size();
//        }
//        inds->data.reserve(finalsize);
//
//        for(auto a:_chunks)
//        {
//            finalsize+=a.second->meshData.indices.size();
//        }

    for(auto a:_chunks)
    {
        appendVectors<uint32_t>(inds->data,a.second->meshData.indices);
    }

    glBindVertexArray(vao);
    inds->Upload();
    glBindVertexArray(0);
}
