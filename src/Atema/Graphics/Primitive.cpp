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

		// Build an orthogonal basis from cone direction
		const Vector3f axisZ = direction.getNormalized();

		const float zDotX = axisZ.dot(Vector3f(1.0f, 0.0f, 0.0f));
		const float dotEpsilon = 0.001f;

		Vector3f axisX;
		if (!Math::equals(zDotX, 1.0f, dotEpsilon) && !Math::equals(zDotX, -1.0f, dotEpsilon))
			axisX = cross(axisZ, Vector3f(1.0f, 0.0f, 0.0f)).getNormalized();
		else
			axisX = cross(axisZ, Vector3f(0.0f, 1.0f, 0.0f)).getNormalized();

		const Vector3f axisY = cross(axisZ, axisX).getNormalized();

		Matrix3f basisChangeMat;
		basisChangeMat[0] = axisX;
		basisChangeMat[1] = axisY;
		basisChangeMat[2] = axisZ;

		// Calculate range & radius for each horizontal subdivision
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

		const float oneOverVerticalSubdivisions = 1.0f / static_cast<float>(verticalSubdivisions);

		const float angleStep = 2.0f * Math::Pi<float> * oneOverVerticalSubdivisions;
		const float halfAngleStep = angleStep / 2.0f;

		const float coneCos = std::cos(angle / 2.0f);
		const float coneSin = std::sin(angle / 2.0f);

		for (size_t v = 0; v <= verticalSubdivisions; v++)
		{
			const float subdivisionAngle = static_cast<float>(v) * angleStep;
			const float subdivisionCos = std::cos(subdivisionAngle);
			const float subdivisionSin = std::sin(subdivisionAngle);

			// Top vertex
			{
				auto& vertex = vertices.emplace_back();

				vertex.position = topPosition;

				vertex.normal.x = coneCos * std::cos(subdivisionAngle + halfAngleStep);
				vertex.normal.y = coneCos * std::sin(subdivisionAngle + halfAngleStep);
				vertex.normal.z = -coneSin;

				vertex.normal = basisChangeMat * vertex.normal;
				vertex.normal.normalize();
				
				vertex.bitangent.x = -coneSin * std::cos(subdivisionAngle + halfAngleStep);
				vertex.bitangent.y = -coneSin * std::cos(subdivisionAngle + halfAngleStep);
				vertex.bitangent.z = -coneCos;

				vertex.bitangent = basisChangeMat * vertex.bitangent;
				vertex.bitangent.normalize();

				vertex.tangent = cross(vertex.bitangent, vertex.normal);
				vertex.tangent.normalize();

				vertex.texCoords.x = 1.0f - oneOverVerticalSubdivisions * (static_cast<float>(v) + 0.5f);
			}

			Vector3f normal = basisChangeMat * Vector3f(coneCos * subdivisionCos, coneCos * subdivisionSin, -coneSin);
			normal.normalize();

			Vector3f bitangent = basisChangeMat * Vector3f(-coneSin * subdivisionCos, -coneSin * subdivisionSin, -coneCos);
			bitangent.normalize();

			Vector3f tangent = cross(bitangent, normal);
			tangent.normalize();

			Vector3f vertexDir = basisChangeMat * Vector3f(subdivisionCos, subdivisionSin, 0.0f);
			vertexDir.normalize();

			// Side vertices
			for (size_t h = 0; h < horizontalSubdivisions; h++)
			{
				auto& vertex = vertices.emplace_back();

				vertex.position = axisZ * ranges[h] + vertexDir * radii[h];

				vertex.normal = normal;
				vertex.tangent = tangent;
				vertex.bitangent = bitangent;

				vertex.texCoords.x = 1.0f - oneOverVerticalSubdivisions * static_cast<float>(v);
				vertex.texCoords.y = oneOverHorizontalSubdivisions * static_cast<float>(h + 1);
			}

			// Base vertex (with base normal)
			{
				auto& vertex = vertices.emplace_back();

				vertex.position = vertices[vertices.size() - 2].position;

				vertex.normal = axisZ;
				vertex.tangent = axisX;
				vertex.bitangent = axisY;

				vertex.texCoords.x = 1.0f - (subdivisionCos + 1.0f) / 2.0f;
				vertex.texCoords.y = (subdivisionSin + 1.0f) / 2.0f;
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

Ptr<Mesh> Primitive::createBox(const ModelLoader::Settings& settings, float width, float height, float depth, size_t widthSubdivisions, size_t heightSubdivisions, size_t depthSubdivisions)
{
	ATEMA_ASSERT(widthSubdivisions > 0, "widthSubdivisions must be at least 1");
	ATEMA_ASSERT(heightSubdivisions > 0, "heightSubdivisions must be at least 1");
	ATEMA_ASSERT(depthSubdivisions > 0, "depthSubdivisions must be at least 1");

	const Vector3f size(width, depth, height);
	const Vector3<size_t> faceSubs(widthSubdivisions, depthSubdivisions, heightSubdivisions);
	const Vector3<size_t> vertexSubs = faceSubs + Vector3<size_t>(1, 1, 1);
	const Vector3f step(size.x / static_cast<float>(faceSubs.x), size.y / static_cast<float>(faceSubs.y), size.z / static_cast<float>(faceSubs.z));
	const Vector3f halfSize = size / 2.0f;

	// Subdivisions along tangent / bitangent axes
	const std::array<Vector2<size_t>, 6> subdivisions =
	{
		Vector2<size_t>(faceSubs.x, faceSubs.z), // Front
		Vector2<size_t>(faceSubs.x, faceSubs.z), // Back
		Vector2<size_t>(faceSubs.y, faceSubs.z), // Right
		Vector2<size_t>(faceSubs.y, faceSubs.z), // Left
		Vector2<size_t>(faceSubs.x, faceSubs.y), // Top
		Vector2<size_t>(faceSubs.x, faceSubs.y)  // Bottom
	};

	const std::array<Vector3f, 6> normals =
	{
		Vector3f(0.0f, -1.0f, 0.0f),// Front
		Vector3f(0.0f, 1.0f, 0.0f), // Back
		Vector3f(1.0f, 0.0f, 0.0f), // Right
		Vector3f(-1.0f, 0.0f, 0.0f),// Left
		Vector3f(0.0f, 0.0f, 1.0f), // Top
		Vector3f(0.0f, 0.0f, -1.0f) // Bottom
	};

	const std::array<Vector3f, 6> tangents =
	{
		Vector3f(1.0f, 0.0f, 0.0f), // Front
		Vector3f(-1.0f, 0.0f, 0.0f),// Back
		Vector3f(0.0f, 1.0f, 0.0f), // Right
		Vector3f(0.0f, -1.0f, 0.0f),// Left
		Vector3f(1.0f, 0.0f, 0.0f), // Top
		Vector3f(-1.0f, 0.0f, 0.0f) // Bottom
	};

	const std::array<Vector3f, 6> bitangents =
	{
		Vector3f(0.0f, 0.0f, 1.0f),// Front
		Vector3f(0.0f, 0.0f, 1.0f),// Back
		Vector3f(0.0f, 0.0f, 1.0f),// Right
		Vector3f(0.0f, 0.0f, 1.0f),// Left
		Vector3f(0.0f, 1.0f, 0.0f),// Top
		Vector3f(0.0f, 1.0f, 0.0f) // Bottom
	};

	const size_t vertexCount = (vertexSubs.x * (vertexSubs.y + vertexSubs.z) + vertexSubs.y * vertexSubs.z) * 2;
	const size_t indexCount = (faceSubs.x * (faceSubs.y + faceSubs.z) + faceSubs.y * faceSubs.z) * 12;

	std::vector<ModelLoader::StaticVertex> vertices;
	std::vector<uint32_t> indices;

	vertices.reserve(vertexCount);
	indices.reserve(indexCount);

	// Vertices
	for (size_t i = 0; i < 6; i++)
	{
		const Vector3f& normal = normals[i];
		const Vector3f& tangent = tangents[i];
		const Vector3f& bitangent = bitangents[i];
		const Vector2<size_t>& subdivision = subdivisions[i];

		const Vector3f origin = (normal - tangent - bitangent) * halfSize;

		for (size_t t = 0; t <= subdivision.x; t++)
		{
			const Vector3f tangentOffset = tangent * step * static_cast<float>(t);
			const float uvx = tangentOffset.getNorm();

			for (size_t b = 0; b <= subdivision.y; b++)
			{
				const Vector3f bitangentOffset = bitangent * step * static_cast<float>(b);

				auto& vertex = vertices.emplace_back();

				vertex.position = origin + tangentOffset + bitangentOffset;

				vertex.normal = normal;
				vertex.tangent = tangent;
				vertex.bitangent = bitangent;

				vertex.texCoords.x = uvx;
				vertex.texCoords.y = -bitangentOffset.getNorm();
			}
		}
	}

	// Indices
	uint32_t faceOffset = 0;
	for (uint32_t i = 0; i < 6; i++)
	{
		Vector2<uint32_t> subdivision;
		subdivision.x = static_cast<uint32_t>(subdivisions[i].x);
		subdivision.y = static_cast<uint32_t>(subdivisions[i].y);

		for (uint32_t t = 0; t < subdivision.x; t++)
		{
			uint32_t line = faceOffset + t * (subdivision.y + 1);
			uint32_t nextLine = line + (subdivision.y + 1);

			for (uint32_t b = 0; b < subdivision.y; b++)
			{
				indices.emplace_back(line + b + 1);
				indices.emplace_back(line + b);
				indices.emplace_back(nextLine + b + 1);

				indices.emplace_back(nextLine + b + 1);
				indices.emplace_back(line + b);
				indices.emplace_back(nextLine + b);
			}
		}

		faceOffset += (subdivision.x + 1) * (subdivision.y + 1);
	}

	return ModelLoader::loadMesh(vertices, indices, settings);
}

Ptr<Mesh> Primitive::createConeFromRadius(const ModelLoader::Settings& settings, const Vector3f& direction, float range, float radius, size_t verticalSubdivisions, size_t horizontalSubdivisions)
{
	const float angle = std::atan(radius / range) * 2.0f;

	return createCone(settings, direction, range, radius, angle, verticalSubdivisions, horizontalSubdivisions);
}

Ptr<Mesh> Primitive::createConeFromAngle(const ModelLoader::Settings& settings, const Vector3f& direction, float range, float angle, size_t verticalSubdivisions, size_t horizontalSubdivisions)
{
	const float radius = range * std::tan(angle / 2.0f);

	return createCone(settings, direction, range, radius, angle, verticalSubdivisions, horizontalSubdivisions);
}

Ptr<Mesh> Primitive::createPlane(const ModelLoader::Settings& settings, const Vector3f& normal, float width, float height, size_t widthSubdivisions, size_t heightSubdivisions)
{
	ATEMA_ASSERT(widthSubdivisions > 0, "widthSubdivisions must be at least 1");
	ATEMA_ASSERT(heightSubdivisions > 0, "heightSubdivisions must be at least 1");

	const size_t vertexCount = (widthSubdivisions + 1) * (heightSubdivisions + 1);
	const size_t indexCount = widthSubdivisions * heightSubdivisions * 6;

	const Quaternionf quaternion(Vector3f(0.0f, 0.0f, 1.0f), normal);

	const Vector3f axisX = quaternion.rotateVector(Vector3f(1.0f, 0.0f, 0.0f)).getNormalized();
	const Vector3f axisY = quaternion.rotateVector(Vector3f(0.0f, 1.0f, 0.0f)).getNormalized();
	const Vector3f axisZ = normal.getNormalized();

	const Vector2f size(width, height);
	const Vector2f halfSize = size / 2.0f;
	const Vector2f step = size / Vector2f(static_cast<float>(widthSubdivisions), static_cast<float>(heightSubdivisions));

	const Vector3f origin = -axisX * halfSize.x - axisY * halfSize.y;

	std::vector<ModelLoader::StaticVertex> vertices;
	std::vector<uint32_t> indices;

	vertices.reserve(vertexCount);
	indices.reserve(indexCount);

	for (size_t x = 0; x <= widthSubdivisions; x++)
	{
		const float uvx = step.x * static_cast<float>(x);
		const Vector3f offsetX = axisX * uvx;

		for (size_t y = 0; y <= heightSubdivisions; y++)
		{
			const float uvy = step.y * static_cast<float>(y);
			const Vector3f offsetY = axisY * uvy;

			auto& vertex = vertices.emplace_back();

			vertex.position = origin + offsetX + offsetY;

			vertex.normal = axisZ;
			vertex.tangent = axisX;
			vertex.bitangent = axisY;

			vertex.texCoords.x = uvx;
			vertex.texCoords.y = uvy;
		}
	}

	for (uint32_t x = 0; x < widthSubdivisions; x++)
	{
		const uint32_t line = x * static_cast<uint32_t>(heightSubdivisions + 1);
		const uint32_t nextLine = (x + 1) * static_cast<uint32_t>(heightSubdivisions + 1);

		for (uint32_t y = 0; y < heightSubdivisions; y++)
		{
			indices.emplace_back(line + y);
			indices.emplace_back(nextLine + y + 1);
			indices.emplace_back(line + y + 1);

			indices.emplace_back(nextLine + y + 1);
			indices.emplace_back(line + y);
			indices.emplace_back(nextLine + y);
		}
	}

	return ModelLoader::loadMesh(vertices, indices, settings);
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
