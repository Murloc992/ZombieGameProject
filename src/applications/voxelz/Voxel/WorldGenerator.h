#ifndef WORLDGENERATOR_H
#define WORLDGENERATOR_H

class SuperChunk;
typedef std::shared_ptr<SuperChunk> SuperChunkPtr;

class WorldGenerator
{
public:
	WorldGenerator(int worldSeed);
	virtual ~WorldGenerator();

	void GenerateSuperChunk(SuperChunkPtr superChunk);
protected:
private:
	int _worldSeed;
};

#endif // WORLDGENERATOR_H
