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

#include <algorithm>
#include <Atema/Core/Error.hpp>
#include <Atema/Graphics/FrameGraphBuilder.hpp>
#include <Atema/Renderer/Framebuffer.hpp>
#include <Atema/Renderer/Image.hpp>
#include <Atema/Renderer/ImageView.hpp>
#include <Atema/Renderer/Renderer.hpp>
#include <Atema/Renderer/RenderPass.hpp>

using namespace at;

namespace
{
	Flags<PipelineStage> getShaderPipelineStages(Flags<ShaderStage> shaderStages)
	{
		Flags<PipelineStage> pipelineStages;

		if (shaderStages & ShaderStage::Vertex)
			pipelineStages |= PipelineStage::VertexShader;

		if (shaderStages & ShaderStage::TessellationControl)
			pipelineStages |= PipelineStage::TessellationControl;

		if (shaderStages & ShaderStage::TessellationEvaluation)
			pipelineStages |= PipelineStage::TessellationEvaluation;

		if (shaderStages & ShaderStage::Geometry)
			pipelineStages |= PipelineStage::GeometryShader;

		if (shaderStages & ShaderStage::Fragment)
			pipelineStages |= PipelineStage::FragmentShader;

		if (shaderStages & ShaderStage::Compute)
			pipelineStages |= PipelineStage::ComputeShader;

		return pipelineStages;
	}
}

// TextureData
size_t FrameGraphBuilder::TextureData::nextRead(size_t passIndex) const
{
	size_t nextIndex = InvalidPassIndex;

	for (auto& index : sampled)
	{
		if (index > passIndex)
		{
			nextIndex = index;
			break;
		}
	}

	for (auto& index : input)
	{
		if (index > passIndex)
		{
			if (nextIndex == InvalidPassIndex)
				nextIndex = index;
			else
				nextIndex = std::min(nextIndex, index);

			break;
		}
	}

	return nextIndex;
}

size_t FrameGraphBuilder::TextureData::nextWrite(size_t passIndex) const
{
	size_t nextIndex = InvalidPassIndex;

	for (auto& index : output)
	{
		if (index > passIndex)
		{
			nextIndex = index;
			break;
		}
	}

	for (auto& index : depth)
	{
		if (index > passIndex)
		{
			if (nextIndex == InvalidPassIndex)
				nextIndex = index;
			else
				nextIndex = std::min(nextIndex, index);

			break;
		}
	}

	return nextIndex;
}

size_t FrameGraphBuilder::TextureData::nextClear(size_t passIndex) const
{
	for (auto& index : clear)
	{
		if (index > passIndex)
			return index;
	}

	return InvalidPassIndex;
}

size_t FrameGraphBuilder::TextureData::nextUse(size_t passIndex) const
{
	return std::min(std::min(nextRead(passIndex), nextWrite(passIndex)), nextClear(passIndex));
}

bool FrameGraphBuilder::TextureData::doClear(size_t passIndex) const
{
	for (auto& index : clear)
	{
		if (index == passIndex)
			return true;
	}

	return false;
}

// FrameGraphBuilder
FrameGraphBuilder::FrameGraphBuilder() :
	m_renderFrameColorTextureHandle(FrameGraph::InvalidTextureHandle),
	m_renderFrameDepthTextureHandle(FrameGraph::InvalidTextureHandle)
{
}

FrameGraphBuilder::~FrameGraphBuilder()
{
}

FrameGraphTextureHandle FrameGraphBuilder::createTexture(const FrameGraphTextureSettings& settings)
{
	m_textures.emplace_back(settings);

	return m_textures.size() - 1;
}

FrameGraphTextureHandle FrameGraphBuilder::importTexture(const Ptr<Image>& image, uint32_t layer, uint32_t mipLevel)
{
	ATEMA_ASSERT(image, "Invalid image");
	ATEMA_ASSERT(layer < image->getLayers(), "Invalid image layer");
	ATEMA_ASSERT(mipLevel < image->getMipLevels(), "Invalid image mip level");

	auto imageSize = image->getSize();

	if (mipLevel > 0)
	{
		imageSize /= 1 << mipLevel;

		if (imageSize.x < 1)
			imageSize.x = 1;

		if (imageSize.y < 1)
			imageSize.y = 1;
	}

	FrameGraphTextureSettings textureSettings;
	textureSettings.width = static_cast<uint32_t>(imageSize.x);
	textureSettings.height = static_cast<uint32_t>(imageSize.y);
	textureSettings.format = image->getFormat();

	const auto textureHandle = createTexture(textureSettings);

	m_importedTextures[textureHandle] = image;
	m_importedViews[textureHandle] = image->getView(layer, 1, mipLevel, 1);

	return textureHandle;
}

FrameGraphPass& FrameGraphBuilder::createPass(const std::string& name)
{
	return *m_passes.emplace_back(new FrameGraphPass(*this, name));
}

Ptr<FrameGraph> FrameGraphBuilder::build()
{
	createRenderFrameOutput();

	createTextureDatas();

	createPassDatas();

	orderPasses();

	updateTextureDatas();

	createPhysicalTextureAliases();

	createPhysicalTextures();

	createPhysicalPasses();

	auto frameGraph = std::make_shared<FrameGraph>();

	// Initialize passes
	auto& passes = frameGraph->m_passes;
	passes.resize(m_passDatas.size());

	for (size_t passIndex = 0; passIndex < m_passDatas.size(); passIndex++)
	{
		const auto& frameGraphPass = *m_passDatas[passIndex].pass;
		auto& physicalPass = m_physicalPasses[passIndex];
		
		auto& pass = passes[passIndex];

		pass.name = frameGraphPass.getName();
		pass.useRenderFrameOutput = physicalPass.useRenderFrameOutput;
		pass.renderPass = std::move(physicalPass.renderPass);
		pass.framebuffer = std::move(physicalPass.framebuffer);
		pass.clearValues = std::move(physicalPass.clearValues);
		pass.executionCallback = frameGraphPass.getExecutionCallback();
		pass.useSecondaryCommandBuffers = frameGraphPass.useSecondaryCommandBuffers();

		for (auto& textureHandle : frameGraphPass.getSampledTextures())
		{
			const auto& textureData = m_textureDatas[textureHandle];
			pass.textures[textureHandle] = textureData.physicalTexture->image;
			pass.views[textureHandle] = textureData.physicalTexture->imageView;
		}

		for (auto& textureHandle : frameGraphPass.getInputTextures())
		{
			const auto& textureData = m_textureDatas[textureHandle];
			pass.textures[textureHandle] = textureData.physicalTexture->image;
			pass.views[textureHandle] = textureData.physicalTexture->imageView;
		}

		for (auto& textureHandle : frameGraphPass.getOutputTextures())
		{
			const auto& textureData = m_textureDatas[textureHandle];
			pass.textures[textureHandle] = textureData.physicalTexture->image;
			pass.views[textureHandle] = textureData.physicalTexture->imageView;
		}

		const auto depthTextureHandle = frameGraphPass.getDepthTexture();
		if (depthTextureHandle != FrameGraph::InvalidTextureHandle)
		{
			const auto& textureData = m_textureDatas[depthTextureHandle];
			pass.textures[depthTextureHandle] = textureData.physicalTexture->image;
			pass.views[depthTextureHandle] = textureData.physicalTexture->imageView;
		}
	}

	// Initialize textures
	auto& textures = frameGraph->m_textures;
	textures.resize(m_physicalTextures.size());

	for (size_t index = 0; index < m_physicalTextures.size(); index++)
	{
		const auto& physicalTexture = m_physicalTextures[index];

		for (auto& textureHandle : physicalTexture->textureHandles)
		{
			const auto& textureData = m_textureDatas[textureHandle];

			for (auto& passIndex : textureData.sampled)
				passes[passIndex].textures[textureHandle] = physicalTexture->image;

			for (auto& passIndex : textureData.input)
				passes[passIndex].textures[textureHandle] = physicalTexture->image;

			for (auto& passIndex : textureData.output)
				passes[passIndex].textures[textureHandle] = physicalTexture->image;

			for (auto& passIndex : textureData.depth)
				passes[passIndex].textures[textureHandle] = physicalTexture->image;
		}

		auto& texture = textures[index];

		texture.image = std::move(physicalTexture->image);
		texture.barriers = std::move(physicalTexture->barriers);
	}

	return frameGraph;
}

const FrameGraphTextureSettings& FrameGraphBuilder::getTextureSettings(FrameGraphTextureHandle textureHandle) const
{
	return m_textures[textureHandle];
}

void FrameGraphBuilder::clearTempData()
{
	m_textureDatas.clear();
	m_passDatas.clear();
	m_physicalPasses.clear();
	m_physicalTextures.clear();
	m_physicalTextures.clear();
}

void FrameGraphBuilder::createRenderFrameOutput()
{
	FrameGraphTextureSettings textureSettings;
	textureSettings.width = 0;
	textureSettings.height = 0;

	// Dummy color texture settings
	textureSettings.format = ImageFormat::RGBA8_UNORM;

	m_renderFrameColorTextureHandle = createTexture(textureSettings);

	// Dummy depth texture settings
	textureSettings.format = ImageFormat::D32_SFLOAT;

	m_renderFrameDepthTextureHandle = createTexture(textureSettings);
}

bool FrameGraphBuilder::isRenderFrameOutput(FrameGraphTextureHandle textureHandle) const noexcept
{
	return textureHandle != FrameGraph::InvalidTextureHandle && (textureHandle == m_renderFrameColorTextureHandle || textureHandle == m_renderFrameDepthTextureHandle);
}

void FrameGraphBuilder::createTextureDatas()
{
	m_textureDatas.clear();
	m_textureDatas.resize(m_textures.size());

	for (size_t passIndex = 0; passIndex < m_passes.size(); passIndex++)
	{
		const auto& pass = *(m_passes[passIndex]);

		for (auto& textureHandle : pass.getSampledTextures())
			m_textureDatas[textureHandle].sampled.emplace_back(passIndex);

		for (auto& textureHandle : pass.getInputTextures())
			m_textureDatas[textureHandle].input.emplace_back(passIndex);

		for (auto& textureHandle : pass.getOutputTextures())
			m_textureDatas[textureHandle].output.emplace_back(passIndex);

		for (auto& textureHandle : pass.getClearedTextures())
			m_textureDatas[textureHandle].clear.emplace_back(passIndex);

		const auto depthTextureHandle = pass.getDepthTexture();
		if (depthTextureHandle != FrameGraph::InvalidTextureHandle)
			m_textureDatas[depthTextureHandle].depth.emplace_back(passIndex);

		if (pass.useRenderFrameOutput())
		{
			m_textureDatas[m_renderFrameColorTextureHandle].output.emplace_back(passIndex);
			m_textureDatas[m_renderFrameDepthTextureHandle].depth.emplace_back(passIndex);
		}
	}

	for (const auto& imageKV : m_importedTextures)
	{
		m_textureDatas[imageKV.first].imported = true;
	}

	m_textureDatas[m_renderFrameColorTextureHandle].imported = true;
	m_textureDatas[m_renderFrameDepthTextureHandle].imported = true;

	for (size_t textureIndex = 0; textureIndex < m_textureDatas.size(); textureIndex++)
	{
		auto& textureData = m_textureDatas[textureIndex];
		textureData.textureHandle = textureIndex;

		// Check if texture is used as a final output
		if (!textureData.output.empty() || !textureData.depth.empty())
		{
			if (textureData.imported)
			{
				textureData.finalOutput = true;
			}
		}
	}
}

void FrameGraphBuilder::createPassDatas()
{
	m_passDatas.clear();
	m_passDatas.resize(m_passes.size());

	// Build passes dependencies
	for (const auto& textureData : m_textureDatas)
	{
		for (const auto& inputPassIndex : textureData.sampled)
		{
			for (const auto& outputPassIndex : textureData.output)
				m_passDatas[inputPassIndex].dependencies.emplace(outputPassIndex);

			for (const auto& depthPassIndex : textureData.depth)
				m_passDatas[inputPassIndex].dependencies.emplace(depthPassIndex);
		}

		for (const auto& inputPassIndex : textureData.input)
		{
			for (const auto& outputPassIndex : textureData.output)
				m_passDatas[inputPassIndex].dependencies.emplace(outputPassIndex);

			for (const auto& depthPassIndex : textureData.depth)
				m_passDatas[inputPassIndex].dependencies.emplace(depthPassIndex);
		}
	}

	// Ensure the graph is acyclic and assign proper pass
	for (size_t passIndex = 0; passIndex < m_passDatas.size(); passIndex++)
	{
		const auto& pass = m_passes[passIndex];
		auto& passData = m_passDatas[passIndex];

		passData.pass = pass.get();

		if (passData.dependencies.count(passIndex))
		{
			ATEMA_ERROR("FrameGraph pass '" + pass->getName() + "' depends on itself");
		}

		std::unordered_set<size_t> processedPassIndices;
		for (auto& dependencyIndex : passData.dependencies)
		{
			checkPassDependencies(passIndex, dependencyIndex, processedPassIndices);
		}
	}

	// Find if there are some unused passes (i.e. a pass that doesn't serve output in any way)
	for (const auto& textureData : m_textureDatas)
	{
		if (textureData.finalOutput)
		{
			for (const auto& passIndex : textureData.output)
				setPassUsed(passIndex);

			for (const auto& passIndex : textureData.depth)
				setPassUsed(passIndex);
		}
	}
}

void FrameGraphBuilder::setPassUsed(size_t passIndex)
{
	auto& passData = m_passDatas[passIndex];

	if (!passData.used)
	{
		passData.used = true;

		const auto& pass = m_passes[passIndex];

		// Ensure the pass as a valid callback
		if (!pass->getExecutionCallback())
		{
			ATEMA_ERROR("No execution callback defined for pass '" + pass->getName() + "'");
		}

		// Flag which textures are used
		for (auto& textureHandle : pass->getSampledTextures())
			m_textureDatas[textureHandle].used = true;

		for (auto& textureHandle : pass->getInputTextures())
			m_textureDatas[textureHandle].used = true;

		for (auto& textureHandle : pass->getOutputTextures())
			m_textureDatas[textureHandle].used = true;

		const auto depthTextureHandle = pass->getDepthTexture();
		if (depthTextureHandle != FrameGraph::InvalidTextureHandle)
			m_textureDatas[depthTextureHandle].used = true;

		if (pass->useRenderFrameOutput())
		{
			m_textureDatas[m_renderFrameColorTextureHandle].used = true;
			m_textureDatas[m_renderFrameDepthTextureHandle].used = true;
		}

		// Make all dependencies used
		for (auto& dependencyIndex : passData.dependencies)
			setPassUsed(dependencyIndex);
	}
}

void FrameGraphBuilder::checkPassDependencies(size_t passIndex, size_t dependencyIndex, std::unordered_set<size_t>& processedPassIndices)
{
	// Avoid infinite loops
	if (processedPassIndices.count(dependencyIndex))
		return;

	processedPassIndices.emplace(dependencyIndex);

	const auto& dependencyPass = m_passDatas[dependencyIndex];

	for (const auto& index : dependencyPass.dependencies)
	{
		if (passIndex == index)
		{
			ATEMA_ERROR("Cyclic dependency between passes '" + m_passes[passIndex]->getName() + "' and '" + m_passes[dependencyIndex]->getName() + "'");
		}

		checkPassDependencies(passIndex, index, processedPassIndices);
	}
}

bool FrameGraphBuilder::dependsOn(size_t passIndex1, size_t passIndex2) const noexcept
{
	if (passIndex1 == passIndex2)
		return true;

	for (const auto& dependencyIndex : m_passDatas[passIndex2].dependencies)
	{
		if (dependsOn(passIndex1, dependencyIndex))
			return true;
	}

	return false;
}

void FrameGraphBuilder::orderPasses()
{
	std::vector<size_t> orderedPasses;

	// Consider a pass only if it is used
	for (size_t passIndex = 0; passIndex < m_passes.size(); passIndex++)
	{
		if (m_passDatas[passIndex].used)
			orderedPasses.emplace_back(passIndex);
	}

	// Order passes according to dependencies
	// If no dependency was found, just sort according to pass index
	std::sort(orderedPasses.begin(), orderedPasses.end(), [&](size_t passIndex1, size_t passIndex2) -> bool
		{
			if (dependsOn(passIndex1, passIndex2))
				return true;

			if (dependsOn(passIndex2, passIndex1))
				return false;

			return passIndex1 < passIndex2;
		});

	// Register new pass indices
	std::vector<size_t> newPassIndices(m_passDatas.size(), InvalidPassIndex);

	for (size_t newPassIndex = 0; newPassIndex < orderedPasses.size(); newPassIndex++)
	{
		const auto oldPassIndex = orderedPasses[newPassIndex];

		newPassIndices[oldPassIndex] = newPassIndex;
	}

	// Update pass indices in PassData
	std::vector<PassData> newPassDatas(orderedPasses.size());

	for (size_t newPassIndex = 0; newPassIndex < orderedPasses.size(); newPassIndex++)
	{
		const auto oldPassIndex = orderedPasses[newPassIndex];

		const auto& oldPassData = m_passDatas[oldPassIndex];

		auto& newPassData = newPassDatas[newPassIndex];
		newPassData.used = true;
		newPassData.pass = oldPassData.pass;

		for (auto& oldDependencyIndex : oldPassData.dependencies)
		{
			const auto& dependencyPass = m_passDatas[oldDependencyIndex];

			if (dependencyPass.used)
				newPassData.dependencies.emplace(newPassIndices[oldDependencyIndex]);
		}
	}

	std::swap(m_passDatas, newPassDatas);
}

void FrameGraphBuilder::updateTextureDatas()
{
	// Update pass indices in TextureData
	for (auto& textureData : m_textureDatas)
	{
		textureData.sampled.clear();
		textureData.input.clear();
		textureData.output.clear();
		textureData.depth.clear();
		textureData.clear.clear();
		textureData.usages.resize(m_passDatas.size());
	}

	for (size_t newPassIndex = 0; newPassIndex < m_passDatas.size(); newPassIndex++)
	{
		const auto& passData = m_passDatas[newPassIndex];

		if (passData.used)
		{
			const auto pass = passData.pass;

			for (auto& textureHandle : pass->getSampledTextures())
			{
				m_textureDatas[textureHandle].sampled.emplace_back(newPassIndex);
				m_textureDatas[textureHandle].usages[newPassIndex] |= TextureUsage::Sampled;
			}

			for (auto& textureHandle : pass->getInputTextures())
			{
				m_textureDatas[textureHandle].input.emplace_back(newPassIndex);
				m_textureDatas[textureHandle].usages[newPassIndex] |= TextureUsage::Input;
			}

			for (auto& textureHandle : pass->getOutputTextures())
			{
				m_textureDatas[textureHandle].output.emplace_back(newPassIndex);
				m_textureDatas[textureHandle].usages[newPassIndex] |= TextureUsage::Output;
			}

			for (auto& textureHandle : pass->getClearedTextures())
			{
				m_textureDatas[textureHandle].clear.emplace_back(newPassIndex);
				m_textureDatas[textureHandle].usages[newPassIndex] |= TextureUsage::Clear;
			}

			const auto depthTextureHandle = pass->getDepthTexture();
			if (depthTextureHandle != FrameGraph::InvalidTextureHandle)
			{
				m_textureDatas[depthTextureHandle].depth.emplace_back(newPassIndex);
				m_textureDatas[depthTextureHandle].usages[newPassIndex] |= TextureUsage::Depth;
			}

			if (pass->useRenderFrameOutput())
			{
				m_textureDatas[m_renderFrameColorTextureHandle].output.emplace_back(newPassIndex);
				m_textureDatas[m_renderFrameColorTextureHandle].usages[newPassIndex] |= TextureUsage::Output;

				m_textureDatas[m_renderFrameDepthTextureHandle].depth.emplace_back(newPassIndex);
				m_textureDatas[m_renderFrameDepthTextureHandle].usages[newPassIndex] |= TextureUsage::Depth;
			}
		}
	}

	// Check first/last read/write for each texture resource
	for (auto& data : m_textureDatas)
	{
		if (!data.used)
			continue;

		// First/last read
		if (!data.sampled.empty())
		{
			data.readRange.first = data.sampled.front();
			data.readRange.last = data.sampled.back();
		}

		if (!data.input.empty())
		{
			data.readRange.first = std::min(data.readRange.first, data.input.front());

			if (data.readRange.last == InvalidPassIndex)
				data.readRange.last = data.input.back();
			else
				data.readRange.last = std::max(data.readRange.last, data.input.back());
		}

		// First/last write
		if (!data.output.empty())
		{
			data.writeRange.first = data.output.front();
			data.writeRange.last = data.output.back();
		}

		if (!data.depth.empty())
		{
			data.writeRange.first = std::min(data.writeRange.first, data.depth.front());

			if (data.writeRange.last == InvalidPassIndex)
				data.writeRange.last = data.depth.back();
			else
				data.writeRange.last = std::max(data.writeRange.last, data.depth.back());
		}

		if (data.readRange.first == InvalidPassIndex)
			data.useRange.first = data.writeRange.first;
		else if (data.writeRange.first == InvalidPassIndex)
			data.useRange.first = data.readRange.first;
		else
			data.useRange.first = std::min(data.readRange.first, data.writeRange.first);

		if (data.readRange.last == InvalidPassIndex)
			data.useRange.last = data.writeRange.last;
		else if (data.writeRange.last == InvalidPassIndex)
			data.useRange.last = data.readRange.last;
		else
			data.useRange.last = std::max(data.readRange.last, data.writeRange.last);
	}
}

void FrameGraphBuilder::createPhysicalTextureAliases()
{
	m_physicalTextureAliases.clear();

	for (size_t textureHandle = 0; textureHandle < m_textures.size(); textureHandle++)
	{
		const auto& textureData = m_textureDatas[textureHandle];

		if (!textureData.used)
			continue;

		const auto& texture = m_textures[textureHandle];

		// Create physical texture
		auto& physicalTexture = m_physicalTextureAliases.emplace_back();
		physicalTexture.textureHandle = textureData.textureHandle;
		physicalTexture.imported = textureData.imported;
		physicalTexture.range = textureData.useRange;

		// Initialize general image settings
		auto& imageSettings = physicalTexture.imageSettings;
		imageSettings.format = texture.format;
		imageSettings.width = texture.width;
		imageSettings.height = texture.height;

		// Initialize image usages
		if (!textureData.sampled.empty())
			imageSettings.usages |= ImageUsage::ShaderSampling;

		if (!textureData.input.empty())
			imageSettings.usages |= ImageUsage::ShaderInput;

		if (!textureData.output.empty())
			imageSettings.usages |= ImageUsage::RenderTarget;

		if (!textureData.depth.empty())
			imageSettings.usages |= ImageUsage::RenderTarget;
	}
}

void FrameGraphBuilder::createPhysicalTextures()
{
	m_physicalTextures.clear();

	for (auto& textureAlias : m_physicalTextureAliases)
	{
		Ptr<PhysicalTexture> physicalTexture;

		if (textureAlias.imported)
		{
			physicalTexture = std::make_shared<PhysicalTexture>();

			physicalTexture->imageSettings = textureAlias.imageSettings;

			// We will use RenderFrame's image when rendering
			if (!isRenderFrameOutput(textureAlias.textureHandle))
			{
				physicalTexture->image = m_importedTextures[textureAlias.textureHandle];
				physicalTexture->imageView = m_importedViews[textureAlias.textureHandle];
			}

			m_physicalTextures.emplace_back(physicalTexture);
		}
		else
		{
			// Find a compatible physical texture for the alias
			for (auto& texture : m_physicalTextures)
			{
				if (texture->isCompatible(textureAlias))
				{
					physicalTexture = texture;
					break;
				}
			}

			// No compatible texture found : create a new one
			if (!physicalTexture)
			{
				physicalTexture = std::make_shared<PhysicalTexture>();

				physicalTexture->imageSettings = textureAlias.imageSettings;
				physicalTexture->image = Image::create(textureAlias.imageSettings);
				physicalTexture->imageView = physicalTexture->image->getView();

				m_physicalTextures.emplace_back(physicalTexture);
			}
		}

		// Register alias (insert ranges and barriers)
		physicalTexture->insert(textureAlias);

		// Register physical texture in TextureDatas
		m_textureDatas[textureAlias.textureHandle].physicalTexture = physicalTexture.get();
	}

	// Create barriers
	for (auto& physicalTexturePtr : m_physicalTextures)
	{
		auto& physicalTexture = *physicalTexturePtr;
		auto& barriers = physicalTexture.barriers;
		barriers.resize(m_passDatas.size());

		const auto& firstTextureData = m_textureDatas[physicalTexture.textureHandles[0]];
		auto currentPassIndex = firstTextureData.useRange.first;
		auto currentUsage = firstTextureData.usages[currentPassIndex];
		auto currentPass = m_passDatas[currentPassIndex].pass;

		for (size_t index = 0; index < physicalTexture.textureHandles.size(); index++)
		{
			const auto textureHandle = physicalTexture.textureHandles[index];
			const auto& passRange = physicalTexture.ranges[index];
			const auto& textureData = m_textureDatas[textureHandle];

			for (size_t passIndex = passRange.first; passIndex <= passRange.last; passIndex++)
			{
				const auto newPass = m_passDatas[passIndex].pass;
				const auto& newUsage = textureData.usages[passIndex];
				auto& barrier = barriers[currentPassIndex];

				if (newUsage == TextureUsage::None || passIndex == currentPassIndex)
				{
					continue;
				}

				// We want to cover W -> R / WAW / RAW
				barrier.valid = currentUsage & TextureUsage::Write || newUsage & TextureUsage::Write;

				if (!barrier.valid)
				{
					currentPassIndex = passIndex;
					currentUsage = newUsage;
					currentPass = newPass;
					continue;
				}

				if (currentUsage & TextureUsage::Output)
				{
					barrier.srcPipelineStages |= PipelineStage::ColorAttachmentOutput;
					barrier.srcMemoryAccesses |= MemoryAccess::ColorAttachmentWrite;
					barrier.srcLayout = ImageLayout::Attachment;
				}
				else if (currentUsage & TextureUsage::Depth)
				{
					barrier.srcPipelineStages |= PipelineStage::EarlyFragmentTests | PipelineStage::LateFragmentTests;
					barrier.srcMemoryAccesses |= MemoryAccess::DepthStencilAttachmentRead | MemoryAccess::DepthStencilAttachmentWrite;
					barrier.srcLayout = ImageLayout::Attachment;
				}
				else if (currentUsage & TextureUsage::Input)
				{
					barrier.srcPipelineStages |= PipelineStage::FragmentShader;
					barrier.srcLayout = ImageLayout::ShaderRead;
				}
				else if (currentUsage & TextureUsage::Sampled)
				{
					const auto shaderStages = currentPass->getSamplingStages(textureHandle);

					barrier.srcPipelineStages |= getShaderPipelineStages(shaderStages);
					barrier.srcLayout = ImageLayout::ShaderRead;
				}
				else
				{
					ATEMA_ERROR("Invalid usage");
				}

				if (newUsage & TextureUsage::Output)
				{
					barrier.dstPipelineStages |= PipelineStage::ColorAttachmentOutput;
					barrier.dstMemoryAccesses |= MemoryAccess::ColorAttachmentWrite;
					barrier.dstLayout = ImageLayout::Attachment;
				}
				else if (newUsage & TextureUsage::Depth)
				{
					barrier.dstPipelineStages |= PipelineStage::EarlyFragmentTests | PipelineStage::LateFragmentTests;
					barrier.dstMemoryAccesses |= MemoryAccess::DepthStencilAttachmentRead | MemoryAccess::DepthStencilAttachmentWrite;
					barrier.dstLayout = ImageLayout::Attachment;
				}
				else if (newUsage & TextureUsage::Input)
				{
					barrier.dstPipelineStages |= PipelineStage::FragmentShader;
					barrier.dstMemoryAccesses |= MemoryAccess::InputAttachmentRead;
					barrier.dstLayout = ImageLayout::ShaderRead;
				}
				else if (newUsage & TextureUsage::Sampled)
				{
					const auto shaderStages = newPass->getSamplingStages(textureHandle);

					barrier.dstPipelineStages |= getShaderPipelineStages(shaderStages);
					barrier.dstMemoryAccesses |= MemoryAccess::ShaderRead;
					barrier.dstLayout = ImageLayout::ShaderRead;
				}
				else
				{
					ATEMA_ERROR("Invalid usage");
				}

				// We can often use RenderPass::ExternalBarrier to synchronize accesses (better performance)
				// Here we create only RenderPass::Settings::outputBarriers, so the texture must be an attachment of the current pass
				// If the texture is sampled in the current pass, it is not an attachment so we can't use RenderPass::ExternalBarrier
				// In that case, we will need an image barrier between the different passes
				barrier.insideRenderPass = !(currentUsage & TextureUsage::Sampled);
				barrier.valid = !barrier.insideRenderPass;

				// Render passes will already set the correct final layout so no need to change it
				barrier.srcLayout = barrier.dstLayout;

				currentPassIndex = passIndex;
				currentUsage = newUsage;
				currentPass = newPass;
			}
		}
	}
}

void FrameGraphBuilder::createPhysicalPasses()
{
	m_physicalPasses.resize(m_passDatas.size());

	for (size_t passIndex = 0; passIndex < m_passDatas.size(); passIndex++)
	{
		auto& physicalPass = m_physicalPasses[passIndex];
		const auto& passData = m_passDatas[passIndex];
		const auto& pass = *passData.pass;

		physicalPass.useRenderFrameOutput = pass.useRenderFrameOutput();

		// The FrameGraph will use RenderFrame's RenderPass & Framebuffer
		if (physicalPass.useRenderFrameOutput)
		{
			physicalPass.clearValues.emplace_back(Color::Black);
			physicalPass.clearValues.emplace_back(DepthStencil());
			continue;
		}

		const auto& framebufferSize = pass.getOutputSize();

		Framebuffer::Settings framebufferSettings;
		framebufferSettings.width = framebufferSize.x;
		framebufferSettings.height = framebufferSize.y;

		RenderPass::Settings renderPassSettings;
		renderPassSettings.subpasses.resize(1);

		auto& subpass = renderPassSettings.subpasses[0];

		auto& barriers = renderPassSettings.outputBarriers;

		uint32_t attachmentIndex = 0;

		uint32_t maxAttachmentLocation = 0;

		std::unordered_map<size_t, uint32_t> attachmentRefs;

		auto addAttachment = [&](FrameGraphTextureHandle textureHandle, bool read, bool depth = false)
		{
			// Attachment shader location
			uint32_t attachmentLocation = 0;

			if (read)
				attachmentLocation = pass.getInputIndex(textureHandle);
			else if (!depth)
				attachmentLocation = pass.getOutputIndex(textureHandle);

			maxAttachmentLocation = std::max(maxAttachmentLocation, attachmentLocation);

			// Get texture infos
			const auto& textureData = m_textureDatas[textureHandle];
			const auto& texture = m_textures[textureHandle];

			// Add the barrier if requested
			const auto& barrierData = textureData.physicalTexture->barriers[passIndex];

			if (barrierData.insideRenderPass)
			{
				if (barriers.empty())
					barriers.emplace_back();

				auto& barrier = barriers[0];

				barrier.srcPipelineStages |= barrierData.srcPipelineStages;
				barrier.srcMemoryAccesses |= barrierData.srcMemoryAccesses;
				barrier.dstPipelineStages |= barrierData.dstPipelineStages;
				barrier.dstMemoryAccesses |= barrierData.dstMemoryAccesses;
			}

			// Add corresponding image to framebuffer and add the corresponding clear value
			framebufferSettings.imageViews.emplace_back(textureData.physicalTexture->imageView);

			if (Renderer::isDepthImageFormat(texture.format) || Renderer::isStencilImageFormat(texture.format))
			{
				DepthStencil depthStencil;

				const auto clearValueIt = pass.m_clearDepths.find(textureHandle);
				if (clearValueIt != pass.m_clearDepths.end())
					depthStencil = clearValueIt->second;

				physicalPass.clearValues.emplace_back(depthStencil);
			}
			else
			{
				Color color = Color::Black;

				const auto clearValueIt = pass.m_clearColors.find(textureHandle);
				if (clearValueIt != pass.m_clearColors.end())
					color = clearValueIt->second;

				physicalPass.clearValues.emplace_back(color);
			}

			// Create AttachmentDescription
			auto& attachmentDescription = renderPassSettings.attachments.emplace_back();
			attachmentDescription.format = texture.format;

			if (read)
			{
				attachmentDescription.loading = AttachmentLoading::Load;
				attachmentDescription.initialLayout = ImageLayout::ShaderRead;
			}
			else // write
			{
				if (textureData.doClear(passIndex))
				{
					attachmentDescription.loading = AttachmentLoading::Clear;
					attachmentDescription.initialLayout = ImageLayout::Undefined;
				}
				else
				{
					attachmentDescription.loading = AttachmentLoading::Load;
					attachmentDescription.initialLayout = ImageLayout::Attachment;
				}
			}

			const auto nextUse = textureData.nextUse(passIndex);
			const auto nextClear = textureData.nextClear(passIndex);
			const bool usedLater = nextUse != InvalidPassIndex;

			// Store the texture if imported or if the next use doesn't clear it
			if (textureData.imported || (usedLater && nextUse != nextClear))
			{
				attachmentDescription.storing = AttachmentStoring::Store;
			}
			// The attachment won't be used or will be cleared : we just change layout depending on next use
			else
			{
				attachmentDescription.storing = AttachmentStoring::Undefined;
			}

			// Next use will be reading
			if (usedLater && nextUse == textureData.nextRead(passIndex))
				attachmentDescription.finalLayout = ImageLayout::ShaderRead;
			// Not used later or next use will be writing
			else
				attachmentDescription.finalLayout = ImageLayout::Attachment;

			attachmentRefs[attachmentLocation] = attachmentIndex;

			attachmentIndex++;
		};

		// Add input textures
		for (auto& textureHandle : pass.getInputTextures())
		{
			addAttachment(textureHandle, true);
		}

		if (!attachmentRefs.empty())
		{
			subpass.input.resize(maxAttachmentLocation + 1, RenderPass::UnusedAttachment);

			for (const auto& [location, attachmentRef] : attachmentRefs)
			{
				subpass.input[location] = attachmentRef;
			}
		}

		// Add output textures
		maxAttachmentLocation = 0;
		attachmentRefs.clear();

		for (auto& textureHandle : pass.getOutputTextures())
		{
			addAttachment(textureHandle, false);
		}

		if (!attachmentRefs.empty())
		{
			subpass.color.resize(maxAttachmentLocation + 1, RenderPass::UnusedAttachment);

			for (const auto& [location, attachmentRef] : attachmentRefs)
			{
				subpass.color[location] = attachmentRef;
			}
		}

		// Add depth texture
		const auto depthTextureHandle = pass.getDepthTexture();

		if (depthTextureHandle != FrameGraph::InvalidTextureHandle)
		{
			addAttachment(depthTextureHandle, false, true);
			subpass.depthStencil = static_cast<uint32_t>(renderPassSettings.attachments.size()) - 1;
		}

		physicalPass.renderPass = RenderPass::create(renderPassSettings);

		framebufferSettings.renderPass = physicalPass.renderPass;

		physicalPass.framebuffer = Framebuffer::create(framebufferSettings);
	}
}
