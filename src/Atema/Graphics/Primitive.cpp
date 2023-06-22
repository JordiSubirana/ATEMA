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
