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

struct ResourcePath
{
	static std::filesystem::path resources() noexcept;
	static std::filesystem::path fonts() noexcept;
	static std::filesystem::path models() noexcept;
	static std::filesystem::path shaders() noexcept;
	static std::filesystem::path textures() noexcept;
};

struct ResourceLoader
{
	struct ModelSettings
	{
		std::filesystem::path modelPath;

		bool overrideMaterial = false;
		std::filesystem::path modelTexturePath;
		std::string modelTextureExtension;

		std::optional<at::Matrix4f> vertexTransformation;
	};

	static at::Ptr<at::Model> loadModel(const ModelSettings& modelSettings);

	static at::Ptr<at::MaterialData> loadMaterialData(const std::filesystem::path& path, const std::string& extension);
};

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

// Between -1 & 1
float randFloat();

#endif
