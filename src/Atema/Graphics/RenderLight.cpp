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

#include <Atema/Graphics/RenderLight.hpp>
#include <Atema/Graphics/DirectionalLight.hpp>
#include <Atema/Graphics/Graphics.hpp>
#include <Atema/Graphics/ShaderData.hpp>

using namespace at;

RenderLight::RenderLight(RenderResourceManager& resourceManager, const Light& light) :
	RenderResource(resourceManager),
	m_light(&light),
	m_updateShadowMapDescriptor(false),
	m_lightDataValid(false),
	m_updateShadowData(false)
{
	Buffer::Settings bufferSettings;
	bufferSettings.usages = BufferUsage::Uniform | BufferUsage::TransferDst;
	bufferSettings.byteSize = LightData::getLayout().getByteSize();

	m_lightBuffer = getResourceManager().createBuffer(bufferSettings);

	m_lightDescriptorSet = Graphics::instance().getLightLayout()->createSet();
	m_lightDescriptorSet->update(0, m_lightBuffer->getBuffer(), m_lightBuffer->getOffset(), m_lightBuffer->getSize());

	if (light.castShadows())
		updateShadowResources();

	//TODO: Replace the const_cast. A Signal doesn't represent an object's state so it could be connected even if it's const
	m_connectionGuard.connect(const_cast<Signal<>&>(light.onLightDataUpdated), [this]()
		{
			m_lightDataValid = false;
		});

	m_connectionGuard.connect(const_cast<Signal<>&>(light.onShadowMapDataUpdated), [this]()
		{
			updateShadowMap();
		});
}

void RenderLight::setShadowData(const std::vector<ShadowData>& cascades)
{
	m_shadowData.cascades = cascades;

	m_updateShadowData = true;
}

void RenderLight::updateResources()
{
	if (!m_lightDataValid)
	{
		void* data = getResourceManager().mapBuffer(*m_lightBuffer);

		LightData lightData;
		lightData.light = m_light;

		lightData.copyTo(data);

		m_lightDataValid = true;
	}

	// New shadow map couldn't be used last frame because it's part of the FrameGraph, use it from now
	if (m_updateShadowMapDescriptor)
	{
		// Ensure shadow resources exist
		updateShadowResources();

		updateShadowDescriptorSet();

		m_updateShadowMapDescriptor = false;
	}

	// Update shadow data
	if (m_updateShadowData)
	{
		void* data = getResourceManager().mapBuffer(*m_shadowBuffer);

		m_shadowData.copyTo(data);

		m_updateShadowData = false;
	}
}

const Light& RenderLight::getLight() const noexcept
{
	return *m_light;
}

const DescriptorSet& RenderLight::getLightDescriptorSet() const noexcept
{
	return *m_lightDescriptorSet;
}

const DescriptorSet& RenderLight::getShadowDescriptorSet() const noexcept
{
	return *m_shadowDescriptorSet;
}

const Ptr<Image>& RenderLight::getShadowMap() const noexcept
{
	return m_shadowMap;
}

bool RenderLight::needShadowMapUpdate() const
{
	if (!m_shadowMap)
		return true;

	if (m_shadowMap->getSize().x != m_light->getShadowMapSize())
		return true;

	if (m_shadowMap->getLayers() != m_light->getShadowCascadeCount())
		return true;

	if (m_shadowMap->getFormat() != m_light->getShadowMapFormat())
		return true;

	return false;
}

void RenderLight::updateShadowResources()
{
	if (m_shadowBuffer)
		return;

	auto& graphics = Graphics::instance();

	Sampler::Settings samplerSettings(SamplerFilter::Linear);
	samplerSettings.addressModeU = SamplerAddressMode::ClampToBorder;
	samplerSettings.addressModeV = SamplerAddressMode::ClampToBorder;
	samplerSettings.addressModeW = SamplerAddressMode::ClampToBorder;
	samplerSettings.borderColor = SamplerBorderColor::WhiteFloat;
	samplerSettings.enableCompare = true;
	samplerSettings.compareOperation = CompareOperation::Less;

	m_sampler = graphics.getSampler(samplerSettings);

	Buffer::Settings bufferSettings;
	bufferSettings.usages = BufferUsage::Uniform | BufferUsage::TransferDst;
	bufferSettings.byteSize = CascadedShadowData::getLayout().getByteSize();

	m_shadowBuffer = getResourceManager().createBuffer(bufferSettings);

	updateShadowMap();

	updateShadowDescriptorSet();
}

void RenderLight::updateShadowMap()
{
	if (m_light->castShadows())
	{
		if (!needShadowMapUpdate())
			return;

		if (m_shadowMap)
			destroyAfterUse(std::move(m_shadowMap));

		Image::Settings imageSettings;
		imageSettings.usages = ImageUsage::RenderTarget | ImageUsage::ShaderSampling;
		imageSettings.format = m_light->getShadowMapFormat();
		imageSettings.width = m_light->getShadowMapSize();
		imageSettings.height = m_light->getShadowMapSize();
		imageSettings.layers = static_cast<uint32_t>(m_light->getShadowCascadeCount());

		m_shadowMap = Image::create(imageSettings);

		m_updateShadowMapDescriptor = true;

		onShadowMapUpdated();
	}
	else if (m_shadowMap)
	{
		destroyAfterUse(std::move(m_shadowDescriptorSet));
		destroyAfterUse(std::move(m_shadowBuffer));
		destroyAfterUse(std::move(m_shadowMap));
		destroyAfterUse(std::move(m_sampler));

		m_updateShadowMapDescriptor = false;

		onShadowMapUpdated();
	}
}

void RenderLight::updateShadowDescriptorSet()
{
	if (m_shadowDescriptorSet)
		destroyAfterUse(std::move(m_shadowDescriptorSet));

	m_shadowDescriptorSet = Graphics::instance().getLightShadowLayout()->createSet();
	m_shadowDescriptorSet->update(0, m_shadowBuffer->getBuffer(), m_shadowBuffer->getOffset(), m_shadowBuffer->getSize());
	m_shadowDescriptorSet->update(1, *m_shadowMap->getView(), *m_sampler);
}
