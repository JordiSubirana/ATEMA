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

#include <Atema/Graphics/MaterialData.hpp>

using namespace at;

MaterialData::Texture::Texture(const std::filesystem::path& path) :
	path(path)
{
}

MaterialData::Parameter::Parameter(const std::string& name, const Value& value) :
	name(name),
	value(value)
{

}

MaterialData::Parameter::Parameter(const Parameter& other) :
	name(other.name),
	value(other.value)
{
}

void MaterialData::set(const Parameter& parameter)
{
	const auto it = m_indices.find(parameter.name);

	if (it != m_indices.end())
	{
		m_parameters[it->second] = parameter;
	}
	else
	{
		m_indices[parameter.name] = m_parameters.size();
		m_parameters.emplace_back(parameter);
	}
}

void MaterialData::set(const std::string& name, const Value& value)
{
	set(Parameter(name, value));
}

bool MaterialData::exists(const std::string& name) const noexcept
{
	return m_indices.find(name) != m_indices.end();
}

MaterialData::Value& MaterialData::getValue(const std::string& name)
{
	const auto it = m_indices.find(name);

	ATEMA_ASSERT(it != m_indices.end(), "Material parameter does not exist");

	return m_parameters[it->second].value;
}

const MaterialData::Value& MaterialData::getValue(const std::string& name) const
{
	const auto it = m_indices.find(name);

	ATEMA_ASSERT(it != m_indices.end(), "Material parameter does not exist");

	return m_parameters[it->second].value;
}

const std::vector<MaterialData::Parameter>& MaterialData::getParameters() const noexcept
{
	return m_parameters;
}
