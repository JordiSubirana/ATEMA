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

#ifndef ATEMA_RENDERER_GRAPHICSPIPELINE_HPP
#define ATEMA_RENDERER_GRAPHICSPIPELINE_HPP

#include <Atema/Renderer/Config.hpp>
#include <Atema/Core/NonCopyable.hpp>
#include <Atema/Core/Pointer.hpp>
#include <Atema/Renderer/Enums.hpp>
#include <Atema/Renderer/VertexInput.hpp>
#include <Atema/Renderer/DescriptorSetLayout.hpp>

#include <vector>

namespace at
{
	class RenderPass;
	class Shader;

	class ATEMA_RENDERER_API GraphicsPipeline : public NonCopyable
	{
	public:
		struct VertexInputState
		{
			std::vector<VertexInput> inputs;
		};

		struct InputAssemblyState
		{
			PrimitiveTopology topology = PrimitiveTopology::TriangleList;
			bool primitiveRestart = false;
		};

		struct RasterizationState
		{
			bool depthClamp = false;
			bool rasterizerDiscard = false;
			PolygonMode polygonMode = PolygonMode::Fill;
			float lineWidth = 1.0f;
			Flags<CullMode> cullMode = CullMode::Back;
			FrontFace frontFace = FrontFace::CounterClockwise;
			//TODO: Add depth bias
		};

		struct MultisampleState
		{
			ImageSamples samples = ImageSamples::S1;
			bool sampleShading = false;
			float minSampleShading = 0.0f;
		};

		struct ColorBlendState
		{
			bool enabled = false;
			BlendOperation colorOperation = BlendOperation::Add;
			BlendFactor colorSrcFactor = BlendFactor::SrcAlpha;
			BlendFactor colorDstFactor = BlendFactor::OneMinusSrcAlpha;
			BlendOperation alphaOperation = BlendOperation::Add;
			BlendFactor alphaSrcFactor = BlendFactor::One;
			BlendFactor alphaDstFactor = BlendFactor::Zero;
		};

		struct DepthState
		{
			bool test = true;
			bool write = true;
			CompareOperation compareOperation = CompareOperation::Less;
			bool boundsTest = false;
			float boundsMin = 0.0f;
			float boundsMax = 1.0f;
		};

		struct StencilState
		{
			StencilOperation failOperation = StencilOperation::Keep;
			StencilOperation passOperation = StencilOperation::Keep;
			StencilOperation depthFailOperation = StencilOperation::Keep;
			CompareOperation compareOperation = CompareOperation::Less;
			uint32_t compareMask = 0;
			uint32_t writeMask = 0;
			uint32_t reference = 0;
		};

		struct State
		{
			VertexInputState vertexInput;
			InputAssemblyState inputAssembly;
			RasterizationState rasterization;
			MultisampleState multisample;
			ColorBlendState colorBlend;
			DepthState depth;
			bool stencil = false;
			StencilState stencilFront;
			StencilState stencilBack;
		};

		struct Settings
		{
			State state;

			std::vector<Ptr<DescriptorSetLayout>> descriptorSetLayouts;
			
			Ptr<Shader> vertexShader;
			Ptr<Shader> fragmentShader;
		};
		
		virtual ~GraphicsPipeline();

		static Ptr<GraphicsPipeline> create(const Settings& settings);

		const std::vector<Ptr<DescriptorSetLayout>>& getDescriptorSetLayouts() const;

		const State& getState() const;

	protected:
		GraphicsPipeline();

	private:
		std::vector<Ptr<DescriptorSetLayout>> m_descriptorSetLayouts;
		State m_state;
	};

	template <>
	struct HashOverload<GraphicsPipeline::Settings>
	{
		template <typename Hasher>
		static constexpr auto hash(const GraphicsPipeline::Settings& settings)
		{
			typename Hasher::HashType hash = 0;

			Hasher::hashCombine(hash, Hasher::hash(static_cast<const void*>(settings.state.vertexInput.inputs.data()), settings.state.vertexInput.inputs.size() * sizeof(VertexInput)));
			Hasher::hashCombine(hash, settings.state.inputAssembly);
			Hasher::hashCombine(hash, settings.state.rasterization);
			Hasher::hashCombine(hash, settings.state.multisample);
			Hasher::hashCombine(hash, settings.state.colorBlend);
			Hasher::hashCombine(hash, settings.state.depth);
			Hasher::hashCombine(hash, settings.state.stencil);
			Hasher::hashCombine(hash, settings.state.stencilFront);
			Hasher::hashCombine(hash, settings.state.stencilBack);

			for (const auto& descriptorSetLayout : settings.descriptorSetLayouts)
				Hasher::hashCombine(hash, descriptorSetLayout.get());

			Hasher::hashCombine(hash, settings.vertexShader.get());
			Hasher::hashCombine(hash, settings.fragmentShader.get());

			return hash;
		}
	};
}

#endif
