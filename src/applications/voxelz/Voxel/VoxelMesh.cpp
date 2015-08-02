#include "Precomp.h"

#include "opengl/Mesh.h"
#include "resources/ResourceCache.h"
#include "Chunk.h"
#include "VoxelMesh.h"
#include "utility/Timer.h"
#include "GreedyMeshBuilder.h"

Voxel VoxelMesh::EMPTY_VOXEL = Voxel();

VoxelMesh::VoxelMesh(const u16vec3 &size)
{
	_vox.resize(size.x*size.y*size.z);

	_size = size;
	_dirty = false;

	buffers[Mesh::POSITION] = new BufferObject<glm::ivec3>();
	buffers[Mesh::COLOR] = new BufferObject<u8vec4>();
	buffers[Mesh::INDICES] = new IndexBufferObject<uint32_t>();

	//    ((BufferObject<glm::ivec3> *) buffers[Mesh::POSITION])->data.reserve((uint32_t)glm::pow(size/2.f,3.f));
	//    ((BufferObject<u8vec4> *) buffers[Mesh::COLOR])->data.reserve((uint32_t)glm::pow(size/2.f,3.f));
	//    ((BufferObject<uint32_t> *) buffers[Mesh::INDICES])->data.reserve((uint32_t)glm::pow(size/2.f,3.f)*36);

	_faceCount = 0;
	_empty = true;

	Init();
}

VoxelMesh::~VoxelMesh()
{
	Cleanup();
}

bool VoxelMesh::Empty()
{
	return _empty;
}

void VoxelMesh::Render(bool wireframe)
{
	if (wireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	if (!Empty() && !_dirty)
	{
		Mesh::Render();
	}

	if (wireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

void VoxelMesh::Cleanup()
{
	if (!Empty())
	{
		freeVector(_vox);

		freeVector(((BufferObject<glm::ivec3> *) buffers[Mesh::POSITION])->data);
		freeVector(((BufferObject<u8vec4> *) buffers[Mesh::COLOR])->data);
		freeVector(((BufferObject<uint32_t> *) buffers[Mesh::INDICES])->data);

		_empty = true;
	}
}

void VoxelMesh::ClearBuffers()
{
	freeVector(((BufferObject<glm::ivec3> *) buffers[Mesh::POSITION])->data);
	freeVector(((BufferObject<u8vec4> *) buffers[Mesh::COLOR])->data);
}

void VoxelMesh::UpdateMesh()
{
	if (!Empty())
	{
		GreedyMeshBuilder::GreedyBuild(this);
		UploadBuffers();
		RecalculateAABB<glm::ivec3>();
		ClearBuffers();
	}
}

const Voxel & VoxelMesh::GetVoxel(int32_t x, int32_t y, int32_t z)
{
	if (x > _size.x - 1 || x<0 || y>_size.y - 1 || y<0 || z>_size.z - 1 || z < 0)
	{
		return VoxelMesh::EMPTY_VOXEL;
	}
	else
	{
		return _vox[x + _size.x*(y + _size.y*z)];
	}
}

void VoxelMesh::CreateVox(int32_t x, int32_t y, int32_t z, const intRGBA &col)
{
	if (x > _size.x - 1 || x<0 || y>_size.y - 1 || y<0 || z>_size.z - 1 || z < 0)
	{
		//printf("overflown wtf");
		return;
	}

	if (_empty)
		_empty = false;
	_vox[x + _size.x*(y + _size.y*z)].active = true;
	_vox[x + _size.x*(y + _size.y*z)].color = col;
}

void VoxelMesh::RemoveVox(int32_t x, int32_t y, int32_t z)
{
	if (x > _size.x - 1 || x<0 || y>_size.y - 1 || y<0 || z>_size.z - 1 || z < 0)
	{
		return;
	}
	_vox[x + _size.x*(y + _size.y*z)] = EMPTY_VOXEL;
}

void VoxelMesh::SaveToFile(const std::string &filename)
{
	auto file = PHYSFS_openWrite(filename.c_str());

	const char * stuff=PHYSFS_getLastError();

	uint32_t sizerino = _vox.size();
	PHYSFS_write(file, (void*)&_size, sizeof(u16vec3), 1);
	PHYSFS_write(file, (void*)&sizerino, sizeof(sizerino), 1);
	PHYSFS_write(file, (void*)&_vox[0], sizeof(Voxel), _vox.size());

	PHYSFS_close(file);
}

VoxelMesh* VoxelMesh::LoadFromFile(const std::string &filename)
{
	auto file = PHYSFS_openRead(filename.c_str());

	u16vec3 meshSize;
	PHYSFS_read(file, &meshSize, sizeof(u16vec3), 1);

	VoxelMesh* ret = new VoxelMesh(meshSize);

	uint32_t size = 0;
	PHYSFS_read(file, &size, sizeof(uint32_t), 1);
	PHYSFS_read(file, &ret->_vox[0], sizeof(Voxel), size);

	ret->_empty = false;

	if (size > 0)
	{
		return ret;
	}
	else
	{
		delete ret;
	}
	return nullptr;
}