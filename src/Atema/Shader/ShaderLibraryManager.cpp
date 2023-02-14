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

#include <Atema/Shader/ShaderLibraryManager.hpp>
#include <Atema/Shader/Ast/AstCloner.hpp>

using namespace at;

ShaderLibraryManager::ShaderLibraryManager() :
	NonCopyable()
{
}

ShaderLibraryManager& ShaderLibraryManager::instance()
{
	static ShaderLibraryManager s_instance;

	return s_instance;
}

void ShaderLibraryManager::setLibrary(const std::string& libraryName, const UPtr<SequenceStatement>& libraryAST)
{
	AstCloner cloner;

	auto ast = cloner.clone(libraryAST);

	setLibrary(libraryName, std::move(ast));
}

void ShaderLibraryManager::setLibrary(const std::string& libraryName, UPtr<SequenceStatement>&& libraryAST)
{
	m_libraries.emplace(libraryName, std::move(libraryAST));
}

bool ShaderLibraryManager::hasLibrary(const std::string& libraryName) const
{
	return m_libraries.find(libraryName) != m_libraries.end();
}

const UPtr<SequenceStatement>& ShaderLibraryManager::getLibrary(const std::string& libraryName) const
{
	ATEMA_ASSERT(hasLibrary(libraryName), "Library not found");
	
	return m_libraries.at(libraryName);
}
