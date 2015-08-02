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

vector<SuperChunkPtr> ChunkManager::_chunksToGenerate;
vector<glm::ivec3> ChunkManager::_removableChunks;

ChunkManager::ChunkManager()
{
	std::string stri = "hakunamytatas";
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
	_superChunks.clear();
}

void ChunkManager::SetBlock(const glm::ivec3 &pos, EBlockType type, bool active)
{
	glm::ivec3 chunkCoords = WorldToSuperChunkCoords(pos), voxelCoords = SuperChunkSpaceCoords(pos);

	if (_superChunks.count(chunkCoords) != 0)
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
	if (_superChunks.count(pos) != 0)
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
	if (_superChunks.count(pos) != 0)
	{
		//printf("Attempting to delete: %d %s\n",_superChunks[pos].use_count(),_superChunks[pos].unique()?"true":"false");
		_superChunks[pos] = nullptr;
		_superChunks.erase(pos);
	}
}

//! pos: in SUPER_CHUNK coordinates
SuperChunkPtr ChunkManager::GetSuperChunk(const glm::ivec3 &pos)
{
	if (_superChunks.count(pos) != 0)
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

	if (_superChunks.count(superchunkPos) != 0)
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

	if (_superChunks.count(superchunkPos) != 0)
	{
		return _superChunks[superchunkPos]->GetChunk(chunkPos);
	}

	return nullptr;
}

const Block &ChunkManager::GetBlock(const glm::ivec3 &pos)
{
	glm::ivec3 chunkCoords = WorldToSuperChunkCoords(pos), voxelCoords = SuperChunkSpaceCoords(pos);
	if (_superChunks.count(chunkCoords) != 0)
		return _superChunks[chunkCoords]->Get(voxelCoords.x, voxelCoords.y, voxelCoords.z);
	else
		return Chunk::EMPTY_BLOCK;
}

void ChunkManager::AsyncGenerate()
{
	while (_chunksToGenerate.size() > 0 && _chunksToGenerate.size() <= 4)
	{
		for (auto it = _chunksToGenerate.begin(); it != _chunksToGenerate.end();)
		{
			auto sc = *it;
			if (!sc->generated)
				_worldGenerator->GenerateSuperChunk(sc);

			if (sc->generated&&!sc->built)
				sc->BuildingLoop();

			if (sc->generated&&sc->built)
			{
				it = _chunksToGenerate.erase(it);
			}
			else
				it++;
		}
		//if(genPool->size()>0)
		//printf("GenPool %d size %d\n",id,genPool->size());
	}
}

void ChunkManager::Update(float dt, Player *player)
{
	//if (_removableChunks.size() > 0)
	//{
	//	//printf("Removable superchunks: %d\n",_removableChunks.size());

	//	for (auto chp : _removableChunks)
	//	{
	//		RemoveSuperChunk(chp);
	//	}
	//	_removableChunks.clear();

	//	//printf("Leftover superchunks: %d\n",_superChunks.size());
	//}

	glm::vec3 playerPos = player->GetFeetPos();

	BOOST_FOREACH(SuperChunkMap::value_type a, _superChunks)
	{
		if (a.second->generated&&a.second->built)
		{
			glm::vec3 chunkCenter = (glm::vec3)(a.second->GetPosition()) + glm::vec3(SUPERCHUNK_SIZE_BLOCKSF / 2.f);
			float dist = glm::distance(chunkCenter, playerPos);

			//printf("DEBUG: %s %s %f\n",GLMVec3ToStr(chunkCenter).c_str(),GLMVec3ToStr(playerPos).c_str(),dist);

			//if (dist > SUPERCHUNK_SIZE_BLOCKSF*3.f)
			//{
			//	_removableChunks.push_back(a.first);
			//	continue;
			//}
		}

		a.second->Update(dt);
	}

	glm::ivec3 ppmin = (glm::ivec3)(playerPos - glm::vec3(SUPERCHUNK_SIZE_BLOCKS));
	glm::ivec3 ppmax = (glm::ivec3)(playerPos + glm::vec3(SUPERCHUNK_SIZE_BLOCKS));

	if (_chunksToGenerate.size() == 0)
	{
		for (int x = ppmin.x; x <= ppmax.x; x += SUPERCHUNK_SIZE_BLOCKS / 2)
			for (int z = ppmin.z; z <= ppmax.z; z += SUPERCHUNK_SIZE_BLOCKS / 2)
				for (int y = ppmin.y; y <= ppmax.y; y += SUPERCHUNK_SIZE_BLOCKS / 2)
				{
					if (_chunksToGenerate.size() >= 4)
					{
						//std::async(std::launch::deferred, &ChunkManager::AsyncGenerate, this);
						break;
					}

					glm::ivec3 scc = WorldToSuperChunkCoords(glm::ivec3(x, y, z));
					SuperChunkPtr chunky = GetSuperChunk(scc);

					if (chunky == nullptr)
					{
						_chunksToGenerate.push_back(AddSuperChunk(scc));
					}
					else if (chunky->generated && !chunky->built)
					{
						_chunksToGenerate.push_back(chunky);
					}
					else
						continue;
				}
	}
	else
	{
		std::async(std::launch::async, &ChunkManager::AsyncGenerate, this); // launch on a thread
	}
}

void ChunkManager::Render(Camera *cam, ShaderPtr vsh, bool wireframe)
{
	glm::mat4 Model;
	if (wireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	BOOST_FOREACH(SuperChunkMap::value_type a, _superChunks)
	{
		//if(!a.second->Empty())
		{
			glm::vec3 pos = glm::vec3(a.first)*SUPERCHUNK_SIZE_BLOCKSF;

			Model = glm::mat4(1.0f);
			Model = glm::translate(Model, pos);
			if (vsh->getparam("M") != -1) MVar<glm::mat4>(vsh->getparam("M"), "M", Model).Set();
			glm::mat4 MVP = cam->GetProjectionMat()*cam->GetViewMat()*Model;
			glm::mat3 normMatrix = glm::inverse(glm::mat3(cam->GetViewMat()*Model));
			if (vsh->getparam("normMatrix") != -1) MVar<glm::mat3>(vsh->getparam("normMatrix"), "normMatrix", normMatrix).Set();
			if (vsh->getparam("MVP") != -1) MVar<glm::mat4>(vsh->getparam("MVP"), "MVP", MVP).Set();
			a.second->Render();
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