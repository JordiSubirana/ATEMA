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

#include <Atema/Graphics/RenderMaterial.hpp>
#include <Atema/Graphics/Graphics.hpp>
#include <Atema/Graphics/ShaderData.hpp>
#include <Atema/Renderer/CommandBuffer.hpp>
#include <Atema/Renderer/Utils.hpp>
#include <Atema/Graphics/Material.hpp>
#include <Atema/Renderer/RenderFrame.hpp>

#include <map>

using namespace at;

namespace
{
	struct Parameter
	{
		uint32_t binding = 0;
		const MaterialParameter* parameter = nullptr;
	};

	void updateBindings(const RenderMaterial& renderMaterial,
		const MaterialParameters& srcMaterialParameters,
		MaterialParameters& dstMaterialParameters,
		std::vector<Ptr<DescriptorSet>>& descriptorSets,
		RenderResourceManager& resourceManager)
	{
		auto& renderFrame = resourceManager.getRenderFrame();
		auto& commandBuffer = resourceManager.getCommandBuffer();

		// First of all, update resources
		for (const auto& srcMaterialParameter : srcMaterialParameters.getParameters())
		{
			// Buffer parameter
			if (srcMaterialParameter.value.is<MaterialParameter::Buffer>())
			{
				const auto& stagingBuffer = srcMaterialParameter.value.get<MaterialParameter::Buffer>().buffer;
				Buffer* buffer = nullptr;

				// Retrieve the parameter or create it if it does not exist
				if (dstMaterialParameters.exists(srcMaterialParameter.name))
				{
					buffer = dstMaterialParameters.get(srcMaterialParameter.name).value.get<MaterialParameter::Buffer>().buffer.get();
				}
				else
				{
					Buffer::Settings bufferSettings;
					bufferSettings.usages = BufferUsage::Uniform | BufferUsage::TransferDst;
					bufferSettings.byteSize = stagingBuffer->getByteSize();

					auto uniformBuffer = Buffer::create(bufferSettings);

					buffer = uniformBuffer.get();

					MaterialParameter dstMaterialParameter;
					dstMaterialParameter.name = srcMaterialParameter.name;
					dstMaterialParameter.value = MaterialParameter::Buffer(std::move(uniformBuffer));

					dstMaterialParameters.set(std::move(dstMaterialParameter));
				}

				// Copy the staging buffer to the uniform buffer
				commandBuffer.copyBuffer(*stagingBuffer, *buffer, stagingBuffer->getByteSize());
			}
			// Other parameter types don't require specific operations
			else if (srcMaterialParameter.value.is<MaterialParameter::Texture>())
			{
				// Destroy previous parameter after use, if it exists (may be used by the previous frame)
				if (dstMaterialParameters.exists(srcMaterialParameter.name))
				{
					auto& dstTextureParameter = dstMaterialParameters.get(srcMaterialParameter.name).value.get<MaterialParameter::Texture>();

					renderFrame.destroyAfterUse(std::move(dstTextureParameter.image));
					renderFrame.destroyAfterUse(std::move(dstTextureParameter.sampler));
				}

				dstMaterialParameters.set(srcMaterialParameter);
			}
			else
			{
				ATEMA_ERROR("Invalid MaterialParameter type");
			}
		}

		// Then update descriptor sets
		// Sort new parameters by set index
		std::vector<std::vector<Parameter>> setParameters;
		setParameters.resize(renderMaterial.getDescriptorSetLayoutCount());

		for (auto& materialParameter : dstMaterialParameters.getParameters())
		{
			const auto& binding = renderMaterial.getBinding(materialParameter.name);

			auto& parameter = setParameters[binding.set].emplace_back();
			parameter.binding = binding.binding;
			parameter.parameter = &materialParameter;
		}

		// Update material descriptor sets
		for (size_t set = 0; set < setParameters.size(); set++)
		{
			auto& descriptorSet = descriptorSets[set];
			auto& parameters = setParameters[set];

			if (parameters.empty())
				continue;

			if (!descriptorSet)
				descriptorSet = renderMaterial.createSet(set);

			for (auto& parameter : parameters)
			{
				const auto binding = parameter.binding;
				auto& materialParameter = *parameter.parameter;

				materialParameter.updateDescriptorSet(*descriptorSet, binding);
			}
		}
	}
}

// RenderMaterial
RenderMaterial::RenderMaterial(RenderResourceManager& resourceManager, const Settings& settings) :
	RenderResource(resourceManager),
	m_material(settings.material),
	m_id(settings.id),
	m_needUpdate(true)
{
	auto& graphics = Graphics::instance();

	m_uberShader = graphics.getUberShader(m_material->getUberShader(), settings.uberShaderOptions, settings.shaderLibraryManager);

	GraphicsPipeline::Settings pipelineSettings;
	pipelineSettings.state = settings.pipelineState;

	auto& vertexReflection = m_uberShader->getReflection(AstShaderStage::Vertex);
	auto& fragmentReflection = m_uberShader->getReflection(AstShaderStage::Fragment);

	// Override vertex input
	//TODO: Take multiple vertex buffers into account
	pipelineSettings.state.vertexInput.inputs.clear();
	for (const auto& input : vertexReflection.getInputs())
		pipelineSettings.state.vertexInput.inputs.emplace_back(0, input.location, getVertexFormat(input.type));

	// DescriptorSetLayouts

	// bindings[setIndex][bindingIndex] = DescriptorSetBinding
	std::map<uint32_t, std::map<uint32_t, DescriptorSetBinding>> bindings;

	for (const auto& external : vertexReflection.getExternals())
	{
		auto& binding = bindings[external.set][external.binding];

		binding.type = getDefaultDescriptorType(external.type);
		binding.count = getDescriptorBindingCount(external.type);
		binding.binding = external.binding;
		binding.shaderStages = ShaderStage::Vertex;

		m_bindings[external.name] = { external.set, external.binding };
	}

	for (const auto& external : fragmentReflection.getExternals())
	{
		auto& set = bindings[external.set];
		const auto it = set.find(external.binding);

		// If the binding is in the fragment shader only, just create it
		if (it == set.end())
		{
			auto& binding = set[external.binding];

			binding.type = getDefaultDescriptorType(external.type);
			binding.count = getDescriptorBindingCount(external.type);
			binding.binding = external.binding;
			binding.shaderStages = ShaderStage::Fragment;

			m_bindings[external.name] = { external.set, external.binding };
		}
		// Otherwise ensure the binding matches with the one defined in the vertex shader side
		else
		{
			auto& binding = it->second;

			if (binding.type != getDefaultDescriptorType(external.type))
				ATEMA_ERROR("DescriptorSetBinding (set " + std::to_string(external.set) + ", binding " + std::to_string(external.binding) + ") type must be the same in vertex & fragment shaders");

			if (binding.count != getDescriptorBindingCount(external.type))
				ATEMA_ERROR("DescriptorSetBinding (set " + std::to_string(external.set) + ", binding " + std::to_string(external.binding) + ") count must be the same in vertex & fragment shaders");

			binding.shaderStages |= ShaderStage::Fragment;
		}
	}

	if (!bindings.empty())
	{
		const size_t layoutSize = bindings.rbegin()->first + 1;

		pipelineSettings.descriptorSetLayouts.resize(layoutSize);
		m_descriptorSetLayouts.resize(layoutSize);
		m_descriptorSets.resize(layoutSize);
	}

	// Retrieving DescriptorSetLayouts corresponding to each set index
	// If no set is bound, the layout at this corresponding index will be empty
	for (auto& [setIndex, bindingMap] : bindings)
	{
		DescriptorSetLayout::Settings layoutSettings;

		for (auto& [bindingIndex, binding] : bindingMap)
			layoutSettings.bindings.emplace_back(std::move(binding));

		auto descriptorSetLayout = graphics.getDescriptorSetLayout(layoutSettings);

		pipelineSettings.descriptorSetLayouts[setIndex] = descriptorSetLayout;
		m_descriptorSetLayouts[setIndex] = std::move(descriptorSetLayout);
	}

	pipelineSettings.vertexShader = graphics.getShader(*graphics.getUberShader(*m_uberShader, AstShaderStage::Vertex));
	pipelineSettings.fragmentShader = graphics.getShader(*graphics.getUberShader(*m_uberShader, AstShaderStage::Fragment));

	m_pipeline = graphics.getGraphicsPipeline(pipelineSettings);

	m_connectionGuard.connect(m_material->onParameterUpdated, [this]()
		{
			m_needUpdate = true;
		});
}

RenderMaterial::~RenderMaterial()
{
	onDestroy();
}

RenderMaterial::ID RenderMaterial::getID() const noexcept
{
	return m_id;
}

void RenderMaterial::bindTo(CommandBuffer& commandBuffer) const
{
	commandBuffer.bindPipeline(*m_pipeline);

	for (size_t set = 0; set < m_descriptorSets.size(); set++)
	{
		if (m_descriptorSets[set])
			commandBuffer.bindDescriptorSet(static_cast<uint32_t>(set), *m_descriptorSets[set]);
	}
}

Ptr<RenderMaterialInstance> RenderMaterial::createInstance(const MaterialInstance& materialInstance)
{
	const auto instanceID = m_instanceIdManager.get();
	auto renderMaterialInstance = Ptr<RenderMaterialInstance>(new RenderMaterialInstance(*this, materialInstance, instanceID));

	m_connectionGuard.connect(renderMaterialInstance->onDestroy, [this, instanceID]()
		{
			m_instanceIdManager.release(instanceID);
		});

	return renderMaterialInstance;
}

bool RenderMaterial::hasBinding(const std::string& name) const
{
	return m_bindings.find(name) != m_bindings.end();
}

const RenderMaterial::Binding& RenderMaterial::getBinding(const std::string& name) const
{
	const auto it = m_bindings.find(name);

	ATEMA_ASSERT(it != m_bindings.end(), "RenderMaterial binding does not exist");

	return it->second;
}

size_t RenderMaterial::getDescriptorSetLayoutCount() const noexcept
{
	return m_descriptorSetLayouts.size();
}

Ptr<DescriptorSet> RenderMaterial::createSet(size_t setIndex) const
{
	ATEMA_ASSERT(m_descriptorSetLayouts.size() > setIndex && m_descriptorSetLayouts[setIndex], "Invalid set index");

	return m_descriptorSetLayouts[setIndex]->createSet();
}

void RenderMaterial::updateResources()
{
	if (m_needUpdate)
	{
		updateBindings(*this, m_material->getParameters(), m_parameters, m_descriptorSets, getResourceManager());

		m_needUpdate = false;
	}
}

// RenderMaterialInstance
RenderMaterialInstance::RenderMaterialInstance(const RenderMaterial& renderMaterial, const MaterialInstance& materialInstance, RenderMaterial::ID id) :
	RenderResource(renderMaterial.getResourceManager()),
	m_renderMaterial(&renderMaterial),
	m_materialInstance(&materialInstance),
	m_id(id),
	m_needUpdate(true)
{
	m_descriptorSets.resize(m_renderMaterial->getDescriptorSetLayoutCount());

	m_connectionGuard.connect(const_cast<MaterialInstance&>(materialInstance).onParameterUpdated, [this]()
		{
			m_needUpdate = true;
		});
}

RenderMaterialInstance::~RenderMaterialInstance()
{
	onDestroy();
}

RenderMaterial::ID RenderMaterialInstance::getID() const noexcept
{
	return m_id;
}

const RenderMaterial& RenderMaterialInstance::getRenderMaterial() const noexcept
{
	return *m_renderMaterial;
}

void RenderMaterialInstance::bindTo(CommandBuffer& commandBuffer) const
{
	for (size_t set = 0; set < m_descriptorSets.size(); set++)
	{
		if (m_descriptorSets[set])
			commandBuffer.bindDescriptorSet(static_cast<uint32_t>(set), *m_descriptorSets[set]);
	}
}

void RenderMaterialInstance::updateResources()
{
	if (m_needUpdate)
	{
		updateBindings(*m_renderMaterial, m_materialInstance->getParameters(), m_parameters, m_descriptorSets, getResourceManager());

		m_needUpdate = false;
	}
}
