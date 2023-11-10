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

#ifndef ATEMA_GRAPHICS_ABSTRACTCUBEMAPPASS_HPP
#define ATEMA_GRAPHICS_ABSTRACTCUBEMAPPASS_HPP

#include <Atema/Graphics/Config.hpp>
#include <Atema/Graphics/AbstractRenderPass.hpp>

#include <optional>
#include <Atema/Graphics/FrameGraphContext.hpp>

namespace at
{
	class RenderMaterial;
	class Mesh;

	class ATEMA_GRAPHICS_API AbstractCubemapPass : public AbstractRenderPass
	{
	public:
		struct BaseFrameGraphSettings
		{
			// Order defined by the CubemapFace enum (Top, Left, Front, Right, Back, Bottom)
			// Must contain 6 elements
			std::vector<FrameGraphTextureHandle> outputCubemapFaces;
			std::optional<Color> clearColor;
		};

		AbstractCubemapPass();
		AbstractCubemapPass(const AbstractCubemapPass& other) = default;
		AbstractCubemapPass(AbstractCubemapPass&& other) noexcept = default;
		~AbstractCubemapPass() = default;

		AbstractCubemapPass& operator=(const AbstractCubemapPass& other) = default;
		AbstractCubemapPass& operator=(AbstractCubemapPass&& other) noexcept = default;

	protected:
		static size_t getCubemapFaceIndex(CubemapFace face);
		void initialize(Ptr<RenderMaterial> renderMaterial);
		RenderMaterial& getRenderMaterial() const;

		// This methods should not be called in derived classes
		FrameGraphPass& createFrameGraphPass(FrameGraphBuilder& frameGraphBuilder, const BaseFrameGraphSettings& settings);
		// This methods should not be called in derived classes
		void initializeDraw(FrameGraphContext& context, const BaseFrameGraphSettings& settings);
		// This methods should not be called in derived classes
		void draw(FrameGraphContext& context, const BaseFrameGraphSettings& settings);

	private:
		Ptr<Mesh> m_mesh;

		Ptr<RenderMaterial> m_renderMaterial;

		Ptr<Buffer> m_cubeDataBuffer;
		Ptr<DescriptorSet> m_cubeDescriptorSet;
	};

	template <typename FrameGraphSettingsType>
	class CubemapPass : public AbstractCubemapPass
	{
	public:
		using FrameGraphSettings = FrameGraphSettingsType;

		static_assert(std::is_base_of_v<BaseFrameGraphSettings, FrameGraphSettings>, "FrameGraph settings must be derived from AbstractCubemapPass::BaseFrameGraphSettings");

		CubemapPass() = default;
		CubemapPass(const CubemapPass& other) = default;
		CubemapPass(CubemapPass&& other) noexcept = default;
		~CubemapPass() = default;

		FrameGraphPass& addToFrameGraph(FrameGraphBuilder& frameGraphBuilder, const FrameGraphSettings& settings);

		void execute(FrameGraphContext& context, const FrameGraphSettings& settings);
		
		CubemapPass& operator=(const CubemapPass& other) = default;
		CubemapPass& operator=(CubemapPass&& other) noexcept = default;

	protected:
		// Defines additional textures for the given pass
		// Does nothing by default
		virtual void initializeFrameGraphPass(FrameGraphPass& frameGraphPass, FrameGraphBuilder& frameGraphBuilder, const FrameGraphSettings& settings);
		// Returns false if resources are not valid : the execution will do nothing
		// Does nothing by default
		virtual bool bindResources(FrameGraphContext& context, const FrameGraphSettings& settings);
	};
}

#include <Atema/Graphics/Passes/AbstractCubemapPass.inl>

#endif
