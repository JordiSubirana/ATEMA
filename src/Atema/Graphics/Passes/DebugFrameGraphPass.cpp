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

#include <Atema/Graphics/Passes/DebugFrameGraphPass.hpp>
#include <Atema/Graphics/FrameGraphBuilder.hpp>
#include <Atema/Graphics/FrameGraphContext.hpp>
#include <Atema/Graphics/Graphics.hpp>
#include <Atema/Graphics/RenderScene.hpp>
#include <Atema/Graphics/VertexTypes.hpp>
#include <Atema/Renderer/Renderer.hpp>

using namespace at;

namespace
{
	constexpr char* ShaderName = "AtemaDebugFrameGraphPass";

	const char ShaderCode[] = R"(
external
{
	[set(0), binding(0)] sampler2Df colorTexture;
}

include Atema.PostProcess;

[entry(fragment)]
void main()
{
	atPostProcessWriteOutColor(sample(colorTexture, atPostProcessGetTexCoords()));
}
)";
}

DebugFrameGraphPass::DebugFrameGraphPass() :
	AbstractRenderPass()
{
	auto& graphics = Graphics::instance();

	if (!graphics.uberShaderExists(ShaderName))
		graphics.setUberShader(ShaderName, ShaderCode);

	DescriptorSetLayout::Settings descriptorSetLayoutSettings;
	descriptorSetLayoutSettings.bindings =
	{
		{ DescriptorType::CombinedImageSampler, 0, 1, ShaderStage::Fragment }
	};
	descriptorSetLayoutSettings.pageSize = Renderer::FramesInFlight;

	m_setLayout = DescriptorSetLayout::create(descriptorSetLayoutSettings);

	const auto& shaderLibraryManager = ShaderLibraryManager::instance();

	GraphicsPipeline::Settings pipelineSettings;
	pipelineSettings.vertexShader = graphics.getShader(*graphics.getUberShaderFromString(std::string(ShaderName), AstShaderStage::Vertex, {}, &shaderLibraryManager));
	pipelineSettings.fragmentShader = graphics.getShader(*graphics.getUberShaderFromString(std::string(ShaderName), AstShaderStage::Fragment, {}, &shaderLibraryManager));
	pipelineSettings.descriptorSetLayouts = { m_setLayout };
	pipelineSettings.state.vertexInput.inputs = Vertex_XYZ_UV::getVertexInput();
	pipelineSettings.state.depth.test = false;
	pipelineSettings.state.depth.write = false;

	m_pipeline = GraphicsPipeline::create(pipelineSettings);

	m_sampler = graphics.getSampler(Sampler::Settings(SamplerFilter::Nearest));

	m_quadMesh = graphics.getQuadMesh();
}

const char* DebugFrameGraphPass::getName() const noexcept
{
	return "Debug FrameGraph";
}

FrameGraphPass& DebugFrameGraphPass::addToFrameGraph(FrameGraphBuilder& frameGraphBuilder, const Settings& settings)
{
	auto& pass = frameGraphBuilder.createPass(getName());

	if (settings.outputClearValue.has_value())
		pass.addOutputTexture(settings.output, 0, settings.outputClearValue.value());
	else
		pass.addOutputTexture(settings.output, 0);

	for (const auto& texture : settings.textures)
		pass.addSampledTexture(texture, ShaderStage::Fragment);

	Settings settingsCopy = settings;

	pass.setExecutionCallback([this, settingsCopy](FrameGraphContext& context)
		{
			execute(context, settingsCopy);
		});

	return pass;
}

void DebugFrameGraphPass::execute(FrameGraphContext& context, const Settings& settings)
{
	if (settings.textures.empty())
		return;

	auto columnCount = settings.columnCount;

	if (columnCount == 0)
		columnCount = static_cast<size_t>(std::ceil(std::sqrt(settings.textures.size())));
	
	const auto& scissor = getRenderScene().getCamera().getScissor();

	Viewport viewport;
	viewport.size = scissor.size;
	viewport.size /= static_cast<float>(columnCount);

	auto& commandBuffer = context.getCommandBuffer();

	commandBuffer.bindPipeline(*m_pipeline);

	commandBuffer.setScissor(scissor.pos, scissor.size);

	commandBuffer.bindVertexBuffer(*m_quadMesh->getBuffer(), 0);

	size_t index = 0;
	for (auto& texture : settings.textures)
	{
		const auto x = index % columnCount;
		const auto y = index / columnCount;

		viewport.position.x = static_cast<float>(x) * viewport.size.x;
		viewport.position.y = static_cast<float>(y) * viewport.size.y;

		commandBuffer.setViewport(viewport);

		auto descriptorSet = m_setLayout->createSet();
		descriptorSet->update(0, *context.getImageView(texture), *m_sampler);

		commandBuffer.bindDescriptorSet(0, *descriptorSet);

		commandBuffer.draw(static_cast<uint32_t>(m_quadMesh->getSize()));

		context.destroyAfterUse(std::move(descriptorSet));

		index++;
	}
}
