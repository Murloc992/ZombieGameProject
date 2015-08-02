#pragma once

class MeshVoxelizer
{
private:
	static glm::vec2 Project(vector<glm::vec3> points, const glm::vec3 &axis);

	template <typename T>
	static bool IsIntersecting(AABB box, Triangle<T> tri);
public:
	template <typename T>
	static void VoxelizeMesh(vector<Triangle<T> > tris, VoxelMesh* voxmesh);
	template <typename T>
	static void VoxelizeMesh(vector<Triangle<T> > tris, ChunkManager* voxmesh);
};

template<typename T>
bool MeshVoxelizer::IsIntersecting(AABB box, Triangle<T> tri)
{
	//test aabb normals
	glm::vec3 boxNormals[] = { glm::vec3(1.f,0.f,0.f),glm::vec3(0.f,1.f,0.f),glm::vec3(0.f,0.f,1.f) };
	loop(i, 3)
	{
		glm::vec3 n = boxNormals[i];
		glm::vec2 proj = Project(tri.points, n);
		if (proj[1] < box.GetMin()[i] || proj[0] > box.GetMax()[i])
			return false; // no intersection possible
	}

	//test triangle normal
	float triOffset = glm::dot(tri.normal, tri.points[0]);
	glm::vec2 proj = Project(box.GetPoints(), tri.normal);
	if (proj[1] < triOffset || proj[0] > triOffset)
		return false; //no intersection possible

					  // test nine edge cross products
	T triEdges[] = { tri.points[0] - tri.points[1],tri.points[1] - tri.points[2],tri.points[2] - tri.points[0] };
	loop(i, 3)
		loop(j, 3)
	{
		glm::vec3 axis = glm::cross(triEdges[i], boxNormals[j]);
		glm::vec2 boxProj = Project(box.GetPoints(), axis);
		glm::vec2 triProj = Project(tri.points, axis);
		if (boxProj[1] < triProj[0] || boxProj[0] > triProj[1])
			return false;
	}

	return true;
}

template<typename T>
void MeshVoxelizer::VoxelizeMesh(vector<Triangle<T> > tris, VoxelMesh* voxmesh)
{
	uint32_t trisize = tris.size();
	loop(i, tris.size())
	{
		//printf("Processing tri %u of %u...\n",i,trisize);

		AABB a;
		a.Reset(tris[i].points[0]);
		a.AddPoint(tris[i].points[1]);
		a.AddPoint(tris[i].points[2]);
		//boxes.push_back(new CubeMesh(a));
		//printf("Tri Points:\n%s\n%s\n%s\n",GLMVec3ToStr(tris[i].points[0]),GLMVec3ToStr(tris[i].points[1]),GLMVec3ToStr(tris[i].points[2]));

		//printf("AABB\nmin %s\nmax %s\ncenter %s\nhalfsize %s\n",GLMVec3ToStr(a.GetMin()),GLMVec3ToStr(a.GetMax()),GLMVec3ToStr(a.GetCenter()),GLMVec3ToStr(a.GetHalfSize()));
		glm::ivec3 amn = (glm::ivec3)glm::round((a.GetMin()));
		glm::ivec3 amx = (glm::ivec3)glm::round((a.GetMax()));
		//printf("Ranges %s %s\n",GLMVec3ToStr(amn),GLMVec3ToStr(amx));

		int32_t sx, ex, sy, ey, sz, ez;

		sx = amn.x;
		sy = amn.y;
		sz = amn.z;

		ex = amx.x;
		ey = amx.y;
		ez = amx.z;

		if (ex < sx)
		{
			int32_t tmp = sx;
			sx = ex;
			ex = tmp;
		}

		if (ey < sy)
		{
			int32_t tmp = sy;
			sy = ey;
			ey = tmp;
		}

		if (ez < sz)
		{
			int32_t tmp = sz;
			sz = ez;
			ez = tmp;
		}

		for (int32_t x = sx - 1; x <= ex + 1; x++)
		{
			for (int32_t y = sy - 1; y <= ey + 1; y++)
			{
				for (int32_t z = sz - 1; z <= ez + 1; z++)
				{
					if (voxmesh->GetVoxel(x, y, z).active == false)
					{
						AABB voxaabb(glm::vec3(x, y, z) + glm::vec3(0.5f), glm::vec3(0.5f));
						voxaabb.CalculatePoints();
						if (IsIntersecting<glm::vec3>(voxaabb, tris[i]))
							voxmesh->CreateVox(x, y, z, VecRGBToIntRGB(u8vec3(255)));
						else
							continue;
					}
					else
						continue;
				}
			}
		}
	}
}

template<typename T>
void MeshVoxelizer::VoxelizeMesh(vector<Triangle<T> > tris, ChunkManager* voxmesh)
{
	uint32_t trisize = tris.size();
	loop(i, tris.size())
	{
		//printf("Processing tri %u of %u...\n",i,trisize);

		AABB a;
		a.Reset(tris[i].points[0]);
		a.AddPoint(tris[i].points[1]);
		a.AddPoint(tris[i].points[2]);
		//boxes.push_back(new CubeMesh(a));
		//printf("Tri Points:\n%s\n%s\n%s\n",GLMVec3ToStr(tris[i].points[0]),GLMVec3ToStr(tris[i].points[1]),GLMVec3ToStr(tris[i].points[2]));

		//printf("AABB\nmin %s\nmax %s\ncenter %s\nhalfsize %s\n",GLMVec3ToStr(a.GetMin()),GLMVec3ToStr(a.GetMax()),GLMVec3ToStr(a.GetCenter()),GLMVec3ToStr(a.GetHalfSize()));
		glm::ivec3 amn = (glm::ivec3)glm::round((a.GetMin()));
		glm::ivec3 amx = (glm::ivec3)glm::ceil((a.GetMax()));
		//printf("Ranges %s %s\n",GLMVec3ToStr(amn),GLMVec3ToStr(amx));

		int32_t sx, ex, sy, ey, sz, ez;

		sx = amn.x;
		sy = amn.y;
		sz = amn.z;

		ex = amx.x;
		ey = amx.y;
		ez = amx.z;

		if (ex < sx)
		{
			int32_t tmp = sx;
			sx = ex;
			ex = tmp;
		}

		if (ey < sy)
		{
			int32_t tmp = sy;
			sy = ey;
			ey = tmp;
		}

		if (ez < sz)
		{
			int32_t tmp = sz;
			sz = ez;
			ez = tmp;
		}

		for (int32_t x = sx - 1; x <= ex + 1; x++)
		{
			for (int32_t y = sy - 1; y <= ey + 1; y++)
			{
				for (int32_t z = sz - 1; z <= ez + 1; z++)
				{
					if (voxmesh->GetBlock(glm::ivec3(x, y, z)) == Chunk::EMPTY_BLOCK)
					{
						//printf("Definitely empty\n");
						AABB voxaabb(glm::vec3(x, y, z) + glm::vec3(0.5f), glm::vec3(0.5f));
						voxaabb.CalculatePoints();
						if (IsIntersecting<glm::vec3>(voxaabb, tris[i]))
							voxmesh->SetBlock(glm::ivec3(x, y, z), EBT_STONE, true);
						else
							continue;
					}
					else
						continue;
				}
			}
		}
	}
}