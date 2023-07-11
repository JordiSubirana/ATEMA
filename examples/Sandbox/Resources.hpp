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
#include <Atema/Graphics/Model.hpp>

#include <filesystem>

const std::filesystem::path rscPath = "../../examples/Sandbox/Resources";
const auto shaderPath = rscPath / "Shaders";
const auto fontPath = rscPath / "Fonts";

const float cameraScale = 0.001f;

// LampPost
/*
const bool overrideMaterial = false;
const std::filesystem::path modelMeshPath = rscPath / "Models/LampPost.obj";
const std::filesystem::path modelTexturePath = rscPath / "Textures/LampPost";
const std::string modelTextureExtension = "png";
const at::Transform modelTransform({ 0.0f, 0.0f, -2.0f }, { at::Math::HalfPi<float>, 0.0f, 0.0f }, { 0.1f, 0.1f, 0.1f });
//*/

// Tardis
/*
const bool overrideMaterial = true;
const std::filesystem::path modelMeshPath = rscPath / "Models/tardis.obj";
const std::filesystem::path modelTexturePath = rscPath / "Textures/tardis";
const std::string modelTextureExtension = "png";
const at::Transform modelTransform({ 0.0f, 0.0f, 0.0f }, { at::Math::HalfPi<float>, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f });
//*/

// Sponza
//*
const bool overrideMaterial = false;
const std::filesystem::path modelMeshPath = rscPath / "Models/sponza.obj"; // StMaryAbbots
const std::filesystem::path modelTexturePath = rscPath / "Textures/uzv-checker";
const std::string modelTextureExtension = "png";
const at::Transform modelTransform({ 0.0f, 0.0f, -1.0f }, { at::Math::HalfPi<float>, 0.0f, 0.0f }, { 0.01f, 0.01f, 0.01f });
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

struct ModelData
{
	ModelData(const std::filesystem::path& path);

	at::Ptr<at::Model> model;
};

at::Ptr<at::MaterialData> loadMaterialData(const std::filesystem::path& path, const std::string& extension);

#endif
