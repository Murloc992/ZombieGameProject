#include "Precomp.h"
#include "Voxel/Block.h"
#include "Voxel/Chunk.h"
#include "ChunkManager.h"
#include "Scenegraph/Camera.h"
#include "opengl/Shader.h"
#include "opengl/MVar.h"
#include "SimplexNoise.h"

ChunkManager::ChunkManager()
{
    int testsize=128;
    int testheight=32;

    for(int i=-testsize; i<testsize; i++)
    {
        for(int j=-testsize; j<testsize; j++)
        {
            float h=scaled_raw_noise_2d(0,testheight,i/128.f,j/128.f);
            for(int y=0; y<testheight; y++)
            {
                if(y==0)
                {
                    Set(glm::vec3(i,y,j),EBT_VOIDROCK,true);
                    continue;
                }
                else if(y==(int)h)
                {
                    Set(glm::vec3(i,y,j),EBT_GRASS,true);
                    continue;
                }
                else if(y>h)
                {
                    break;
                }
                else
                {
                    Set(glm::vec3(i,y,j),EBT_STONE,true);
                }
            }
        }
    }
    //ctor
//    m_chunks[glm::vec3(0,0,0)]=new Chunk();
//    m_chunks[glm::vec3(0,0,0)]->SetupSphere();
//    m_chunks[glm::vec3(0,0,0)]->Generate();
//
//    m_chunks[glm::vec3(1,1,1)]=new Chunk();
//    m_chunks[glm::vec3(1,1,1)]->SetupSphere();
//    m_chunks[glm::vec3(1,1,1)]->Generate();

//    m_chunks[glm::vec3(-1,-1,-1)]=new Chunk();
//    m_chunks[glm::vec3(-1,-1,-1)]->SetupSphere();
//    m_chunks[glm::vec3(-1,-1,-1)]->Generate();
//
//    m_chunks[glm::vec3(-1,0,-1)]=new Chunk();
//    m_chunks[glm::vec3(-1,0,-1)]->SetupSphere();
//    m_chunks[glm::vec3(-1,0,-1)]->Generate();
//
//    m_chunks[glm::vec3(0,0,-1)]=new Chunk();
//    m_chunks[glm::vec3(0,0,-1)]->SetupSphere();
//    m_chunks[glm::vec3(0,0,-1)]->Generate();
//
//    m_chunks[glm::vec3(-1,0,0)]=new Chunk();
//    m_chunks[glm::vec3(-1,0,0)]->SetupSphere();
//    m_chunks[glm::vec3(-1,0,0)]->Generate();
}

ChunkManager::~ChunkManager()
{
    //dtor
}

void ChunkManager::Set(glm::vec3 pos,EBlockType type,bool active)
{
    glm::vec3 chunkCoords=WorldToChunkCoords(pos),voxelCoords=ChunkSpaceCoords(pos);

    if(m_chunks.find(chunkCoords)!=m_chunks.end())
    {
        m_chunks[chunkCoords]->Set(voxelCoords.x,voxelCoords.y,voxelCoords.z,type,active);
    }
    else
    {
        m_chunks[chunkCoords]=new Chunk();
        m_chunks[chunkCoords]->Set(voxelCoords.x,voxelCoords.y,voxelCoords.z,type,active);
    }
}

Block ChunkManager::Get(glm::vec3 pos)
{
    glm::vec3 chunkCoords=WorldToChunkCoords(pos),voxelCoords=ChunkSpaceCoords(pos);
    if(m_chunks.find(chunkCoords)!=m_chunks.end())
        return m_chunks[chunkCoords]->Get(voxelCoords.x,voxelCoords.y,voxelCoords.z);
    else
        return Block();
}

void ChunkManager::Render(Camera *cam)
{

    glm::mat4 Model,MVP;

    for(std::pair<glm::vec3,Chunk*> a:m_chunks)
    {
        glm::vec3 pos=a.first*CHUNK_SIZEF;

        Model = glm::mat4(1.0f);
        Model = glm::translate(Model,pos);
        MVP   = cam->GetViewProjMat() * Model;
        MVar<glm::mat4>(0, "mvp", MVP).Set();
        a.second->Render();
    }
}
