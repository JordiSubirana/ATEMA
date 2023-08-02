/*
	Copyright 2023 Jordi SUBIRANA

	Permission is hereby granted, free of charge, to any person obtaining a copy of
	this software and associated documentation files (the "Software"), to deal in
	the Software without restriction, including without limitation the rights to use,
	copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
	Software, and to permit persons to whom the Software is furnished to do so, subject
	to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
	INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
	PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
	HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
	CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
	OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <Atema/Graphics/Primitive.hpp>
#include <Atema/Graphics/IndexBuffer.hpp>
#include <Atema/Graphics/Loaders/ModelLoader.hpp>

using namespace at;

namespace
{
	Ptr<Mesh> createCone(const ModelLoader::Settings& settings, const Vector3f& direction, float range, float radius, float angle, size_t verticalSubdivisions, size_t horizontalSubdivisions)
	{
		ATEMA_ASSERT(verticalSubdivisions >= 3, "Cone vertical subdivisions must be at least 3");
		ATEMA_ASSERT(horizontalSubdivisions >= 1, "Cone vertical subdivisions must be at least 1");

		if (verticalSubdivisions < 3)
			verticalSubdivisions = 3;

		if (horizontalSubdivisions < 1)
			horizontalSubdivisions = 1;

		const size_t quadSubdivisions = (horizontalSubdivisions > 1) ? horizontalSubdivisions - 1 : 0;

		const size_t baseVertexCount = verticalSubdivisions;

		// (verticalSubdivisions + 1) * (horizontalSubdivisions + top + base) + baseCenter;
		// verticalSubdivisions + 1 to have complete UVs
		const size_t vertexCount = (verticalSubdivisions + 1) * (horizontalSubdivisions + 2) + 1;
		// verticalSubdivisions * (quadSubdivisions * 2 triangles + top triangle + base triangle);
		const size_t indexCount = verticalSubdivisions * (quadSubdivisions * 6 + 3 + 3);

		std::vector<ModelLoader::StaticVertex> vertices;
		std::vector<uint32_t> indices;

		vertices.reserve(vertexCount);
		indices.reserve(indexCount);

		const Vector3f axisZ = direction.getNormalized();

		Vector3f axisX(-axisZ.y, axisZ.x, axisZ.z);

		if (Math::equals(axisZ.x, 0.0f) && Math::equals(axisZ.y, 0.0f))
			axisX = Vector3f(axisZ.z, 0.0f, 0.0f);

		const Vector3f axisY = cross(axisZ, axisX);

		Matrix3f basisChangeMat;
		basisChangeMat[0] = axisX;
		basisChangeMat[1] = axisY;
		basisChangeMat[2] = axisZ;

		const float oneOverHorizontalSubdivisions = 1.0f / static_cast<float>(horizontalSubdivisions);
		const float rangeStep = range * oneOverHorizontalSubdivisions;

		std::vector<float> ranges;
		std::vector<float> radii;
		ranges.reserve(horizontalSubdivisions);
		radii.reserve(horizontalSubdivisions);
		for (size_t h = 1; h <= horizontalSubdivisions; h++)
		{
			ranges.emplace_back(static_cast<float>(h) * rangeStep);
			radii.emplace_back(radius * ranges.back() / range);
		}

		const Vector3f topPosition(0.0f, 0.0f, 0.0f);
		const Vector3f horizontalVectorOffset = axisZ * rangeStep;
		const Vector3f baseCenter = topPosition + axisZ * range;

		const float oneOverSubdivisions = 1.0f / static_cast<float>(verticalSubdivisions);

		const float angleStep = 2.0f * Math::Pi<float> * oneOverSubdivisions;
		const float halfAngleStep = angleStep / 2.0f;

		const float coneAngleCos = std::cos(angle);
		const float coneAngleSin = std::sin(angle);

		for (size_t v = 0; v <= verticalSubdivisions; v++)
		{
			const float subdivisionAngle = static_cast<float>(v) * angleStep;
			const float subdivisionAngleCos = std::cos(subdivisionAngle);
			const float subdivisionAngleSin = std::sin(subdivisionAngle);

			// Top vertex
			{
				auto& vertex = vertices.emplace_back();

				vertex.position = topPosition;

				vertex.normal.x = std::cos(subdivisionAngle + halfAngleStep);
				vertex.normal.y = std::sin(subdivisionAngle + halfAngleStep);
				vertex.normal.z = coneAngleSin;
				
				vertex.bitangent = -vertex.normal;
				vertex.bitangent.z = coneAngleCos;

				vertex.normal.normalize();
				vertex.normal = basisChangeMat * vertex.normal;

				vertex.bitangent.normalize();
				vertex.bitangent = basisChangeMat * vertex.bitangent;

				vertex.tangent = cross(vertex.bitangent, vertex.normal);

				vertex.texCoords.x = 1.0f - oneOverSubdivisions * (static_cast<float>(v) + 0.5f);
			}

			const Vector3f normal = basisChangeMat * Vector3f(subdivisionAngleCos, subdivisionAngleSin, coneAngleSin).getNormalized();
			const Vector3f bitangent = basisChangeMat * Vector3f(-subdivisionAngleCos, -subdivisionAngleSin, coneAngleCos).getNormalized();
			const Vector3f tangent = cross(bitangent, normal);

			// Side vertices
			for (size_t h = 0; h < horizontalSubdivisions; h++)
			{
				auto& vertex = vertices.emplace_back();

				vertex.position.x = radii[h] * subdivisionAngleCos;
				vertex.position.y = radii[h] * subdivisionAngleSin;
				vertex.position.z = ranges[h];

				vertex.position = basisChangeMat * vertex.position;

				vertex.normal = normal;
				vertex.tangent = tangent;
				vertex.bitangent = bitangent;

				vertex.texCoords.x = 1.0f - oneOverSubdivisions * static_cast<float>(v);
				vertex.texCoords.y = oneOverHorizontalSubdivisions * static_cast<float>(h + 1);
			}

			// Base vertex (with base normal)
			{
				auto& vertex = vertices.emplace_back();

				vertex.position = vertices[vertices.size() - 2].position;

				vertex.normal = axisZ;
				vertex.tangent = axisX;
				vertex.bitangent = axisY;

				vertex.texCoords.x = 1.0f - (subdivisionAngleCos + 1.0f) / 2.0f;
				vertex.texCoords.y = (subdivisionAngleSin + 1.0f) / 2.0f;
			}
		}

		// Base center vertex
		{
			auto& vertex = vertices.emplace_back();

			vertex.position = baseCenter;

			vertex.normal = axisZ;
			vertex.tangent = axisX;
			vertex.bitangent = axisY;

			vertex.texCoords.x = 0.5f;
			vertex.texCoords.y = 0.5f;
		}

		// horizontalSubdivisions + top + base
		const uint32_t verticesPerSubdivision = static_cast<uint32_t>(horizontalSubdivisions) + 2;
		const uint32_t subdivisionMod = static_cast<uint32_t>((verticalSubdivisions + 1) * verticesPerSubdivision);
		const uint32_t baseCenterIndex = static_cast<uint32_t>(vertices.size() - 1);

		for (uint32_t v = 0; v < static_cast<uint32_t>(verticalSubdivisions); v++)
		{
			const uint32_t startIndex = verticesPerSubdivision * v;

			// Top triangle
			indices.emplace_back(startIndex);
			indices.emplace_back((startIndex + verticesPerSubdivision + 1) % subdivisionMod);
			indices.emplace_back(startIndex + 1);

			// Quads
			for (uint32_t h = 1; h <= static_cast<uint32_t>(quadSubdivisions); h++)
			{
				indices.emplace_back(startIndex + h + 1);
				indices.emplace_back(startIndex + h);
				indices.emplace_back((startIndex + verticesPerSubdivision + h) % subdivisionMod);

				indices.emplace_back(startIndex + h + 1);
				indices.emplace_back((startIndex + verticesPerSubdivision + h) % subdivisionMod);
				indices.emplace_back((startIndex + verticesPerSubdivision + h + 1) % subdivisionMod);
			}

			// Base triangle
			indices.emplace_back(baseCenterIndex);
			indices.emplace_back(startIndex + verticesPerSubdivision - 1);
			indices.emplace_back((startIndex + 2 * verticesPerSubdivision -1) % subdivisionMod);
		}

		return ModelLoader::loadMesh(vertices, indices, settings);
	}
}

Ptr<Mesh> Primitive::createConeFromRadius(const ModelLoader::Settings& settings, const Vector3f& direction, float range, float radius, size_t verticalSubdivisions, size_t horizontalSubdivisions)
{
	const float angle = std::atan(radius / range);

	return createCone(settings, direction, range, radius, angle, verticalSubdivisions, horizontalSubdivisions);
}

Ptr<Mesh> Primitive::createConeFromAngle(const ModelLoader::Settings& settings, const Vector3f& direction, float range, float angle, size_t verticalSubdivisions, size_t horizontalSubdivisions)
{
	const float radius = range * std::tan(angle / 2.0f);

	return createCone(settings, direction, range, radius, angle, verticalSubdivisions, horizontalSubdivisions);
}

Ptr<Mesh> Primitive::createUVSphere(const ModelLoader::Settings& settings, float radius, size_t verticalSliceCount, size_t horizontalSliceCount)
{
	// Adapted from http://www.songho.ca/opengl/gl_sphere.html

	const size_t vertexCount = (verticalSliceCount + 1) * (horizontalSliceCount + 1);
	const size_t indexCount = verticalSliceCount * (horizontalSliceCount - 1) * 6;

	std::vector<ModelLoader::StaticVertex> vertices;
	std::vector<uint32_t> indices;

	vertices.reserve(vertexCount);
	indices.reserve(indexCount);

	// Generate vertices
	const float inverseRadius = 1.0f / radius;
	const float verticalStep = 2.0f * Math::Pi<float> / static_cast<float>(verticalSliceCount);
	const float horizontalStep = Math::Pi<float> / static_cast<float>(horizontalSliceCount);

	for (size_t h = 0; h <= horizontalSliceCount; h++)
	{
		// Polar angle from Pi/2 to -Pi/2
		const float polarAngle = Math::Pi<float> / 2.0f - static_cast<float>(h) * horizontalStep;
		const float cp = std::cos(polarAngle);
		const float sp = std::sin(polarAngle);

		const float rcp = radius * cp;
		const float rsp = radius * sp;

		// First and last vertices have same position and normal, but different tex coords
		for (size_t v = 0; v <= verticalSliceCount; v++)
		{
			auto& vertex = vertices.emplace_back();

			// Azimuth from 0 to 2Pi
			const float azimuth = static_cast<float>(v) * verticalStep;
			const float ca = std::cos(azimuth);
			const float sa = std::sin(azimuth);

			vertex.position.x = rcp * ca;
			vertex.position.y = rcp * sa;
			vertex.position.z = rsp;

			vertex.normal = vertex.position / inverseRadius;

			vertex.tangent.x = -sa;
			vertex.tangent.y = ca;
			vertex.tangent.z = 0;

			vertex.bitangent = cross(vertex.normal, vertex.tangent);

			vertex.texCoords.x = static_cast<float>(v) / static_cast<float>(verticalSliceCount);
			vertex.texCoords.y = static_cast<float>(h) / static_cast<float>(horizontalSliceCount);
		}
	}

	// Generate indices (2 triangles per sector excluding first and last horizontal slices)
	for (uint32_t h = 0; h < horizontalSliceCount; h++)
	{
		uint32_t k1 = h * static_cast<uint32_t>(verticalSliceCount + 1);
		uint32_t k2 = k1 + static_cast<uint32_t>(verticalSliceCount) + 1;

		for (uint32_t v = 0; v < verticalSliceCount; v++, k1++, k2++)
		{
			if (h != 0)
			{
				indices.emplace_back(k1);
				indices.emplace_back(k2);
				indices.emplace_back(k1 + 1);
			}

			if (h != (horizontalSliceCount - 1))
			{
				indices.emplace_back(k1 + 1);
				indices.emplace_back(k2);
				indices.emplace_back(k2 + 1);
			}
		}
	}

	return ModelLoader::loadMesh(vertices, indices, settings);
}
