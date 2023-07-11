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

#include <Atema/Graphics/MaterialParameters.hpp>
#include <Atema/Renderer/DescriptorSet.hpp>

using namespace at;

//-----
// MaterialParameter
MaterialParameter::Texture::Texture(const Ptr<Image>& image, const Ptr<Sampler>& sampler) :
	image(image),
	sampler(sampler)
{
}

MaterialParameter::Buffer::Buffer(const Ptr<at::Buffer>& buffer) :
	buffer(buffer)
{
}

MaterialParameter::MaterialParameter()
{
}

MaterialParameter::MaterialParameter(const std::string& name) :
	name(name)
{
}

MaterialParameter::MaterialParameter(const std::string& name, const Ptr<Image>& image, const Ptr<Sampler>& sampler) :
	name(name),
	value(MaterialParameter::Texture(image, sampler))
{
}

MaterialParameter::MaterialParameter(const std::string& name, const Ptr<at::Buffer>& buffer) :
	name(name),
	value(MaterialParameter::Buffer(buffer))
{
}

MaterialParameter::MaterialParameter(const MaterialParameter& other)
{
	operator=(other);
}

MaterialParameter::~MaterialParameter()
{
}

void MaterialParameter::updateDescriptorSet(DescriptorSet& descriptorSet, uint32_t bindingIndex) const
{
	if (value.is<MaterialParameter::Texture>())
	{
		auto& texture = value.get<MaterialParameter::Texture>();
		descriptorSet.update(bindingIndex, *texture.image->getView(), *texture.sampler);
	}
	else if (value.is<MaterialParameter::Buffer>())
	{
		auto& buffer = value.get<MaterialParameter::Buffer>();
		descriptorSet.update(bindingIndex, *buffer.buffer);
	}
}

MaterialParameter& MaterialParameter::operator=(const MaterialParameter& other)
{
	name = other.name;
	value = other.value;

	return *this;
}

//-----
// MaterialParameters
MaterialParameters::MaterialParameters()
{
}

MaterialParameters::~MaterialParameters()
{
}

bool MaterialParameters::exists(const std::string& parameterName) const noexcept
{
	return m_orderedParameters.find(parameterName) != m_orderedParameters.end();
}

void MaterialParameters::set(const MaterialParameter& parameter)
{
	emplace(parameter);
}

void MaterialParameters::set(MaterialParameter&& parameter)
{
	emplace(std::move(parameter));
}

MaterialParameter& MaterialParameters::get(const std::string& parameterName)
{
	return *m_orderedParameters.at(parameterName);
}

const MaterialParameter& MaterialParameters::get(const std::string& parameterName) const
{
	return *m_orderedParameters.at(parameterName);
}

const std::vector<MaterialParameter>& MaterialParameters::getParameters() const
{
	return m_parameters;
}

Hash MaterialParameters::getHash() const
{
	Hash hash = 0;

	for (const auto& kv : m_orderedParameters)
		DefaultHasher::hashCombine(hash, *kv.second);

	return hash;
}

void MaterialParameters::emplace(const MaterialParameter& parameter)
{
	const auto it = m_orderedParameters.find(parameter.name);

	if (it != m_orderedParameters.end())
	{
		*it->second = parameter;
	}
	else
	{
		m_parameters.emplace_back(parameter);
		m_orderedParameters[parameter.name] = &m_parameters.back();
	}
}

void MaterialParameters::emplace(MaterialParameter&& parameter)
{
	const auto it = m_orderedParameters.find(parameter.name);

	if (it != m_orderedParameters.end())
	{
		*it->second = std::move(parameter);
	}
	else
	{
		m_parameters.emplace_back(std::move(parameter));
		m_orderedParameters[m_parameters.back().name] = &m_parameters.back();
	}
}
