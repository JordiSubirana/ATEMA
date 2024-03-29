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

#ifndef ATEMA_GLOBAL_GRAPHICS_HPP
#define ATEMA_GLOBAL_GRAPHICS_HPP

#include <Atema/Graphics/Config.hpp>
#include <Atema/Graphics/AbstractFrameRenderer.hpp>
#include <Atema/Graphics/AbstractRenderPass.hpp>
#include <Atema/Graphics/Camera.hpp>
#include <Atema/Graphics/DebugRenderer.hpp>
#include <Atema/Graphics/DefaultLightingModels.hpp>
#include <Atema/Graphics/DefaultMaterials.hpp>
#include <Atema/Graphics/DirectionalLight.hpp>
#include <Atema/Graphics/Enums.hpp>
#include <Atema/Graphics/FrameGraph.hpp>
#include <Atema/Graphics/FrameGraphBuilder.hpp>
#include <Atema/Graphics/FrameGraphContext.hpp>
#include <Atema/Graphics/FrameGraphPass.hpp>
#include <Atema/Graphics/FrameGraphTexture.hpp>
#include <Atema/Graphics/FrameRenderer.hpp>
#include <Atema/Graphics/GBuffer.hpp>
#include <Atema/Graphics/Graphics.hpp>
#include <Atema/Graphics/IndexBuffer.hpp>
#include <Atema/Graphics/Light.hpp>
#include <Atema/Graphics/LightingModel.hpp>
#include <Atema/Graphics/Loaders/DefaultImageLoader.hpp>
#include <Atema/Graphics/Loaders/ImageLoader.hpp>
#include <Atema/Graphics/Loaders/ModelLoader.hpp>
#include <Atema/Graphics/Loaders/ObjLoader.hpp>
#include <Atema/Graphics/Material.hpp>
#include <Atema/Graphics/MaterialData.hpp>
#include <Atema/Graphics/MaterialParameters.hpp>
#include <Atema/Graphics/Mesh.hpp>
#include <Atema/Graphics/Model.hpp>
#include <Atema/Graphics/OrthographicCamera.hpp>
#include <Atema/Graphics/Passes/AbstractCubemapPass.hpp>
#include <Atema/Graphics/Passes/DebugFrameGraphPass.hpp>
#include <Atema/Graphics/Passes/DebugRendererPass.hpp>
#include <Atema/Graphics/Passes/EnvironmentIrradiancePass.hpp>
#include <Atema/Graphics/Passes/EnvironmentPrefilterPass.hpp>
#include <Atema/Graphics/Passes/EquirectangularToCubemapPass.hpp>
#include <Atema/Graphics/Passes/GBufferPass.hpp>
#include <Atema/Graphics/Passes/LightPass.hpp>
#include <Atema/Graphics/Passes/ScreenPass.hpp>
#include <Atema/Graphics/Passes/ShadowPass.hpp>
#include <Atema/Graphics/Passes/SkyPass.hpp>
#include <Atema/Graphics/Passes/ToneMappingPass.hpp>
#include <Atema/Graphics/PerspectiveCamera.hpp>
#include <Atema/Graphics/Pipelines/EnvironmentPipeline.hpp>
#include <Atema/Graphics/PointLight.hpp>
#include <Atema/Graphics/Primitive.hpp>
#include <Atema/Graphics/Renderable.hpp>
#include <Atema/Graphics/RenderContext.hpp>
#include <Atema/Graphics/RenderElement.hpp>
#include <Atema/Graphics/RenderLight.hpp>
#include <Atema/Graphics/RenderMaterial.hpp>
#include <Atema/Graphics/RenderObject.hpp>
#include <Atema/Graphics/RenderResource.hpp>
#include <Atema/Graphics/RenderResourceManager.hpp>
#include <Atema/Graphics/RenderScene.hpp>
#include <Atema/Graphics/ShaderBinding.hpp>
#include <Atema/Graphics/ShaderData.hpp>
#include <Atema/Graphics/SkyBox.hpp>
#include <Atema/Graphics/SpotLight.hpp>
#include <Atema/Graphics/StaticModel.hpp>
#include <Atema/Graphics/StaticRenderModel.hpp>
#include <Atema/Graphics/VertexBuffer.hpp>
#include <Atema/Graphics/VertexFormat.hpp>
#include <Atema/Graphics/VertexTypes.hpp>

#endif