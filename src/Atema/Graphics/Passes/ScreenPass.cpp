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

#include <Atema/Graphics/Passes/ScreenPass.hpp>
#include <Atema/Graphics/FrameGraphBuilder.hpp>
#include <Atema/Graphics/FrameGraphContext.hpp>
#include <Atema/Graphics/RenderScene.hpp>
#include <Atema/Graphics/VertexBuffer.hpp>
#include <Atema/Graphics/Graphics.hpp>
#include <Atema/Graphics/VertexTypes.hpp>
#include <Atema/Renderer/DescriptorSetLayout.hpp>
#include <Atema/Renderer/Renderer.hpp>

using namespace at;

namespace
{
	constexpr char* ShaderName = "AtemaScreenPass";
	
	const char ShaderCode[] = R"(
include Atema.PostProcess;

external
{
	[set(0), binding(0)] sampler2Df colorTexture;
}

vec4f getPostProcessColor(vec2f uv)
{
	return sample(colorTexture, uv);
}
)";
}

ScreenPass::ScreenPass() :
	m_showUI(true)
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

const char* ScreenPass::getName() const noexcept
{
	return "Screen";
}

void ScreenPass::showUI(bool show)
{
	m_showUI = show;
}

FrameGraphPass& ScreenPass::addToFrameGraph(FrameGraphBuilder& frameGraphBuilder, const Settings& settings)
{
	auto& pass = frameGraphBuilder.createPass(getName());

	pass.enableRenderFrameOutput(true);

	pass.addSampledTexture(settings.input, ShaderStage::Fragment);

	Settings settingsCopy = settings;

	pass.setExecutionCallback([this, settingsCopy](FrameGraphContext& context)
		{
			execute(context, settingsCopy);
		});

	return pass;
}

void ScreenPass::execute(FrameGraphContext& context, const Settings& settings)
{
	const auto& renderScene = getRenderScene();

	if (!renderScene.isValid())
		return;

	const auto& viewport = getRenderScene().getCamera().getViewport();
	const auto& scissor = getRenderScene().getCamera().getScissor();
	
	auto descriptorSet = m_setLayout->createSet();

	descriptorSet->update(0, *context.getImageView(settings.input), *m_sampler);

	auto& commandBuffer = context.getCommandBuffer();

	commandBuffer.bindPipeline(*m_pipeline);

	commandBuffer.setViewport(viewport);

	commandBuffer.setScissor(scissor.pos, scissor.size);

	commandBuffer.bindDescriptorSet(0, *descriptorSet);

	commandBuffer.bindVertexBuffer(*m_quadMesh->getBuffer(), 0);

	commandBuffer.draw(static_cast<uint32_t>(m_quadMesh->getSize()));

	context.destroyAfterUse(std::move(descriptorSet));
	
	if (m_showUI)
	{
		auto uiContext = UiContext::getActive();
		if (uiContext)
			uiContext->draw(ImGui::GetDrawData(), commandBuffer);
	}
}
