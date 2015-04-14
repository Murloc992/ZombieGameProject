#include "Precomp.h"
#include "Voxel/Chunk.h"
#include "ChunkManager.h"
#include "scenegraph/Camera.h"
#include "opengl/Shader.h"
#include "opengl/MVar.h"
#include "utility/SimplexNoise.h"
#include "utility/Timer.h"
#include "WorldGenerator.h"
#include "Game/Player.h"
#include <boost/foreach.hpp>

ChunkManager::ChunkManager()
{
    usedThreads=0;
    generated=false;
    runAsync=true;
    loopi(GENERATION_THREAD_COUNT)
    {
        _generationPools.push_back(vector<SuperChunkPtr>());
        _generationPools[i].reserve(32);
        generationThreads[i]=std::thread(AsyncGeneration,this,i);
    }

    std::string stri="hakunamytatas";
    int seed=0;
    for(char c:stri)
    {
        seed+=(int)(c);
    }

    _worldGenerator=new WorldGenerator(seed);

    //Generate();
}

ChunkManager::~ChunkManager()
{
    runAsync=false;
    loopi(GENERATION_THREAD_COUNT)
    {
        generationThreads[i].join();
    }
    _superChunks.clear();
}

void ChunkManager::SetBlock(const glm::ivec3 &pos,EBlockType type,bool active)
{
    glm::ivec3 chunkCoords=WorldToSuperChunkCoords(pos),voxelCoords=SuperChunkSpaceCoords(pos);

    if(_superChunks.count(chunkCoords)!=0)
    {
        _superChunks[chunkCoords]->Set(voxelCoords.x,voxelCoords.y,voxelCoords.z,type,active);
    }
    else
    {
        AddSuperChunk(chunkCoords);
        //_superChunks[chunkCoords]->Fill();
        _superChunks[chunkCoords]->Set(voxelCoords.x,voxelCoords.y,voxelCoords.z,type,active);
    }
}

//! pos: in SUPERCHUNK coordinates
SuperChunkPtr ChunkManager::AddSuperChunk(const glm::ivec3 &pos)
{
    if(_superChunks.count(pos)!=0)
    {
        return _superChunks[pos];
    }
    else
    {
        _superChunks[pos]=share(new SuperChunk(this,SuperChunkToWorldCoords(pos)));
        return _superChunks[pos];
    }
}

void ChunkManager::RemoveSuperChunk(const glm::ivec3 &pos)
{
    if(_superChunks.count(pos)!=0)
    {
        _superChunks.erase(pos);
        //_superChunks[pos]=nullptr;
    }
}

//! pos: in SUPER_CHUNK coordinates
SuperChunkPtr ChunkManager::GetSuperChunk(const glm::ivec3 &pos)
{
    if(_superChunks.count(pos)!=0)
        return _superChunks[pos];
    else
        return nullptr;
}

//! pos in WORLD coordinates
ChunkPtr ChunkManager::GetChunkWorld(const glm::ivec3 &pos)
{
    glm::ivec3 superchunkPos=WorldToSuperChunkCoords(pos);
    glm::ivec3 superchunkspaceCoords=SuperChunkSpaceCoords(pos);
    glm::ivec3 chunkCoords=WorldToChunkCoords(superchunkspaceCoords);

    if(_superChunks.count(superchunkPos)!=0)
    {
        return _superChunks[superchunkPos]->GetChunk(chunkCoords);
    }
}

//! pos in CHUNK INDEX coordinates
ChunkPtr ChunkManager::GetChunk(const glm::ivec3 &pos)
{
    glm::ivec3 superchunkPos=ChunkToSuperChunkCoords(pos);
    glm::ivec3 chunkPos=SuperChunkSpaceChunkCoords(pos);

    if(_superChunks.count(superchunkPos)!=0)
    {
        return _superChunks[superchunkPos]->GetChunk(chunkPos);
    }

    return nullptr;
}

const Block &ChunkManager::GetBlock(const glm::ivec3 &pos)
{
    glm::ivec3 chunkCoords=WorldToSuperChunkCoords(pos),voxelCoords=SuperChunkSpaceCoords(pos);
    if(_superChunks.count(chunkCoords)!=0)
        return _superChunks[chunkCoords]->Get(voxelCoords.x,voxelCoords.y,voxelCoords.z);
    else
        return Chunk::EMPTY_BLOCK;
}

vector<SuperChunkPtr> ChunkManager::GetLeastBusyGenerationPool()
{
    usedThreads=(usedThreads+1)%GENERATION_THREAD_COUNT;
    return _generationPools[usedThreads];
//return *boost::min_element(_generationPools);
}

void ChunkManager::AsyncGeneration(int id)
{
    while(runAsync)
    {
        LockGuard(generationLock);
        auto generationPool=_generationPools[id];
        if(generationPool.size()>0)
        {
            for(auto sc:generationPool)
            {
                /// For now, generate first, build later
                /// later: worldGenerator -> generate (superchunk)
                if(!sc->generated)
                    _worldGenerator->GenerateSuperChunk(sc);

                if(sc->generated&&!sc->built)
                    sc->BuildingLoop();
            }
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}

void ChunkManager::Generate()
{
    if(!generated)
    {
        for(int x=0; x<2; x++)
        for(int z=0; z<2; z++)
        for(int y=0; y<1; y++)
        {
            auto sc=AddSuperChunk(glm::ivec3(x,y,z));
        }
        generated=true;
    }
}

void ChunkManager::Update(float dt,Player *player)
{
    for(auto gp:_generationPools)
    {
        if(gp.size()>0)
            gp.clear();
    }

    glm::ivec3 plPos=(glm::ivec3)player->GetFeetPos();

    for(int x=plPos.x-SUPERCHUNK_SIZE_BLOCKS*2; x<=plPos.x+SUPERCHUNK_SIZE_BLOCKS*2; x+=64)
    {
        for(int z=plPos.z-SUPERCHUNK_SIZE_BLOCKS*2; z<=plPos.z+SUPERCHUNK_SIZE_BLOCKS*2; z+=64)
        {
            glm::ivec3 scp=WorldToSuperChunkCoords(glm::ivec3(x,0,z));
            if(GetSuperChunk(scp)!=nullptr)
                continue;
            else
                AddSuperChunk(scp);
        }
    }

    BOOST_FOREACH(SuperChunkMap::value_type a,_superChunks)
    {
        if(a.second==nullptr)
            continue;

//        if(glm::abs(glm::distance((glm::vec3)(a.second->GetPosition())+glm::vec3(64.f),player->GetFeetPos()))>128.f)
//        {
//            RemoveSuperChunk(a.first);
//            continue;
//        }

        /// Uploads will be handled here as they must be synchronous
        a.second->Update(dt);
        if(!a.second->generated)
        {
            auto pool=GetLeastBusyGenerationPool();
            _generationPools[usedThreads].push_back(a.second);
        }
    }
}

void ChunkManager::Render(Camera *cam,ShaderPtr vsh,bool wireframe)
{
    glm::mat4 Model;
    if(wireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
    }
    BOOST_FOREACH(SuperChunkMap::value_type a,_superChunks)
    {
        //if(!a.second->Empty())
        {
            glm::vec3 pos=glm::vec3(a.first)*SUPERCHUNK_SIZE_BLOCKSF;

            Model = glm::mat4(1.0f);
            Model = glm::translate(Model,pos);
            if(vsh->getparam("M")!=-1) MVar<glm::mat4>(vsh->getparam("M"), "M", Model).Set();
            glm::mat4 MVP=cam->GetProjectionMat()*cam->GetViewMat()*Model;
            glm::mat3 normMatrix = glm::inverse(glm::mat3(cam->GetViewMat()*Model));
            if(vsh->getparam("normMatrix")!=-1) MVar<glm::mat3>(vsh->getparam("normMatrix"), "normMatrix", normMatrix).Set();
            if(vsh->getparam("MVP")!=-1) MVar<glm::mat4>(vsh->getparam("MVP"), "MVP", MVP).Set();
            a.second->Render();
        }
    }
    if(wireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
    }
}

uint32_t ChunkManager::GetChunkCount()
{
    return _superChunks.size();
}

