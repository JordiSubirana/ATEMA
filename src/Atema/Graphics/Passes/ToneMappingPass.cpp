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

#include <Atema/Graphics/FrameGraphBuilder.hpp>
#include <Atema/Graphics/FrameGraphContext.hpp>
#include <Atema/Graphics/Graphics.hpp>
#include <Atema/Graphics/RenderMaterial.hpp>
#include <Atema/Graphics/ShaderData.hpp>
#include <Atema/Graphics/VertexBuffer.hpp>
#include <Atema/Graphics/Passes/ToneMappingPass.hpp>
#include <Atema/Renderer/Viewport.hpp>

using namespace at;

namespace
{
	constexpr uint32_t ToneMappingSetIndex = 0;
	constexpr uint32_t TextureInputSetIndex = 1;

	const std::string ToneMappingShaderName = "ToneMappingShader";
	constexpr char ToneMappingShader[] = R"(
include Atema.PostProcess;

struct ToneMappingDataStruct
{
	float Exposure;
	float Gamma;
}

external
{
	[set(0), binding(0)] ToneMappingDataStruct ToneMappingData;
	[set(1), binding(0)] sampler2Df SourceTexture;
}

// Unreal engine tonemapping approximation

vec3f aces(vec3f color)
{
	color = color * 0.6f;
	float a = 2.51f;
	float b = 0.03f;
	float c = 2.43f;
	float d = 0.59f;
	float e = 0.14f;
	return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0f, 1.0f);
}

vec4f getPostProcessColor(vec2f uv)
{
	float exposure = ToneMappingData.Exposure;
	float gamma = ToneMappingData.Gamma;

	vec3f color = texture(SourceTexture, uv).rgb;

	// Tone mapping
	//color = (color * exposure) / (1.0 + (color / exposure));
	color = aces(color);
	//color = vec3f(1.0f) - exp(-color * exposure);
	
	// Gamma correction
	color = pow(color, vec3f(1.0/gamma));
	
	return vec4f(color, 1.0);
}
)";

	class ToneMappingData : public ShaderData
	{
	public:
		class Layout : public ShaderData::Layout
		{
		public:
			Layout() = delete;
			Layout(StructLayout structLayout = StructLayout::Default)
			{
				BufferLayout bufferLayout(structLayout);

				exposureOffset = bufferLayout.add(BufferElementType::Float);
				gammaOffset = bufferLayout.add(BufferElementType::Float);

				initialize(bufferLayout);
			}

			size_t exposureOffset;
			size_t gammaOffset;
		};

		ToneMappingData() = default;
		ToneMappingData(const ToneMappingData& other) = default;
		ToneMappingData(ToneMappingData&& other) noexcept = default;
		virtual ~ToneMappingData() = default;

		size_t getByteSize(StructLayout structLayout = StructLayout::Default) const noexcept override
		{
			return Layout(structLayout).getByteSize();
		}

		void copyTo(void* dstData, StructLayout structLayout = StructLayout::Default) const override
		{
			const Layout layout(structLayout);

			mapMemory<float>(dstData, layout.exposureOffset) = exposure;
			mapMemory<float>(dstData, layout.gammaOffset) = gamma;
		}

		float exposure = 1.0f;
		float gamma = 2.2f;

		ToneMappingData& operator=(const ToneMappingData& other) = default;
		ToneMappingData& operator=(ToneMappingData&& other) noexcept = default;
	};
}

ToneMappingPass::ToneMappingPass(RenderResourceManager& resourceManager) :
	m_resourceManager(&resourceManager),
	m_updateResources(true),
	m_exposure(1.0f),
	m_gamma(2.2f)
{
	// Material
	Graphics& graphics = Graphics::instance();

	if (!graphics.uberShaderExists(ToneMappingShaderName))
		graphics.setUberShader(ToneMappingShaderName, ToneMappingShader);

	m_material = graphics.getMaterial(*graphics.getUberShader(ToneMappingShaderName));

	RenderMaterial::Settings renderMaterialSettings;
	renderMaterialSettings.material = m_material.get();
	renderMaterialSettings.shaderLibraryManager = &ShaderLibraryManager::instance();
	renderMaterialSettings.pipelineState.depth.test = false;
	renderMaterialSettings.pipelineState.depth.write = false;
	renderMaterialSettings.pipelineState.rasterization.cullMode = CullMode::None;

	m_renderMaterial = std::make_shared<RenderMaterial>(resourceManager, renderMaterialSettings);

	// Buffer
	Buffer::Settings bufferSettings;
	bufferSettings.usages = BufferUsage::Uniform | BufferUsage::TransferDst;
	bufferSettings.byteSize = ToneMappingData().getByteSize();

	m_toneMappingBuffer = resourceManager.createBuffer(bufferSettings);

	m_toneMappingSet = m_renderMaterial->createSet(ToneMappingSetIndex);
	m_toneMappingSet->update(0, m_toneMappingBuffer->getBuffer(), m_toneMappingBuffer->getOffset(), m_toneMappingBuffer->getSize());

	// Sampler
	m_sampler = graphics.getSampler(Sampler::Settings(SamplerFilter::Linear));

	// Quad mesh
	m_quadMesh = graphics.getQuadMesh();
}

const char* ToneMappingPass::getName() const noexcept
{
	return "ToneMapping";
}

void ToneMappingPass::setExposure(float exposure) noexcept
{
	m_exposure = exposure;

	m_updateResources = true;
}

float ToneMappingPass::getExposure() const noexcept
{
	return m_exposure;
}

void ToneMappingPass::setGamma(float gamma) noexcept
{
	m_gamma = gamma;

	m_updateResources = true;
}

float ToneMappingPass::getGamma() const noexcept
{
	return m_gamma;
}

void ToneMappingPass::updateResources(CommandBuffer& commandBuffer)
{
	if (m_updateResources)
	{
		void* data = m_resourceManager->mapBuffer(*m_toneMappingBuffer);

		ToneMappingData uniformData;
		uniformData.exposure = m_exposure;
		uniformData.gamma = m_gamma;

		uniformData.copyTo(data);

		m_updateResources = false;
	}
}

FrameGraphPass& ToneMappingPass::addToFrameGraph(FrameGraphBuilder& frameGraphBuilder, const Settings& settings)
{
	auto& pass = frameGraphBuilder.createPass(getName());

	if (settings.outputClearColor.has_value())
		pass.addOutputTexture(settings.output, 0, settings.outputClearColor.value());
	else
		pass.addOutputTexture(settings.output, 0);

	pass.addSampledTexture(settings.input, ShaderStage::Fragment);

	Settings settingsCopy = settings;

	pass.setExecutionCallback([this, settingsCopy](FrameGraphContext& context)
		{
			execute(context, settingsCopy);
		});

	return pass;
}

void ToneMappingPass::execute(FrameGraphContext& context, const Settings& settings)
{
	// Output image size
	Vector2u imageSize = context.getTexture(settings.output)->getSize();
	const uint32_t mipLevel = context.getImageView(settings.output)->getBaseMipLevel();
	imageSize.x /= 1 << mipLevel;
	imageSize.y /= 1 << mipLevel;

	// Temporary descriptor set for source texture
	auto sourceTextureSet = m_renderMaterial->createSet(TextureInputSetIndex);
	sourceTextureSet->update(0, *context.getImageView(settings.input), *m_sampler);

	// Commands
	auto& commandBuffer = context.getCommandBuffer();

	Viewport viewport;
	viewport.size = imageSize;

	commandBuffer.setViewport(viewport);

	commandBuffer.setScissor(Vector2i(0, 0), imageSize);

	m_renderMaterial->bindTo(commandBuffer);

	commandBuffer.bindDescriptorSet(ToneMappingSetIndex, *m_toneMappingSet);
	commandBuffer.bindDescriptorSet(TextureInputSetIndex, *sourceTextureSet);

	commandBuffer.bindVertexBuffer(*m_quadMesh->getBuffer(), 0);

	commandBuffer.draw(static_cast<uint32_t>(m_quadMesh->getSize()));

	context.destroyAfterUse(std::move(sourceTextureSet));
}
