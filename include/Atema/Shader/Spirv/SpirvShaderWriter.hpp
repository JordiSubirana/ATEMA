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

#ifndef ATEMA_SHADER_SPIRVSHADERWRITER_HPP
#define ATEMA_SHADER_SPIRVSHADERWRITER_HPP

#include <Atema/Shader/Config.hpp>
#include <Atema/Shader/ShaderWriter.hpp>
#include <Atema/Shader/Glsl/GlslShaderWriter.hpp>

#include <sstream>

namespace at
{
	class ATEMA_SHADER_API SpirvShaderWriter : public GlslShaderWriter
	{
	public:
		struct Settings
		{
			std::optional<AstShaderStage> stage;
		};

		SpirvShaderWriter();
		SpirvShaderWriter(const Settings& settings);
		~SpirvShaderWriter();

		void compile(std::vector<uint32_t>& spirv);
		void compile(std::ostream& ostream);

		void visit(EntryFunctionDeclarationStatement& statement) override;
		
	private:
		void compileSpirv(std::vector<uint32_t>& spirv);

		std::optional<AstShaderStage> m_requestedStage;
		bool m_entryFound;
		AstShaderStage m_stage;
		std::stringstream m_glslStream;
	};
}

#endif
