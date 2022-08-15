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

#include <Atema/Shader/Loaders/AtslLoader.hpp>
#include <Atema/Core/Error.hpp>
#include <Atema/Shader/Atsl/AtslParser.hpp>
#include <Atema/Shader/Atsl/AtslToAstConverter.hpp>
#include <Atema/Shader/UberShader.hpp>

#include <sstream>
#include <fstream>

using namespace at;

namespace
{
	std::unordered_set<std::string_view> atslExtensions =
	{
		".atsl"
	};
}

Ptr<UberShader> AtslLoader::load(const std::filesystem::path& path)
{
	if (!isExtensionSupported(path.extension()))
		return nullptr;

	std::stringstream code;

	// Load code
	{
		std::ifstream file(path);

		if (!file.is_open())
			ATEMA_ERROR("Failed to open file '" + path.string() + "'");

		code << file.rdbuf();
	}

	// Parse code and create tokens
	AtslParser parser;

	const auto atslTokens = parser.createTokens(code.str());

	// Convert tokens to AST representation
	AtslToAstConverter converter;

	return std::make_shared<UberShader>(converter.createAst(atslTokens));
}

bool AtslLoader::isExtensionSupported(const std::filesystem::path& extension)
{
	return atslExtensions.count(extension.string()) > 0;
}
