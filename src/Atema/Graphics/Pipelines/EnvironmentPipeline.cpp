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

#include <Atema/Graphics/Pipelines/EnvironmentPipeline.hpp>
#include <Atema/Graphics/FrameGraphBuilder.hpp>
#include <Atema/Graphics/Passes/EnvironmentIrradiancePass.hpp>
#include <Atema/Graphics/Passes/EnvironmentPrefilterPass.hpp>
#include <Atema/Graphics/Passes/EquirectangularToCubemapPass.hpp>

using namespace at;

EnvironmentPipeline::EnvironmentPipeline() :
	m_updateFrameGraph(true)
{
	m_equirectangularToCubemapPass = std::make_unique<EquirectangularToCubemapPass>(m_resourceManager);
	m_irradiancePass = std::make_unique<EnvironmentIrradiancePass>(m_resourceManager);
}

void EnvironmentPipeline::setInput(const Ptr<Image>& environmentMap)
{
	ATEMA_ASSERT(environmentMap, "Invalid environment map");
	ATEMA_ASSERT(environmentMap->getType() == ImageType::Image2D || environmentMap->getType() == ImageType::CubeMap, "Environment map must be an Image2D or a CubeMap");

	m_inEnvironmentMap = environmentMap;

	m_updateFrameGraph = true;
}

void EnvironmentPipeline::setOutput(const Ptr<Image>& environmentMap, const Ptr<Image>& irradianceMap, const Ptr<Image>& prefilteredMap)
{
	ATEMA_ASSERT(!environmentMap || environmentMap->getType() == ImageType::CubeMap, "Environment map must be a CubeMap");
	ATEMA_ASSERT(!irradianceMap || irradianceMap->getType() == ImageType::CubeMap, "Irradiance map must be a CubeMap");
	ATEMA_ASSERT(!prefilteredMap || prefilteredMap->getType() == ImageType::CubeMap, "Prefiltered map must be a CubeMap");

	m_outEnvironmentMap = environmentMap;
	m_outIrradianceMap = irradianceMap;
	m_outPrefilteredMap = prefilteredMap;

	m_updateFrameGraph = true;
}

void EnvironmentPipeline::createFrameGraph()
{
	m_oldResources.emplace_back(std::move(m_environmentFrameGraph));
	m_oldResources.emplace_back(std::move(m_processFrameGraph));

	m_activePasses.clear();

	ATEMA_ASSERT(m_inEnvironmentMap, "Invalid environment map");

	Ptr<Image> environmentMap;

	// Environment FrameGraph : generate a cubemap from an equirectangular map if needed
	if (m_inEnvironmentMap->getType() == ImageType::CubeMap)
	{
		environmentMap = m_inEnvironmentMap;
	}
	else
	{
		if (!m_outEnvironmentMap)
		{
			Image::Settings imageSettings;
			imageSettings.width = std::min(m_inEnvironmentMap->getSize().x, m_inEnvironmentMap->getSize().y);
			imageSettings.height = imageSettings.width;
			imageSettings.format = m_inEnvironmentMap->getFormat();
			imageSettings.usages = ImageUsage::RenderTarget;
			imageSettings.mipLevels = 1;

			m_outEnvironmentMap = Image::create(imageSettings);
		}

		environmentMap = m_outEnvironmentMap;

		FrameGraphBuilder frameGraphBuilder;

		EquirectangularToCubemapPass::FrameGraphSettings passSettings;
		passSettings.clearColor = Color::Black;
		passSettings.environmentMap = frameGraphBuilder.importTexture(m_inEnvironmentMap);

		auto& outputTextures = passSettings.outputCubemapFaces;
		outputTextures.emplace_back(frameGraphBuilder.importTexture(m_outEnvironmentMap, CubemapFace::Top));
		outputTextures.emplace_back(frameGraphBuilder.importTexture(m_outEnvironmentMap, CubemapFace::Left));
		outputTextures.emplace_back(frameGraphBuilder.importTexture(m_outEnvironmentMap, CubemapFace::Front));
		outputTextures.emplace_back(frameGraphBuilder.importTexture(m_outEnvironmentMap, CubemapFace::Right));
		outputTextures.emplace_back(frameGraphBuilder.importTexture(m_outEnvironmentMap, CubemapFace::Back));
		outputTextures.emplace_back(frameGraphBuilder.importTexture(m_outEnvironmentMap, CubemapFace::Bottom));

		m_equirectangularToCubemapPass->addToFrameGraph(frameGraphBuilder, passSettings);
		m_activePasses.emplace_back(m_equirectangularToCubemapPass.get());

		m_environmentFrameGraph = frameGraphBuilder.build();
	}

	// Process FrameGraph : generate all maps (irradiance / prefiltered)
	if (m_outIrradianceMap || m_outPrefilteredMap)
	{
		FrameGraphBuilder frameGraphBuilder;

		// Generate irradiance map
		if (m_outIrradianceMap)
		{
			EnvironmentIrradiancePass::FrameGraphSettings passSettings;
			passSettings.clearColor = Color::Black;

			auto& outputTextures = passSettings.outputCubemapFaces;
			outputTextures.emplace_back(frameGraphBuilder.importTexture(m_outIrradianceMap, CubemapFace::Top));
			outputTextures.emplace_back(frameGraphBuilder.importTexture(m_outIrradianceMap, CubemapFace::Left));
			outputTextures.emplace_back(frameGraphBuilder.importTexture(m_outIrradianceMap, CubemapFace::Front));
			outputTextures.emplace_back(frameGraphBuilder.importTexture(m_outIrradianceMap, CubemapFace::Right));
			outputTextures.emplace_back(frameGraphBuilder.importTexture(m_outIrradianceMap, CubemapFace::Back));
			outputTextures.emplace_back(frameGraphBuilder.importTexture(m_outIrradianceMap, CubemapFace::Bottom));

			m_irradiancePass->setEnvironmentMap(*environmentMap);

			m_irradiancePass->addToFrameGraph(frameGraphBuilder, passSettings);
			m_activePasses.emplace_back(m_irradiancePass.get());
		}

		// Generate prefiltered maps
		if (m_outPrefilteredMap)
		{
			uint32_t mipLevel = 0;

			for (auto& prefilterPass : m_prefilterPasses)
			{
				EnvironmentPrefilterPass::FrameGraphSettings passSettings;
				passSettings.clearColor = Color::Black;

				auto& outputTextures = passSettings.outputCubemapFaces;
				outputTextures.emplace_back(frameGraphBuilder.importTexture(m_outPrefilteredMap, CubemapFace::Top, mipLevel));
				outputTextures.emplace_back(frameGraphBuilder.importTexture(m_outPrefilteredMap, CubemapFace::Left, mipLevel));
				outputTextures.emplace_back(frameGraphBuilder.importTexture(m_outPrefilteredMap, CubemapFace::Front, mipLevel));
				outputTextures.emplace_back(frameGraphBuilder.importTexture(m_outPrefilteredMap, CubemapFace::Right, mipLevel));
				outputTextures.emplace_back(frameGraphBuilder.importTexture(m_outPrefilteredMap, CubemapFace::Back, mipLevel));
				outputTextures.emplace_back(frameGraphBuilder.importTexture(m_outPrefilteredMap, CubemapFace::Bottom, mipLevel));

				prefilterPass->setEnvironmentMap(*environmentMap);

				prefilterPass->addToFrameGraph(frameGraphBuilder, passSettings);
				m_activePasses.emplace_back(prefilterPass.get());

				mipLevel++;
			}
		}

		m_processFrameGraph = frameGraphBuilder.build();
	}
}

void EnvironmentPipeline::beginRender()
{
	const uint32_t prefilteredMipLevels = m_outPrefilteredMap->getMipLevels();

	// Ensure we have enough prefilter passes
	if (m_prefilterPasses.size() < prefilteredMipLevels)
	{
		const size_t count = static_cast<size_t>(prefilteredMipLevels) - m_prefilterPasses.size();
		
		for (size_t i = 0; i < count; i++)
			m_prefilterPasses.emplace_back(std::make_unique<EnvironmentPrefilterPass>(m_resourceManager));

		m_updateFrameGraph = true;
	}

	const float roughnessStep = prefilteredMipLevels == 1 ? 1.0f : 1.0f / static_cast<float>(prefilteredMipLevels - 1);

	for (uint32_t i = 0; i < prefilteredMipLevels; i++)
		m_prefilterPasses[i]->setRoughness(roughnessStep * static_cast<float>(i));

	if (m_updateFrameGraph)
	{
		createFrameGraph();

		m_updateFrameGraph = false;
	}
}

void EnvironmentPipeline::updateResources(CommandBuffer& commandBuffer, RenderContext& renderContext)
{
	m_resourceManager.beginTransfer(commandBuffer, renderContext);

	m_equirectangularToCubemapPass->updateResources(commandBuffer);

	m_irradiancePass->updateResources(commandBuffer);

	for (auto& prefilteredPass : m_prefilterPasses)
		prefilteredPass->updateResources(commandBuffer);

	m_resourceManager.endTransfer();
}

void EnvironmentPipeline::render(CommandBuffer& commandBuffer, RenderContext& renderContext)
{
	if (m_environmentFrameGraph)
	{
		m_environmentFrameGraph->execute(commandBuffer, renderContext);

		commandBuffer.imageBarrier(*m_outEnvironmentMap,
			PipelineStage::ColorAttachmentOutput, PipelineStage::Transfer,
			MemoryAccess::ColorAttachmentWrite, MemoryAccess::TransferRead,
			ImageLayout::Attachment, ImageLayout::TransferDst,
			0, 0, 0, 1);

		commandBuffer.imageBarrier(*m_outEnvironmentMap,
			PipelineStage::ColorAttachmentOutput, PipelineStage::Transfer,
			MemoryAccess::ColorAttachmentWrite, MemoryAccess::TransferRead,
			ImageLayout::Undefined, ImageLayout::TransferDst,
			0, 0, 1, 0);

		commandBuffer.createMipmaps(*m_outEnvironmentMap, PipelineStage::FragmentShader, MemoryAccess::ShaderRead, ImageLayout::ShaderRead);

		commandBuffer.imageBarrier(*m_outEnvironmentMap,
			PipelineStage::Transfer, PipelineStage::FragmentShader,
			MemoryAccess::TransferWrite, MemoryAccess::ShaderRead,
			ImageLayout::ShaderRead, ImageLayout::ShaderRead);
	}

	if (m_processFrameGraph)
	{
		m_processFrameGraph->execute(commandBuffer, renderContext);
	}
}
