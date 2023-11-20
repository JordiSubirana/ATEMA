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

#include <Atema/Graphics/Graphics.hpp>
#include <Atema/Graphics/RenderMaterial.hpp>
#include <Atema/Graphics/RenderScene.hpp>
#include <Atema/Graphics/Passes/EquirectangularToCubemapPass.hpp>

using namespace at;

namespace
{
	constexpr uint32_t TextureSetIndex = 1;

	const std::string EquirectangularToCubemapShaderName = "EquirectangularToCubemapShader";
	constexpr char EquirectangularToCubemapShader[] = R"(
include Atema.CubemapPass;

external
{
	[set(1), binding(0)] sampler2Df Texture;
}

vec2f getEquirectangularUV(vec3f uvw)
{
	const vec2f invAtan = vec2f(0.1591, 0.3183);

	vec2f uv = vec2f(atan(uvw.x, uvw.z), -asin(uvw.y));
	uv = uv * invAtan;
	uv = uv + 0.5;
	return uv;
}

vec4f getCubemapFaceColor(int faceIndex, vec3f uvw)
{
	return sample(Texture, getEquirectangularUV(uvw));
}
)";
}

EquirectangularToCubemapPass::EquirectangularToCubemapPass(RenderResourceManager& resourceManager)
{
	// Material
	Graphics& graphics = Graphics::instance();

	if (!graphics.uberShaderExists(EquirectangularToCubemapShaderName))
		graphics.setUberShader(EquirectangularToCubemapShaderName, EquirectangularToCubemapShader);

	auto material = graphics.getMaterial(*graphics.getUberShader(EquirectangularToCubemapShaderName));

	RenderMaterial::Settings renderMaterialSettings;
	renderMaterialSettings.material = material.get();
	renderMaterialSettings.shaderLibraryManager = &ShaderLibraryManager::instance();
	renderMaterialSettings.pipelineState.depth.test = false;
	renderMaterialSettings.pipelineState.depth.write = false;
	renderMaterialSettings.pipelineState.rasterization.cullMode = CullMode::None;

	auto renderMaterial = std::make_shared<RenderMaterial>(resourceManager, renderMaterialSettings);

	initialize(std::move(material), std::move(renderMaterial));

	// Sampler
	m_sampler = graphics.getSampler(Sampler::Settings(SamplerFilter::Linear));
}

const char* EquirectangularToCubemapPass::getName() const noexcept
{
	return "EquirectangularToCubemap";
}

void EquirectangularToCubemapPass::initializeFrameGraphPass(FrameGraphPass& frameGraphPass, FrameGraphBuilder& frameGraphBuilder, const FrameGraphSettings& settings)
{
	frameGraphPass.addSampledTexture(settings.environmentMap, ShaderStage::Fragment);
}

bool EquirectangularToCubemapPass::bindResources(FrameGraphContext& context, const FrameGraphSettings& settings)
{
	if (settings.environmentMap == FrameGraph::InvalidTextureHandle)
		return false;

	Ptr<DescriptorSet> textureSet = getRenderMaterial().createSet(TextureSetIndex);

	textureSet->update(0, *context.getImageView(settings.environmentMap), *m_sampler);

	context.getCommandBuffer().bindDescriptorSet(TextureSetIndex, *textureSet);

	context.destroyAfterUse(std::move(textureSet));

	return true;
}
