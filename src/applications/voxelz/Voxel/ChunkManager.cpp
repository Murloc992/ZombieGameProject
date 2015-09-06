#include "Precomp.h"
#include <future>
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

#define CHUNKS_PER_THREAD 1

ChunkManager::ChunkManager()
{
	std::string stri = "besiprisikiskiakopusteliaudamasis";
	int seed = 0;
	for (char c : stri)
	{
		seed += rand() % (int)(c);
	}
	printf("Seed is: %d\n", seed);
	_worldGenerator = new WorldGenerator(seed);
}

ChunkManager::~ChunkManager()
{
	_removableChunks.clear();
	_chunksToGenerate.clear();
	_chunksToRender.clear();
	_superChunks.clear();
}

void ChunkManager::SetBlock(const glm::ivec3 &pos, EBlockType type, bool active)
{
	glm::ivec3 chunkCoords = WorldToSuperChunkCoords(pos), voxelCoords = SuperChunkSpaceCoords(pos);

	if (_superChunks.find(chunkCoords) != _superChunks.end())
	{
		_superChunks[chunkCoords]->Set(voxelCoords.x, voxelCoords.y, voxelCoords.z, type, active);
	}
	else
	{
		AddSuperChunk(chunkCoords);
		//_superChunks[chunkCoords]->Fill();
		_superChunks[chunkCoords]->Set(voxelCoords.x, voxelCoords.y, voxelCoords.z, type, active);
	}
}

//! pos: in SUPERCHUNK coordinates
SuperChunkPtr ChunkManager::AddSuperChunk(const glm::ivec3 &pos)
{
	if (_superChunks.find(pos) != _superChunks.end())
	{
		return _superChunks[pos];
	}
	else
	{
		_superChunks[pos] = share(new SuperChunk(this, SuperChunkToWorldCoords(pos)));
		return _superChunks[pos];
	}
}

void ChunkManager::RemoveSuperChunk(const glm::ivec3 &pos)
{
	if (_superChunks.find(pos) != _superChunks.end())
	{
		//printf("Attempting to delete: %d %s\n",_superChunks[pos].use_count(),_superChunks[pos].unique()?"true":"false");
		_superChunks[pos] = nullptr;
		_superChunks.erase(pos);
	}
}

//! pos: in SUPER_CHUNK coordinates
SuperChunkPtr ChunkManager::GetSuperChunk(const glm::ivec3 &pos)
{
	if (_superChunks.find(pos) != _superChunks.end())
		return _superChunks[pos];
	else
		return nullptr;
}

//! pos in WORLD coordinates
ChunkPtr ChunkManager::GetChunkWorld(const glm::ivec3 &pos)
{
	glm::ivec3 superchunkPos = WorldToSuperChunkCoords(pos);
	glm::ivec3 superchunkspaceCoords = SuperChunkSpaceCoords(pos);
	glm::ivec3 chunkCoords = WorldToChunkCoords(superchunkspaceCoords);

	if (_superChunks.find(superchunkPos) != _superChunks.end())
	{
		return _superChunks[superchunkPos]->GetChunk(chunkCoords);
	}

	return nullptr;
}

//! pos in CHUNK INDEX coordinates
ChunkPtr ChunkManager::GetChunk(const glm::ivec3 &pos)
{
	glm::ivec3 superchunkPos = ChunkToSuperChunkCoords(pos);
	glm::ivec3 chunkPos = SuperChunkSpaceChunkCoords(pos);

	if (_superChunks.find(superchunkPos) != _superChunks.end())
	{
		return _superChunks[superchunkPos]->GetChunk(chunkPos);
	}

	return nullptr;
}

const Block &ChunkManager::GetBlock(const glm::ivec3 &pos)
{
	glm::ivec3 chunkCoords = WorldToSuperChunkCoords(pos), voxelCoords = SuperChunkSpaceCoords(pos);
	if (_superChunks.find(chunkCoords) != _superChunks.end())
		return _superChunks[chunkCoords]->Get(voxelCoords.x, voxelCoords.y, voxelCoords.z);
	else
		return Chunk::EMPTY_BLOCK;
}

void ChunkManager::AsyncGenerate(vector<SuperChunkPtr> chunksToGenerate)
{
	vector<SuperChunkPtr> internalChunks = chunksToGenerate;

	//printf("Started async.\n");

	while (internalChunks.size() > 0 && internalChunks.size() <= CHUNKS_PER_THREAD)
	{
		for (auto it = internalChunks.begin(); it != internalChunks.end();)
		{
			auto sc = *it;

			if (!sc->generated)
				_worldGenerator->GenerateSuperChunk(sc);

			if (sc->generated&&!sc->built)
				sc->BuildingLoop();

			if (sc->generated&&sc->built)
			{
				sc->generating = false;
				it = internalChunks.erase(it);
			}
			else
				it++;
		}
		//if(genPool->size()>0)
		//printf("GenPool %d size %d\n",id,genPool->size());
	}
	//printf("Thread exitted.\n");
}

void ChunkManager::Update(float dt, Player *player)
{
	_chunksToRender.clear();

	if (_removableChunks.size() > 0)
	{
		//printf("Removable superchunks: %d\n",_removableChunks.size());

		for (auto chp : _removableChunks)
		{
			RemoveSuperChunk(chp);
		}
		_removableChunks.clear();

		//printf("Leftover superchunks: %d\n",_superChunks.size());
	}

	glm::vec3 playerPos = player->GetFeetPos();

	BOOST_FOREACH(SuperChunkMap::value_type a, _superChunks)
	{
		if (!a.second->generating)
		{
			a.second->Update(dt);

			glm::vec3 chunkCenter = (glm::vec3)(a.second->GetPosition()) + glm::vec3(SUPERCHUNK_SIZE_BLOCKSF / 2.f);
			float dist = glm::distance(chunkCenter, playerPos);

			//printf("DEBUG: %s %s %f\n",GLMVec3ToStr(chunkCenter).c_str(),GLMVec3ToStr(playerPos).c_str(),dist);

			if (dist > SUPERCHUNK_SIZE_BLOCKSF*5.f)
			{
				_removableChunks.push_back(a.first);
				continue;
			}
			else //if (a.second->uploaded)
			{
				_chunksToRender.push_back(a.second);
			}
		}
	}

	glm::ivec3 ppmin = (glm::ivec3)(playerPos - glm::vec3(SUPERCHUNK_SIZE_BLOCKS * 2));
	glm::ivec3 ppmax = (glm::ivec3)(playerPos + glm::vec3(SUPERCHUNK_SIZE_BLOCKS * 2));

	if (_chunksToGenerate.size() == 0)
	{
		for (int x = ppmin.x; x <= ppmax.x; x += SUPERCHUNK_SIZE_BLOCKS / 2)
		{
			for (int z = ppmin.z; z <= ppmax.z; z += SUPERCHUNK_SIZE_BLOCKS / 2)
			{
				//for (int y = ppmin.y; y <= ppmax.y; y += SUPERCHUNK_SIZE_BLOCKS / 2)
				//{
				if (_chunksToGenerate.size() >= CHUNKS_PER_THREAD)
				{
					break;
				}

				glm::ivec3 scc = WorldToSuperChunkCoords(glm::ivec3(x, 0, z));
				SuperChunkPtr chunky = GetSuperChunk(scc);


				if (chunky == nullptr)
				{
					//printf("Added a superchunk.\n");
					chunky = AddSuperChunk(scc);
					_chunksToGenerate.push_back(chunky);
					chunky->generating = true;
				}
				else if (!chunky->generating)
				{
					if (!chunky->generated || !chunky->built)
					{
						//printf("Added a NON-GENERATED/BUILT superchunk.\n");
						_chunksToGenerate.push_back(chunky);
						chunky->generating = true;
					}
				}
				else
					continue;
				//}
			}
		}
	}

	if (_chunksToGenerate.size() >= CHUNKS_PER_THREAD || _chunksToGenerate.size() > 0)
	{
		//printf("Created a thread to generate.\n");
		//std::async(std::launch::async, &ChunkManager::AsyncGenerate, this, _chunksToGenerate); // launch on a thread
		std::thread t(&ChunkManager::AsyncGenerate, this, _chunksToGenerate);
		t.detach();
		_chunksToGenerate.clear();
		//printf("Size of _chunksToGenerate: %u\n", _chunksToGenerate.size());
	}
}

void ChunkManager::Render(Camera *cam, ShaderPtr vsh, bool wireframe)
{
	glm::mat4 Model;
	if (wireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	for (auto sc : _chunksToRender)
	{
		//if(!a.second->Empty())
		{
			glm::ivec3 pos = sc->GetPosition();

			Model = glm::mat4(1.0f);
			Model = glm::translate(Model, glm::vec3(pos));
			if (vsh->getparam("M") != -1) MVar<glm::mat4>(vsh->getparam("M"), "M", Model).Set();
			glm::mat4 MVP = cam->GetProjectionMat()*cam->GetViewMat()*Model;
			glm::mat3 normMatrix = glm::inverse(glm::mat3(cam->GetViewMat()*Model));
			if (vsh->getparam("normMatrix") != -1) MVar<glm::mat3>(vsh->getparam("normMatrix"), "normMatrix", normMatrix).Set();
			if (vsh->getparam("MVP") != -1) MVar<glm::mat4>(vsh->getparam("MVP"), "MVP", MVP).Set();
			sc->Render();
		}
	}
	if (wireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

uint32_t ChunkManager::GetChunkCount()
{
	return _superChunks.size();
}