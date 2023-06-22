/*
	Copyright 2022 Jordi SUBIRANA

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

#ifndef ATEMA_GRAPHICS_MESHLOADERSETTINGS_HPP
#define ATEMA_GRAPHICS_MESHLOADERSETTINGS_HPP

#include <Atema/Graphics/Config.hpp>
#include <Atema/Graphics/VertexFormat.hpp>
#include <Atema/Core/Pointer.hpp>
#include <Atema/Math/Matrix.hpp>
#include <Atema/Renderer/Enums.hpp>

#include <optional>
#include <vector>
#include <list>

namespace at
{
	class CommandBuffer;
	class Mesh;
	class Buffer;

	struct ATEMA_GRAPHICS_API ModelLoader
	{
		struct StaticVertex
		{
			Vector3f position;
			Vector2f texCoords;
			Vector3f normal;
			Vector3f tangent;
			Vector3f bitangent;

			bool operator==(const StaticVertex& other) const;
		};

		class ATEMA_GRAPHICS_API Settings
		{
		public:
			Settings() = delete;
			// Vertex format is assigned to the vertex buffer
			// Each vertex component must be at least 32-bits floating points components, except for Color and UserData
			// Each vertex component is filled as if it were an array of size N, N being VertexInput::getElementSize()
			// Position is the only required component
			// Position (XYZ if N = 3 / XYZW with W = 1 if N = 4)
			// TexCoords (UV if N = 2)
			// Normal (XYZ if N = 3 / XYZW with W = 0 if N = 4)
			// Tangent (XYZ if N = 3 / XYZW with W = 0 if N = 4)
			// Bitangent (XYZ if N = 3 / XYZW with W = 0 if N = 4)
			Settings(const VertexFormat& vertexFormat);
			Settings(const Settings& other) = default;
			Settings(Settings&& other) noexcept = default;

			Settings& operator=(const Settings& other) = default;
			Settings& operator=(Settings&& other) noexcept = default;

			const VertexFormat& getVertexFormat() const;

			// Flip vertical component of texture coordinates
			bool flipTexCoords = false;

			// Optional additional vertex buffer usages
			Flags<BufferUsage> vertexBufferUsages;

			// Optional additional index buffer usages
			Flags<BufferUsage> indexBufferUsages;

			// Optional index type
			// If unspecifided, the loader will take the smallest index size compatible with each mesh
			std::optional<IndexType> indexType;

			// Optional vertex transformation
			// If set, the loader will transform position/normal/tangent/bitangent components
			std::optional<Matrix4f> vertexTransformation;

			// Optional root for material textures
			std::filesystem::path textureDir;

			// Optional command buffer
			// If valid:
			//	- it will be used if there are staging buffers to copy to final buffers (ie vertex buffer or index buffer is not mappable)
			//	- CommandBuffer::begin() & CommandBuffer::end() must be called by the user before and after the loading
			// If not, a temporary command buffer will be created by the loader and models will be valid just after the loading
			// See 'stagingBuffers' member for more details
			Ptr<CommandBuffer> commandBuffer;

			// Optional list containing all the staging buffers used by the loader
			// This is an output parameter, storing all the staging buffers created to load the model, if any
			// The user must keep them around until 'commandBuffer' is executed
			// If 'commandBuffer' is valid, 'stagingBuffers' must be valid
			// See 'commandBuffer' member for more details
			std::list<Ptr<Buffer>>* stagingBuffers;

		private:
			void ensureVertexFormatValid() const;

			VertexFormat m_vertexFormat;
		};

		// Generate tangents and bitangeants
		// As they are coherent inside each triangle, the vertex count may be different
		// Generated vertices are unique (no need to remove duplicates)
		// Indices are considered to define triangles
		static void generateTangents(std::vector<StaticVertex>& vertices, std::vector<uint32_t>& indices);

		// Remove duplicate vertices and compute the new indices
		static void removeDuplicates(std::vector<StaticVertex>& vertices, std::vector<uint32_t>& indices);

		// Process vertices & indices if required by the settings, then creates a mesh
		static Ptr<Mesh> loadMesh(std::vector<StaticVertex>& vertices, std::vector<uint32_t>& indices, const Settings& settings);
	};
}

#endif
