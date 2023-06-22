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

#ifndef ATEMA_GRAPHICS_DEBUGRENDERER_HPP
#define ATEMA_GRAPHICS_DEBUGRENDERER_HPP

#include <Atema/Graphics/Config.hpp>
#include <Atema/Core/Pointer.hpp>
#include <Atema/Math/Vector.hpp>
#include <Atema/Math/AABB.hpp>
#include <Atema/Renderer/Color.hpp>
#include <Atema/Math/Frustum.hpp>
#include <Atema/Graphics/VertexFormat.hpp>

#include <vector>

namespace at
{
	class DescriptorSet;
	class DescriptorSetLayout;
	class Buffer;
	class FrameGraphContext;
	class CommandBuffer;
	class RenderFrame;
	class GraphicsPipeline;
	class VertexBuffer;

	class ATEMA_GRAPHICS_API DebugRenderer
	{
	public:
		DebugRenderer();
		DebugRenderer(const DebugRenderer& other) = default;
		DebugRenderer(DebugRenderer&& other) noexcept = default;
		~DebugRenderer();

		void clear();

		void drawLine(const Vector3f& point1, const Vector3f& point2, const Color& color = Color::White);
		void draw(const AABBf& aabb, const Color& color = Color::White);
		void draw(const Frustumf& frustum, const Color& color = Color::White);

		void render(FrameGraphContext& frameGraphContext, CommandBuffer& commandBuffer, const Matrix4f& viewProjection);

		DebugRenderer& operator=(const DebugRenderer& other) = default;
		DebugRenderer& operator=(DebugRenderer&& other) noexcept = default;
		
	private:
		struct Vertex
		{
			Vertex(const Vector3f& position, const Vector3f& color) : position(position), color(color) {}

			Vector3f position;
			Vector3f color;
		};

		struct Line
		{
			Vector3f point1;
			Vector3f point2;
		};

		Ptr<DescriptorSetLayout> m_descriptorSetLayout;
		Ptr<GraphicsPipeline> m_graphicsPipeline;
		VertexFormat m_vertexFormat;

		std::vector<Vertex> m_vertices;

		std::vector<Ptr<VertexBuffer>> m_vertexBuffers;
		std::vector<Ptr<VertexBuffer>> m_obsoleteVertexBuffers;

		Ptr<Buffer> m_uniformBuffer;

		Ptr<DescriptorSet> m_descriptorSet;
	};
}

#endif
