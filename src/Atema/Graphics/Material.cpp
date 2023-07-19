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

#include <Atema/Graphics/Material.hpp>
#include <Atema/Graphics/Graphics.hpp>
#include <Atema/Graphics/ShaderData.hpp>
#include <Atema/Renderer/Utils.hpp>
#include <Atema/Shader/UberShader.hpp>

using namespace at;

namespace
{
	void setMaterialParameter(MaterialParameters& materialParameters, const std::string& name, const ShaderData& shaderData)
	{
		Ptr<Buffer> buffer;
		if (materialParameters.exists(name))
		{
			buffer = materialParameters.get(name).value.get<MaterialParameter::Buffer>().buffer;
		}
		else
		{
			Buffer::Settings bufferSettings;
			bufferSettings.usages = BufferUsage::Map | BufferUsage::TransferSrc;
			bufferSettings.byteSize = shaderData.getByteSize();

			buffer = Buffer::create(bufferSettings);

			materialParameters.set(name, buffer);
		}

		shaderData.copyTo(buffer->map());
	}

	void setMaterialParameter(MaterialParameters& materialParameters, const std::string& name, const Ptr<Image>& image, const Ptr<Sampler>& sampler)
	{
		MaterialParameter materialParameter;
		materialParameter.name = name;
		materialParameter.value = MaterialParameter::Texture(image, sampler);

		materialParameters.set(materialParameter);
	}
}

// Material
Material::Material(Ptr<UberShader> uberShader, const MaterialData& metaData) :
	m_uberShader(std::move(uberShader)),
	m_metaData(metaData)
{
	
}

Material::~Material()
{
	onDestroy();
}

const UberShader& Material::getUberShader() const noexcept
{
	return *m_uberShader;
}

const MaterialData& Material::getMetaData() const noexcept
{
	return m_metaData;
}

void Material::setParameter(const std::string& name, const ShaderData& shaderData)
{
	setMaterialParameter(m_parameters, name, shaderData);

	onParameterUpdated();
}

void Material::setParameter(const std::string& name, const Ptr<Image>& image, const Ptr<Sampler>& sampler)
{
	setMaterialParameter(m_parameters, name, image, sampler);

	onParameterUpdated();
}

const MaterialParameters& Material::getParameters() const noexcept
{
	return m_parameters;
}

// MaterialInstance
MaterialInstance::MaterialInstance(Ptr<Material> material) :
	m_material(std::move(material))
{
}

MaterialInstance::~MaterialInstance()
{
	onDestroy();
}

const Ptr<Material>& MaterialInstance::getMaterial() const noexcept
{
	return m_material;
}

void MaterialInstance::setParameter(const std::string& name, const ShaderData& shaderData)
{
	setMaterialParameter(m_parameters, name, shaderData);

	onParameterUpdated();
}

void MaterialInstance::setParameter(const std::string& name, const Ptr<Image>& image, const Ptr<Sampler>& sampler)
{
	setMaterialParameter(m_parameters, name, image, sampler);

	onParameterUpdated();
}

const MaterialParameters& MaterialInstance::getParameters() const noexcept
{
	return m_parameters;
}
