#include "Precomp.h"
#include "utility\Vector.h"
#include "opengl\AABB.h"
#include "opengl\Mesh.h"
#include "ChunkManager.h"
#include "VoxelMesh.h"
#include "MeshVoxelizer.h"

glm::vec2 MeshVoxelizer::Project(vector<glm::vec3> points, const glm::vec3 &axis)
{
	float min = 999999.f;
	float max = -999999.f;

	for (uint32_t i = 0; i < points.size(); i++)
	{
		float val = glm::dot(points[i], axis);
		if (val < min) min = val;
		if (val > max) max = val;
	}

	return glm::vec2(min, max);
}