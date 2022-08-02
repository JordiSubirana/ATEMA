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

#ifndef ATEMA_SANDBOX_RESOURCES_HPP
#define ATEMA_SANDBOX_RESOURCES_HPP

#include <Atema/Atema.hpp>
#include <Atema/Math/AABB.hpp>

#include <filesystem>

const std::filesystem::path rscPath = "../../examples/Sandbox/Resources";
const auto shaderPath = rscPath / "Shaders";
const auto fontPath = rscPath / "Fonts";

const float cameraScale = 0.001f;

// LampPost
/*
const std::filesystem::path modelMeshPath = rscPath / "Models/LampPost.obj";
const std::filesystem::path modelTexturePath = rscPath / "Textures/LampPost";
const std::string modelTextureExtension = "png";
//*/

// Tardis
//*
const std::filesystem::path modelMeshPath = rscPath / "Models/tardis.obj";
const std::filesystem::path modelTexturePath = rscPath / "Textures/tardis";
const std::string modelTextureExtension = "png";
//*/

// Ground
const std::filesystem::path groundTexturePath = rscPath / "Textures/Rocks";
const std::string groundTextureExtension = "png";

inline at::Vector2f toPolar(const at::Vector2f& cartesian)
{
	const auto x = cartesian.x;
	const auto y = cartesian.y;

	at::Vector2f value;
	value.x = std::sqrt(x * x + y * y);
	value.y = 2.0f * std::atan2f(y, x + value.x);

	return value;
}

inline at::Vector2f toCartesian(const at::Vector2f& polar)
{
	const auto r = polar.x;
	const auto a = polar.y;

	at::Vector2f value;
	value.x = r * std::cos(a);
	value.y = r * std::sin(a);

	return value;
}

struct BasicVertex
{
	at::Vector3f position;
	at::Vector3f normal;
	at::Vector3f tangent;
	at::Vector3f bitangent;
	at::Vector2f texCoords;

	bool operator==(const BasicVertex& other) const
	{
		return position == other.position && normal == other.normal && tangent == other.tangent && bitangent == other.bitangent && texCoords == other.texCoords;
	}
};

ATEMA_DECLARE_STD_HASH(BasicVertex);

struct ModelData
{
	ModelData() = delete;
	ModelData(const std::filesystem::path& path);

	at::Ptr<at::Buffer> vertexBuffer;
	at::Ptr<at::Buffer> indexBuffer;
	uint32_t indexCount = 0;

	at::AABBf aabb;
};

struct MaterialData
{
	MaterialData() = delete;
	MaterialData(const std::filesystem::path& path, const std::string& format);

	at::Ptr<at::Image> color;
	at::Ptr<at::Image> normal;
	at::Ptr<at::Image> ambientOcclusion;
	at::Ptr<at::Image> height;
	at::Ptr<at::Image> emissive;
	at::Ptr<at::Image> metalness;
	at::Ptr<at::Image> roughness;

	at::Ptr<at::Sampler> sampler;

	at::Ptr<at::Buffer> uniformBuffer;

	at::Ptr<at::Shader> vertexShader;
	at::Ptr<at::Shader> fragmentShader;

	at::Ptr<at::DescriptorSetLayout> descriptorSetLayout;
	at::Ptr<at::DescriptorSet> descriptorSet;
};

#endif
