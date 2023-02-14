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

#ifndef ATEMA_SHADER_SHADERLIBRARYMANAGER_HPP
#define ATEMA_SHADER_SHADERLIBRARYMANAGER_HPP

#include <Atema/Shader/Config.hpp>
#include <Atema/Core/NonCopyable.hpp>
#include <Atema/Core/Pointer.hpp>
#include <Atema/Shader/Ast/Statement.hpp>

#include <string>
#include <unordered_map>

namespace at
{
	class ATEMA_SHADER_API ShaderLibraryManager : public NonCopyable
	{
	public:
		ShaderLibraryManager();
		ShaderLibraryManager(ShaderLibraryManager&& other) noexcept = default;
		~ShaderLibraryManager() = default;

		// Default library manager
		static ShaderLibraryManager& instance();

		// Copy the AST an consider it as a library
		void setLibrary(const std::string& libraryName, const UPtr<SequenceStatement>& libraryAST);
		void setLibrary(const std::string& libraryName, UPtr<SequenceStatement>&& libraryAST);

		bool hasLibrary(const std::string& libraryName) const;

		const UPtr<SequenceStatement>& getLibrary(const std::string& libraryName) const;

	private:
		std::unordered_map<std::string, UPtr<SequenceStatement>> m_libraries;
	};
}

#endif