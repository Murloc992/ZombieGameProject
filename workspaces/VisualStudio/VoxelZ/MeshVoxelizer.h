#pragma once
class MeshVoxelizer
{
private:
	static glm::vec2 Project(vector<glm::vec3> points, const glm::vec3 &axis);

	template <typename T>
	static bool IsIntersecting(AABB box, Triangle<T> tri);
public:
	template<typename T>
	static void VoxelizeMesh(vector<Triangle<T> > tris, VoxelMesh* voxmesh);

	template<typename T>
	static void VoxelizeMesh(vector<Triangle<T> > tris, ChunkManager* voxmesh)
};

