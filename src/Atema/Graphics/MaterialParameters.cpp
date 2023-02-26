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

using namespace at;

//-----
// MaterialParameter
MaterialParameter::Texture::Texture(const Texture& other)
{
	operator=(other);
}

MaterialParameter::Texture::Texture(Texture&& other) noexcept
{
	operator=(std::move(other));
}

MaterialParameter::Texture::Texture(const Ptr<Image>& image, const Ptr<Sampler>& sampler) :
	image(image),
	sampler(sampler)
{
}

MaterialParameter::Texture& MaterialParameter::Texture::operator=(const Texture& other)
{
	image = other.image;
	sampler = other.sampler;

	return *this;
}

MaterialParameter::Texture& MaterialParameter::Texture::operator=(Texture&& other) noexcept
{
	image = std::move(other.image);
	sampler = std::move(other.sampler);

	return *this;
}

MaterialParameter::Buffer::Buffer(const Buffer& other)
{
	operator=(other);
}

MaterialParameter::Buffer::Buffer(Buffer&& other) noexcept
{
	operator=(std::move(other));
}

MaterialParameter::Buffer::Buffer(const Ptr<at::Buffer>& buffer) :
	buffer(buffer)
{
}

MaterialParameter::Buffer& MaterialParameter::Buffer::operator=(const Buffer& other)
{
	buffer = other.buffer;

	return *this;
}

MaterialParameter::Buffer& MaterialParameter::Buffer::operator=(Buffer&& other) noexcept
{
	buffer = std::move(other.buffer);

	return *this;
}

MaterialParameter::MaterialParameter()
{
}

MaterialParameter::MaterialParameter(const MaterialParameter& other)
{
	operator=(other);
}

MaterialParameter::MaterialParameter(MaterialParameter&& other) noexcept
{
	operator=(std::move(other));
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

MaterialParameter::~MaterialParameter()
{
}

MaterialParameter& MaterialParameter::operator=(const MaterialParameter& other)
{
	name = other.name;
	value = other.value;

	return *this;
}

MaterialParameter& MaterialParameter::operator=(MaterialParameter&& other) noexcept
{
	name = std::move(other.name);
	value = std::move(other.value);

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

void MaterialParameters::set(const MaterialParameter& parameter)
{
	emplace(parameter);
}

void MaterialParameters::set(MaterialParameter&& parameter)
{
	emplace(std::move(parameter));
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
