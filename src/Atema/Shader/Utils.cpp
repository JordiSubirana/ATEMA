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

#include <Atema/Shader/Utils.hpp>
#include <Atema/Core/Error.hpp>

#include <unordered_set>
#include <string_view>

using namespace at;

namespace
{
	const std::unordered_set<std::string_view> astExtensions
	{
	};

	const std::unordered_set<std::string_view> atslExtensions
	{
		"atsl"
	};

	const std::unordered_set<std::string_view> spirvExtensions
	{
		"spv"
	};

	const std::unordered_set<std::string_view> glslExtensions
	{
		"vert", "frag", "glsl"
	};
}

ShaderLanguage at::getShaderLanguage(const std::string& extension)
{
	std::string_view ext = extension;
	ext.remove_prefix(ext.find_first_not_of("."));

	if (astExtensions.count(ext) > 0)
		return ShaderLanguage::Ast;

	if (atslExtensions.count(ext) > 0)
		return ShaderLanguage::Atsl;

	if (spirvExtensions.count(ext) > 0)
		return ShaderLanguage::SpirV;

	if (glslExtensions.count(ext) > 0)
		return ShaderLanguage::Glsl;

	ATEMA_ERROR("Unknown shader extension");

	return ShaderLanguage::Ast;
}
