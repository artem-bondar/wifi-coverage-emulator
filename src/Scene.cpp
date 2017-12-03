#include "string.h"
#include "Scene.h"

using namespace glm;

CScene::CScene() : voxelSize(128)
{
	voxel = new float**[voxelSize];
	for (int x = 0; x < voxelSize; ++x) {
		voxel[x] = new float*[voxelSize];
		for (int y = 0; y < voxelSize; ++y) {
			voxel[x][y] = new float[voxelSize];
			for (int z = 0; z < voxelSize; ++z) {
				voxel[x][y][z] = 0;
			}
		}
	}
}

CScene::~CScene()
{
	for (int x = 0; x < voxelSize; ++x) {
		for (int y = 0; y < voxelSize; ++y) {
			delete[] voxel[x][y];
		}
		delete[] voxel[x];
	}
	delete[] voxel;
}

bool CScene::load(FILE* file)
{
	int res = 1;
	char lineHeader[128];

	while (res != EOF) {
		res = fscanf(file, "%s", lineHeader);
		if (strcmp(lineHeader, "v") == 0) {
			glm::vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			flat.m_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			glm::vec2 uv;
			float temp;
			fscanf(file, "%f %f %f\n", &uv.x, &uv.y, &temp);
			flat.m_textures.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			glm::vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			flat.m_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0],
				&vertexIndex[1], &uvIndex[1], &normalIndex[1],
				&vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches == 1)
			{
				matches = fscanf(file, "/%d %d//%d %d//%d\n", &normalIndex[0], &vertexIndex[1], &normalIndex[1], &vertexIndex[2], &normalIndex[2]);
				uvIndex[0] = uvIndex[1] = uvIndex[2] = 0;
				if (matches != 5) {
					printf("File can't be read by parser\n");
					return false;
				}
			}
			else if (matches != 9) {
				printf("File can't be read by parser\n");
				return false;
			}
			flat.m_triangles.push_back(glm::uvec3(vertexIndex[0] - 1, vertexIndex[1] - 1, vertexIndex[2] - 1));
			flat.m_triangles_normals.push_back(normalIndex[0] - 1);
		}
	}

	fclose(file);
	printf("Loaded vertexes: %u\nLoaded textures: %u\nLoaded normals: %u\nLoaded triangles: %u\n\n", flat.m_vertices.size(), flat.m_textures.size(), flat.m_normals.size(), flat.m_triangles.size());

	return true;
}

void CScene::preprocess()
{
	// Find bounds for voxel
	lowBound = highBound = flat.m_vertices[0];
	for (int i = 0; i < flat.m_vertices.size(); i++)
	{
		if (flat.m_vertices[i].x > highBound.x) highBound.x = flat.m_vertices[i].x;
		if (flat.m_vertices[i].y > highBound.y) highBound.y = flat.m_vertices[i].y;
		if (flat.m_vertices[i].z > highBound.z) highBound.z = flat.m_vertices[i].z;
		if (flat.m_vertices[i].x < lowBound.x) lowBound.x = flat.m_vertices[i].x;
		if (flat.m_vertices[i].y < lowBound.y) lowBound.y = flat.m_vertices[i].y;
		if (flat.m_vertices[i].z < lowBound.z) lowBound.z = flat.m_vertices[i].z;
	}

	printf("High bound: (%f, %f, %f)\nLow bound: (%f, %f, %f)\n\n", highBound.x, highBound.y, highBound.z, lowBound.x, lowBound.y, lowBound.z);

	// Delete roof
	const double zBound = highBound.z * 0.95; // 5% bound
	for (int i = 0; i < flat.m_triangles.size(); i++)
	{
		if (flat.m_normals[flat.m_triangles_normals[i]].z == 1.0 && flat.m_vertices[flat.m_triangles[i][2]].z > zBound)
		{
			printf("Deleted roof polygon: (%.0f, %.0f, %.0f) (%.0f, %.0f, %.0f) (%.0f, %.0f, %.0f)\n", flat.m_vertices[flat.m_triangles[i][0]].x, flat.m_vertices[flat.m_triangles[i][0]].y, flat.m_vertices[flat.m_triangles[i][0]].z,
				flat.m_vertices[flat.m_triangles[i][1]].x, flat.m_vertices[flat.m_triangles[i][1]].y, flat.m_vertices[flat.m_triangles[i][1]].z,
				flat.m_vertices[flat.m_triangles[i][2]].x, flat.m_vertices[flat.m_triangles[i][2]].y, flat.m_vertices[flat.m_triangles[i][2]].z);
			flat.m_triangles.erase(flat.m_triangles.begin() + i);
			flat.m_triangles_normals.erase(flat.m_triangles_normals.begin() + i);
			i -= 1;
		}
	}

	resizeRatio = highBound - lowBound;
	resizeRatio /= (voxelSize - 1);
}

void CScene::processSignalMap(glm::vec3 routerPos, float routerPower, int maxRays)
{
	for (int i = 0; i < maxRays; i++)
	{
		vec3 direction = vec3(45.0f - rand() % 91, 45.0f - rand() % 91, 45.0f - rand() % 91);
		marchRay(SRay(routerPos, normalize(direction)), routerPower);
	}
}

bool CScene::countIntersection(SRay ray, glm::uvec3 triangle, float *distance)
{
	vec3 a = flat.m_vertices[triangle[0]];
	vec3 b = flat.m_vertices[triangle[1]];
	vec3 c = flat.m_vertices[triangle[2]];
	vec3 e1 = b - a;
	vec3 e2 = c - a;
	vec3 t = ray.m_start - a;
	vec3 p = cross(ray.m_dir, e2);
	vec3 q = cross(t, e1);
	float coef = dot(e1, p);
	float u = dot(t, p) / coef;
	float v = dot(ray.m_dir, q) / coef;

	if (u < 0 || u > 1)
		return false;
	else
		if (v < 0 || u + v > 1)
			return false;
		else
		{
			*distance = dot(e2, q) / coef;
			return true;
		}
}

void CScene::marchRay(SRay ray, float routerPower)
{
	bool hit;
	float minDistance = INFINITY, distance = 0.0f, step = 25.0f, power = routerPower,
		powerCoef = 0.000001f, wallAbsorption = 50.0f, wallReflection = 8.8f;
	int polygonHit = -1;
	for (int i = 0; i < flat.m_triangles.size(); i++)
	{
		hit = countIntersection(ray, flat.m_triangles[i], &distance);
		if (hit && distance < minDistance && distance > 0)
		{
			minDistance = distance;
			polygonHit = i;
		}
	}

	vec3 line = ray.m_start;
	vec3 vecStep = ray.m_dir * step;
	if (polygonHit != -1)
	{
		for (; ray.length < minDistance && power > 0; ray.length += step, line += vecStep, power -= powerCoef * step * step)
		{
			int x = static_cast<int>(abs(lowBound.x - line.x) / resizeRatio.x);
			int y = static_cast<int>(abs(lowBound.y - line.y) / resizeRatio.y);
			int z = static_cast<int>(abs(lowBound.z - line.z) / resizeRatio.z);
			if (power > voxel[x][y][z]) voxel[x][y][z] = power;
		}
		if (power > 0)
		{
			if (power - wallAbsorption > 0)
				marchRay(SRay(line, ray.m_dir), power - wallAbsorption);
			if (power - wallReflection > 0)
				marchRay(SRay(line, reflect(ray.m_dir, flat.m_normals[flat.m_triangles_normals[polygonHit]])), power - wallReflection);
		}
	}
	else
	{
		for (; power > 0; ray.length += step, line += vecStep, power -= powerCoef * step * step)
		{
			if (line.x < lowBound.x || line.y < lowBound.y || line.z < lowBound.z ||
				line.x > highBound.x || line.y > highBound.y || line.z > highBound.z)
				break;
			int x = static_cast<int>(abs(lowBound.x - line.x) / resizeRatio.x);
			int y = static_cast<int>(abs(lowBound.y - line.y) / resizeRatio.y);
			int z = static_cast<int>(abs(lowBound.z - line.z) / resizeRatio.z);
			if (power > voxel[x][y][z]) voxel[x][y][z] = power;
		}
	}
}
