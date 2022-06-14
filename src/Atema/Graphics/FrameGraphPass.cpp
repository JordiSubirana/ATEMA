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

#include <Atema/Graphics/FrameGraphPass.hpp>
#include <Atema/Graphics/FrameGraph.hpp>
#include <Atema/Graphics/FrameGraphBuilder.hpp>

using namespace at;

FrameGraphPass::FrameGraphPass(const FrameGraphBuilder& frameGraphBuilder, const std::string& name) :
	m_frameGraphBuilder(frameGraphBuilder),
	m_name(name),
	m_useRenderFrameOutput(false),
	m_useSecondaryCommandBuffers(false),
	m_depthTexture(FrameGraph::InvalidTextureHandle)
{
}

FrameGraphPass::~FrameGraphPass()
{
}

const std::string& FrameGraphPass::getName() const noexcept
{
	return m_name;
}

void FrameGraphPass::enableRenderFrameOutput(bool enable)
{
	m_useRenderFrameOutput = enable;
}

void FrameGraphPass::enableSecondaryCommandBuffers(bool enable)
{
	m_useSecondaryCommandBuffers = enable;
}

void FrameGraphPass::setExecutionCallback(const ExecutionCallback& callback)
{
	m_executionCallback = callback;
}

void FrameGraphPass::addSampledTexture(FrameGraphTextureHandle textureHandle, Flags<ShaderStage> shaderStages)
{
	registerTexture(textureHandle);

	m_sampledTextures.emplace_back(textureHandle);

	m_samplingStages[textureHandle] = shaderStages;
}

void FrameGraphPass::addInputTexture(FrameGraphTextureHandle textureHandle, uint32_t index)
{
	if (!validateSize(textureHandle, m_inputSize))
	{
		ATEMA_ERROR("All input textures must have the same size");
	}

	registerTexture(textureHandle);

	m_inputTextures.emplace_back(textureHandle);

	m_inputIndices[textureHandle] = index;
}

void FrameGraphPass::addOutputTexture(FrameGraphTextureHandle textureHandle, uint32_t index)
{
	if (!validateSize(textureHandle, m_outputSize))
	{
		ATEMA_ERROR("All output / depth textures must have the same size");
	}

	registerTexture(textureHandle);

	m_outputTextures.emplace_back(textureHandle);

	m_outputIndices[textureHandle] = index;
}

void FrameGraphPass::addOutputTexture(FrameGraphTextureHandle textureHandle, uint32_t index, const Color& clearColor)
{
	addOutputTexture(textureHandle, index);

	m_clearedTextures.emplace_back(textureHandle);

	m_clearColors[textureHandle] = clearColor;
}

void FrameGraphPass::setDepthTexture(FrameGraphTextureHandle textureHandle)
{
	if (!validateSize(textureHandle, m_outputSize))
	{
		ATEMA_ERROR("All output / depth textures must have the same size");
	}

	registerTexture(textureHandle);

	m_depthTexture = textureHandle;
}

void FrameGraphPass::setDepthTexture(FrameGraphTextureHandle textureHandle, const DepthStencil& depthStencil)
{
	setDepthTexture(textureHandle);

	m_clearedTextures.emplace_back(textureHandle);

	m_clearDepths[textureHandle] = depthStencil;
}

bool FrameGraphPass::useRenderFrameOutput() const noexcept
{
	return m_useRenderFrameOutput;
}

bool FrameGraphPass::useSecondaryCommandBuffers() const noexcept
{
	return m_useSecondaryCommandBuffers;
}

const FrameGraphPass::ExecutionCallback& FrameGraphPass::getExecutionCallback() const noexcept
{
	return m_executionCallback;
}

const std::vector<FrameGraphTextureHandle>& FrameGraphPass::getSampledTextures() const noexcept
{
	return m_sampledTextures;
}

const std::vector<FrameGraphTextureHandle>& FrameGraphPass::getInputTextures() const noexcept
{
	return m_inputTextures;
}

const std::vector<FrameGraphTextureHandle>& FrameGraphPass::getOutputTextures() const noexcept
{
	return m_outputTextures;
}

const std::vector<FrameGraphTextureHandle>& FrameGraphPass::getClearedTextures() const noexcept
{
	return m_clearedTextures;
}

FrameGraphTextureHandle FrameGraphPass::getDepthTexture() const noexcept
{
	return m_depthTexture;
}

uint32_t FrameGraphPass::getInputIndex(FrameGraphTextureHandle textureHandle) const
{
	if (m_inputIndices.count(textureHandle) == 0)
	{
		ATEMA_ERROR("Invalid input binding index : the texture was not declared as an input");
	}

	return m_inputIndices.at(textureHandle);
}

uint32_t FrameGraphPass::getOutputIndex(FrameGraphTextureHandle textureHandle) const
{
	if (m_outputIndices.count(textureHandle) == 0)
	{
		ATEMA_ERROR("Invalid output binding index : the texture was not declared as an output");
	}

	return m_outputIndices.at(textureHandle);
}

Flags<ShaderStage> FrameGraphPass::getSamplingStages(FrameGraphTextureHandle textureHandle) const
{
	if (m_samplingStages.count(textureHandle) == 0)
	{
		ATEMA_ERROR("Invalid sampling stages : the texture was not declared as sampled");
	}

	return m_samplingStages.at(textureHandle);
}

const Vector2u& FrameGraphPass::getOutputSize() const noexcept
{
	return m_outputSize;
}

void FrameGraphPass::registerTexture(FrameGraphTextureHandle textureHandle)
{
	if (m_textures.count(textureHandle) != 0)
	{
		ATEMA_ERROR("The texture can't be processed multiple times by the same pass");
	}

	m_textures.emplace(textureHandle);
}

bool FrameGraphPass::validateSize(FrameGraphTextureHandle textureHandle, Vector2u& size)
{
	const auto& textureSettings = m_frameGraphBuilder.getTextureSettings(textureHandle);

	if (size.x == 0 && size.y == 0)
	{
		size = { textureSettings.width, textureSettings.height };
	}
	else if (textureSettings.width != size.x || textureSettings.height != size.y)
	{
		return false;
	}

	return true;
}
