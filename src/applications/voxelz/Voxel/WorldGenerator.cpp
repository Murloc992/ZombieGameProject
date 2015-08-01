#include "Precomp.h"

#include "SuperChunk.h"

#include "WorldGenerator.h"

WorldGenerator::WorldGenerator(int worldSeed)
{
	//ctor
	_worldSeed = worldSeed;
}

WorldGenerator::~WorldGenerator()
{
	//dtor
}

void WorldGenerator::GenerateSuperChunk(SuperChunkPtr superChunk)
{
	glm::ivec3 cpos = superChunk->GetPosition();
	/// get some noiz
	float noises[SUPERCHUNK_SIZE_BLOCKS*SUPERCHUNK_SIZE_BLOCKS];
	loop(i, SUPERCHUNK_SIZE_BLOCKS)
		loop(j, SUPERCHUNK_SIZE_BLOCKS)
	{
		noises[i*SUPERCHUNK_SIZE_BLOCKS + j] = scaled_octave_noise_2d(8, 1.f / 8.f, 0.5f, 0.f, 128.f, (_worldSeed + i + cpos.x) / SUPERCHUNK_SIZE_BLOCKSF, (_worldSeed + j + cpos.z) / SUPERCHUNK_SIZE_BLOCKSF);
	}
	/// generate the chinky
	loop(x, SUPERCHUNK_SIZE_BLOCKS)
	{
		loop(z, SUPERCHUNK_SIZE_BLOCKS)
		{
			float noiseval = noises[x*SUPERCHUNK_SIZE_BLOCKS + z];

			loop(y, SUPERCHUNK_SIZE_BLOCKS)
			{
				float absoluteY = y + cpos.y;

				if (absoluteY == 0)
				{
					superChunk->Set(x, y, z, EBT_VOIDROCK, true);
					continue;
				}
				else if (absoluteY == (int)noiseval)
				{
					superChunk->Set(x, y, z, EBT_GRASS, true);
					continue;
				}
				else if (absoluteY > noiseval)
				{
					if (absoluteY < 64)
					{
						superChunk->Set(x, y, z, EBT_WATER, true);
					}
					else
					{
						superChunk->Set(x, y, z, EBT_AIR, false);
						continue;
					}
				}
				else
				{
					superChunk->Set(x, y, z, EBT_STONE, true);
				}
			}
		}
	}
	superChunk->FlagGenerated();
}