/*
	Copyright 2021 Jordi SUBIRANA

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

#ifndef ATEMA_SANDBOX_RESOURCES_HPP
#define ATEMA_SANDBOX_RESOURCES_HPP

#include <Atema/Atema.hpp>

struct BasicVertex
{
	at::Vector3f pos;
	at::Vector3f color;
	at::Vector2f texCoord;

	bool operator==(const BasicVertex& other) const
	{
		return pos == other.pos && color == other.color && texCoord == other.texCoord;
	}
};

namespace std
{
	template<> struct hash<at::Vector3f>
	{
		size_t operator()(at::Vector3f const& vertex) const noexcept
		{
			size_t h1 = std::hash<double>()(vertex.x);
			size_t h2 = std::hash<double>()(vertex.y);
			size_t h3 = std::hash<double>()(vertex.z);
			return (h1 ^ (h2 << 1)) ^ h3;
		}
	};

	template<> struct hash<at::Vector2f>
	{
		size_t operator()(at::Vector2f const& vertex) const noexcept
		{
			return std::hash<at::Vector3f>()({ vertex.x, vertex.y, 0.0f });
		}
	};

	template<> struct hash<BasicVertex>
	{
		size_t operator()(BasicVertex const& vertex) const noexcept
		{
			return ((hash<at::Vector3f>()(vertex.pos) ^
				(hash<at::Vector3f>()(vertex.color) << 1)) >> 1) ^
				(hash<at::Vector2f>()(vertex.texCoord) << 1);
		}
	};
}

struct ModelData
{
	ModelData() = delete;
	ModelData(const std::filesystem::path& path, at::Ptr<at::CommandPool> commandPool);

	at::Ptr<at::Buffer> vertexBuffer;
	at::Ptr<at::Buffer> indexBuffer;
	uint32_t indexCount = 0;
};

struct MaterialData
{
	MaterialData() = delete;
	MaterialData(const std::filesystem::path& path, at::Ptr<at::CommandPool> commandPool);

	at::Ptr<at::Image> texture;
	at::Ptr<at::Sampler> sampler;
};

#endif
