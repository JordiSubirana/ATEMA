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

#ifndef ATEMA_GRAPHICS_VERTEXTYPES_HPP
#define ATEMA_GRAPHICS_VERTEXTYPES_HPP

#include <Atema/Graphics/Config.hpp>
#include <Atema/Math/Vector.hpp>
#include <Atema/Renderer/VertexInput.hpp>
#include <Atema/Graphics/VertexFormat.hpp>
#include <Atema/Renderer/Color.hpp>

namespace at
{
	struct ATEMA_GRAPHICS_API Vertex_XY
	{
		Vector2f position;
		
		// Default vertex format
		static constexpr DefaultVertexFormat VertexFormat = DefaultVertexFormat::XY;

		// Default vertex input
		static const std::vector<VertexInput>& getVertexInput();
	};
	
	struct ATEMA_GRAPHICS_API Vertex_XY_RGB
	{
		Vector2f position;
		Vector3f color;

		// Default vertex format
		static constexpr DefaultVertexFormat VertexFormat = DefaultVertexFormat::XY_RGB;

		// Default vertex input
		static const std::vector<VertexInput>& getVertexInput();
	};

	struct ATEMA_GRAPHICS_API Vertex_XY_RGBA
	{
		Vector2f position;
		Color color;

		// Default vertex format
		static constexpr DefaultVertexFormat VertexFormat = DefaultVertexFormat::XY_RGBA;

		// Default vertex input
		static const std::vector<VertexInput>& getVertexInput();
	};

	struct ATEMA_GRAPHICS_API Vertex_XY_UV
	{
		Vector2f position;
		Vector2f texCoords;

		// Default vertex format
		static constexpr DefaultVertexFormat VertexFormat = DefaultVertexFormat::XY_UV;

		// Default vertex input
		static const std::vector<VertexInput>& getVertexInput();
	};

	struct ATEMA_GRAPHICS_API Vertex_XYZ
	{
		Vector3f position;

		// Default vertex format
		static constexpr DefaultVertexFormat VertexFormat = DefaultVertexFormat::XYZ;

		// Default vertex input
		static const std::vector<VertexInput>& getVertexInput();
	};

	struct ATEMA_GRAPHICS_API Vertex_XYZ_RGB
	{
		Vector3f position;
		Vector3f color;

		// Default vertex format
		static constexpr DefaultVertexFormat VertexFormat = DefaultVertexFormat::XYZ_RGB;

		// Default vertex input
		static const std::vector<VertexInput>& getVertexInput();
	};

	struct ATEMA_GRAPHICS_API Vertex_XYZ_RGB_N
	{
		Vector3f position;
		Vector3f color;
		Vector3f normal;

		// Default vertex format
		static constexpr DefaultVertexFormat VertexFormat = DefaultVertexFormat::XYZ_RGB_N;

		// Default vertex input
		static const std::vector<VertexInput>& getVertexInput();
	};

	struct ATEMA_GRAPHICS_API Vertex_XYZ_RGB_NT
	{
		Vector3f position;
		Vector3f color;
		Vector3f normal;
		Vector3f tangent;

		// Default vertex format
		static constexpr DefaultVertexFormat VertexFormat = DefaultVertexFormat::XYZ_RGB_NT;

		// Default vertex input
		static const std::vector<VertexInput>& getVertexInput();
	};

	struct ATEMA_GRAPHICS_API Vertex_XYZ_RGB_NTB
	{
		Vector3f position;
		Vector3f color;
		Vector3f normal;
		Vector3f tangent;
		Vector3f bitangent;

		// Default vertex format
		static constexpr DefaultVertexFormat VertexFormat = DefaultVertexFormat::XYZ_RGB_NTB;

		// Default vertex input
		static const std::vector<VertexInput>& getVertexInput();
	};

	struct ATEMA_GRAPHICS_API Vertex_XYZ_RGBA
	{
		Vector3f position;
		Color color;

		// Default vertex format
		static constexpr DefaultVertexFormat VertexFormat = DefaultVertexFormat::XYZ_RGBA;

		// Default vertex input
		static const std::vector<VertexInput>& getVertexInput();
	};

	struct ATEMA_GRAPHICS_API Vertex_XYZ_RGBA_N
	{
		Vector3f position;
		Color color;
		Vector3f normal;

		// Default vertex format
		static constexpr DefaultVertexFormat VertexFormat = DefaultVertexFormat::XYZ_RGBA_N;

		// Default vertex input
		static const std::vector<VertexInput>& getVertexInput();
	};

	struct ATEMA_GRAPHICS_API Vertex_XYZ_RGBA_NT
	{
		Vector3f position;
		Color color;
		Vector3f normal;
		Vector3f tangent;

		// Default vertex format
		static constexpr DefaultVertexFormat VertexFormat = DefaultVertexFormat::XYZ_RGBA_NT;

		// Default vertex input
		static const std::vector<VertexInput>& getVertexInput();
	};

	struct ATEMA_GRAPHICS_API Vertex_XYZ_RGBA_NTB
	{
		Vector3f position;
		Color color;
		Vector3f normal;
		Vector3f tangent;
		Vector3f bitangent;

		// Default vertex format
		static constexpr DefaultVertexFormat VertexFormat = DefaultVertexFormat::XYZ_RGBA_NTB;

		// Default vertex input
		static const std::vector<VertexInput>& getVertexInput();
	};

	struct ATEMA_GRAPHICS_API Vertex_XYZ_UV
	{
		Vector3f position;
		Vector2f texCoords;

		// Default vertex format
		static constexpr DefaultVertexFormat VertexFormat = DefaultVertexFormat::XYZ_UV;

		// Default vertex input
		static const std::vector<VertexInput>& getVertexInput();
	};

	struct ATEMA_GRAPHICS_API Vertex_XYZ_UV_N
	{
		Vector3f position;
		Vector2f texCoords;
		Vector3f normal;

		// Default vertex format
		static constexpr DefaultVertexFormat VertexFormat = DefaultVertexFormat::XYZ_UV_N;

		// Default vertex input
		static const std::vector<VertexInput>& getVertexInput();
	};

	struct ATEMA_GRAPHICS_API Vertex_XYZ_UV_NT
	{
		Vector3f position;
		Vector2f texCoords;
		Vector3f normal;
		Vector3f tangent;

		// Default vertex format
		static constexpr DefaultVertexFormat VertexFormat = DefaultVertexFormat::XYZ_UV_NT;

		// Default vertex input
		static const std::vector<VertexInput>& getVertexInput();
	};

	struct ATEMA_GRAPHICS_API Vertex_XYZ_UV_NTB
	{
		Vector3f position;
		Vector2f texCoords;
		Vector3f normal;
		Vector3f tangent;
		Vector3f bitangent;

		// Default vertex format
		static constexpr DefaultVertexFormat VertexFormat = DefaultVertexFormat::XYZ_UV_NTB;

		// Default vertex input
		static const std::vector<VertexInput>& getVertexInput();
	};
}

#endif