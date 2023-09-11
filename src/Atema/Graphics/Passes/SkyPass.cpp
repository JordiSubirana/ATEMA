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

#include <Atema/Graphics/Passes/SkyPass.hpp>
#include <Atema/Graphics/FrameGraphBuilder.hpp>
#include <Atema/Graphics/FrameGraphContext.hpp>
#include <Atema/Graphics/Graphics.hpp>
#include <Atema/Graphics/Primitive.hpp>
#include <Atema/Graphics/RenderMaterial.hpp>
#include <Atema/Graphics/RenderScene.hpp>

using namespace at;

namespace
{
	constexpr uint32_t FrameSetIndex = 0;
	constexpr uint32_t SkySetIndex = 1;

	const std::string SkyBoxShaderName = "SkyPassBoxShader";
	constexpr char SkyBoxShader[] = R"(
struct FrameDataStruct
{
	mat4f ViewProjection;
}

external
{
	[set(0), binding(0)] FrameDataStruct FrameData;
	
	[set(1), binding(0)] samplerCubef SkyTexture;
}

[stage(vertex)]
input
{
	[location(0)] vec3f inPosition;
}

[stage(vertex)]
output
{
	[location(0)] vec3f outTexCoords;
}

[entry(vertex)]
void main()
{
	vec4f screenPosition = FrameData.ViewProjection * vec4f(inPosition, 1.0);
	
	// Y is the top vector in Vulkan, but it's Z in Atema
	outTexCoords = inPosition.xzy;
	
	// This way the depth will be fixed at max value
	setVertexPosition(screenPosition.xyww);
}

[stage(fragment)]
input
{
	[location(0)] vec3f inTexCoords;
}

[stage(fragment)]
output
{
	[location(0)] vec4f outColor;
}

[entry(fragment)]
void main()
{
	outColor = vec4f(sample(SkyTexture, inTexCoords).rgb, 0);
}
)";

	const std::string SkySphereShaderName = "SkyPassSphereShader";
	constexpr char SkySphereShader[] = R"(
struct FrameDataStruct
{
	mat4f ViewProjection;
}

external
{
	[set(0), binding(0)] FrameDataStruct FrameData;
	
	[set(1), binding(0)] sampler2Df SkyTexture;
}

[stage(vertex)]
input
{
	[location(0)] vec3f inPosition;
	[location(1)] vec2f inTexCoords;
}

[stage(vertex)]
output
{
	[location(0)] vec2f outTexCoords;
}

[entry(vertex)]
void main()
{
	vec4f screenPosition = FrameData.ViewProjection * vec4f(inPosition, 1.0);
	
	outTexCoords = inTexCoords;
	
	// This way the depth will be fixed at max value
	setVertexPosition(screenPosition.xyww);
}

[stage(fragment)]
input
{
	[location(0)] vec2f inTexCoords;
}

[stage(fragment)]
output
{
	[location(0)] vec4f outColor;
}

[entry(fragment)]
void main()
{
	vec3f envColor = sample(SkyTexture, inTexCoords).rgb;
	
	envColor = envColor / (envColor + vec3f(1.0));
    envColor = pow(envColor, vec3f(1.0/2.2));
	
	outColor = vec4f(envColor, 1);
}
)";

	const std::string SkySphereToBoxShaderName = "SkyPassSphereToBoxShader";
	constexpr char SkySphereToBoxShader[] = R"(
struct FrameDataStruct
{
	mat4f ViewProjection;
}

external
{
	[set(0), binding(0)] FrameDataStruct FrameData;
	
	[set(1), binding(0)] sampler2Df SkyTexture;
}

[stage(vertex)]
input
{
	[location(0)] vec3f inPosition;
}

[stage(vertex)]
output
{
	[location(0)] vec3f outTexCoords;
}

[entry(vertex)]
void main()
{
	vec4f screenPosition = FrameData.ViewProjection * vec4f(inPosition, 1.0);
	
	// Y is the top vector in Vulkan, but it's Z in Atema
	outTexCoords = inPosition.xyz;
	
	// This way the depth will be fixed at max value
	setVertexPosition(screenPosition.xyww);
}

[stage(fragment)]
input
{
	[location(0)] vec3f inTexCoords;
}

[stage(fragment)]
output
{
	[location(0)] vec4f outColor;
}

const vec2f invAtan = vec2f(0.1591, 0.3183);
vec2f SampleSphericalMap(vec3f v)
{
	vec2f uv = vec2f(atan(v.y, v.x), -asin(v.z));
	uv = uv * invAtan;
	uv = uv + 0.5;
	return uv;
}

[entry(fragment)]
void main()
{
	vec2f uv = SampleSphericalMap(normalize(inTexCoords));
	
	outColor = vec4f(sample(SkyTexture, uv).rgb, 0);
}
)";

	class SkyFrameData : public ShaderData
	{
	public:
		class Layout : public ShaderData::Layout
		{
		public:
			Layout() = delete;
			Layout(StructLayout structLayout = StructLayout::Default)
			{
				BufferLayout bufferLayout(structLayout);

				viewProjectionOffset = bufferLayout.addMatrix(BufferElementType::Float, 4, 4);

				initialize(bufferLayout);
			}

			size_t viewProjectionOffset;
		};

		SkyFrameData() = default;
		SkyFrameData(const SkyFrameData& other) = default;
		SkyFrameData(SkyFrameData&& other) noexcept = default;
		virtual ~SkyFrameData() = default;

		size_t getByteSize(StructLayout structLayout = StructLayout::Default) const noexcept override
		{
			return Layout(structLayout).getByteSize();
		}

		void copyTo(void* dstData, StructLayout structLayout = StructLayout::Default) const override
		{
			const Layout layout(structLayout);

			mapMemory<Matrix4f>(dstData, layout.viewProjectionOffset) = viewProjection;
		}

		Matrix4f viewProjection;

		SkyFrameData& operator=(const SkyFrameData& other) = default;
		SkyFrameData& operator=(SkyFrameData&& other) noexcept = default;
	};
}

SkyPass::SkyPass(RenderResourceManager& resourceManager) :
	m_resourceManager(&resourceManager)
{
	{
		ModelLoader::Settings settings(VertexFormat::create(DefaultVertexFormat::XYZ));

		m_boxMesh = Primitive::createBox(settings, 1.0f, 1.0f, 1.0f, 1, 1, 1);
	}

	{
		ModelLoader::Settings settings(VertexFormat::create(DefaultVertexFormat::XYZ_UV));

		m_sphereMesh = Primitive::createUVSphere(settings, 0.5f, 100, 100);
	}

	auto& graphics = Graphics::instance();

	if (!graphics.uberShaderExists(SkyBoxShaderName))
		graphics.setUberShader(SkyBoxShaderName, SkyBoxShader);

	if (!graphics.uberShaderExists(SkySphereShaderName))
		graphics.setUberShader(SkySphereShaderName, SkySphereShader);

	if (!graphics.uberShaderExists(SkySphereToBoxShaderName))
		graphics.setUberShader(SkySphereToBoxShaderName, SkySphereToBoxShader);

	RenderMaterial::Settings renderMaterialSettings;
	renderMaterialSettings.pipelineState.rasterization.cullMode = CullMode::Front;
	renderMaterialSettings.pipelineState.depth.test = true;
	renderMaterialSettings.pipelineState.depth.write = false;
	renderMaterialSettings.pipelineState.depth.compareOperation = CompareOperation::LessOrEqual;

	renderMaterialSettings.material = std::make_shared<Material>(graphics.getUberShader(SkyBoxShaderName));
	m_skyBoxMaterial = std::make_shared<RenderMaterial>(resourceManager, renderMaterialSettings);

	renderMaterialSettings.material = std::make_shared<Material>(graphics.getUberShader(SkySphereShaderName));
	m_skySphereMaterial = std::make_shared<RenderMaterial>(resourceManager, renderMaterialSettings);

	renderMaterialSettings.material = std::make_shared<Material>(graphics.getUberShader(SkySphereToBoxShaderName));
	m_skySphereToBoxMaterial = std::make_shared<RenderMaterial>(resourceManager, renderMaterialSettings);

	m_sampler = graphics.getSampler(Sampler::Settings(SamplerFilter::Linear));

	Buffer::Settings bufferSettings;
	bufferSettings.usages = BufferUsage::Uniform | BufferUsage::TransferDst;
	bufferSettings.byteSize = SkyFrameData().getByteSize();

	m_frameDataBuffer = m_resourceManager->createBuffer(bufferSettings);

	m_frameDataDescriptorSet = m_skyBoxMaterial->createSet(FrameSetIndex);
	m_frameDataDescriptorSet->update(0, m_frameDataBuffer->getBuffer(), m_frameDataBuffer->getOffset(), m_frameDataBuffer->getSize());
}

const char* SkyPass::getName() const noexcept
{
	return "Sky";
}

FrameGraphPass& SkyPass::addToFrameGraph(FrameGraphBuilder& frameGraphBuilder, const Settings& settings)
{
	auto& pass = frameGraphBuilder.createPass(getName());

	if (settings.outputClearValue.has_value())
		pass.addOutputTexture(settings.output, 0, settings.outputClearValue.value());
	else
		pass.addOutputTexture(settings.output, 0);

	pass.setDepthTexture(settings.gbufferDepthStencil);

	Settings settingsCopy = settings;

	pass.setExecutionCallback([this, settingsCopy](FrameGraphContext& context)
		{
			execute(context, settingsCopy);
		});

	return pass;
}

void SkyPass::updateResources(CommandBuffer& commandBuffer)
{
	const auto& camera = getRenderScene().getCamera();

	// Remove translation part from view matrix to only keep rotation and scale
	Matrix4f viewMatrix = camera.getViewMatrix();
	viewMatrix[3] = { 0.0f, 0.0f, 0.0f, 1.0f };

	SkyFrameData frameData;
	frameData.viewProjection = camera.getProjectionMatrix() * viewMatrix;

	void* data = m_resourceManager->mapBuffer(*m_frameDataBuffer);

	frameData.copyTo(data);
}

void SkyPass::execute(FrameGraphContext& context, const Settings& settings)
{
	const auto& renderScene = getRenderScene();

	if (!renderScene.isValid() || !renderScene.getSkyBox())
		return;

	//Image& skyBox = *renderScene.getSkyBox()->environmentMap;
	//Image& skyBox = *renderScene.getSkyBox()->irradianceMap;
	Image& skyBox = *renderScene.getSkyBox()->prefilteredMap;

	const auto& viewport = getRenderScene().getCamera().getViewport();
	const auto& scissor = getRenderScene().getCamera().getScissor();

	auto& commandBuffer = context.getCommandBuffer();

	commandBuffer.setViewport(viewport);

	commandBuffer.setScissor(scissor.pos, scissor.size);

	Mesh* skyMesh = nullptr;
	Ptr<DescriptorSet> skyTextureSet;
	if (skyBox.getType() == ImageType::CubeMap)
	{
		skyMesh = m_boxMesh.get();

		skyTextureSet = m_skyBoxMaterial->createSet(SkySetIndex);

		m_skyBoxMaterial->bindTo(commandBuffer);
	}
	else
	{
		/*
		skyMesh = m_boxMesh.get();

		skyTextureSet = m_skySphereToBoxMaterial->createSet(SkySetIndex);

		m_skySphereToBoxMaterial->bindTo(commandBuffer);
		//*/

		//*
		skyMesh = m_sphereMesh.get();

		skyTextureSet = m_skySphereMaterial->createSet(SkySetIndex);

		m_skySphereMaterial->bindTo(commandBuffer);
		//*/
	}

	skyTextureSet->update(0, *skyBox.getView(), *m_sampler);

	commandBuffer.bindDescriptorSet(FrameSetIndex, *m_frameDataDescriptorSet);
	commandBuffer.bindDescriptorSet(SkySetIndex, *skyTextureSet);

	const VertexBuffer& vertexBuffer = *skyMesh->getVertexBuffer();
	const IndexBuffer& indexBuffer = *skyMesh->getIndexBuffer();

	commandBuffer.bindVertexBuffer(*vertexBuffer.getBuffer(), 0);
	commandBuffer.bindIndexBuffer(*indexBuffer.getBuffer(), indexBuffer.getIndexType());

	commandBuffer.drawIndexed(static_cast<uint32_t>(indexBuffer.getSize()));

	context.destroyAfterUse(std::move(skyTextureSet));
}
