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

#ifndef ATEMA_GLOBAL_SHADER_HPP
#define ATEMA_GLOBAL_SHADER_HPP

#include <Atema/Shader/Config.hpp>
#include <Atema/Shader/ShaderWriter.hpp>
#include <Atema/Shader/Ast/AstCloner.hpp>
#include <Atema/Shader/Ast/AstRecursiveVisitor.hpp>
#include <Atema/Shader/Ast/AstStageExtractor.hpp>
#include <Atema/Shader/Ast/AstUtils.hpp>
#include <Atema/Shader/Ast/AstVisitor.hpp>
#include <Atema/Shader/Ast/Constant.hpp>
#include <Atema/Shader/Ast/Enums.hpp>
#include <Atema/Shader/Ast/Expression.hpp>
#include <Atema/Shader/Ast/Statement.hpp>
#include <Atema/Shader/Ast/Type.hpp>
#include <Atema/Shader/Atsl/AtslParser.hpp>
#include <Atema/Shader/Atsl/AtslShaderWriter.hpp>
#include <Atema/Shader/Atsl/AtslToAstConverter.hpp>
#include <Atema/Shader/Atsl/AtslToken.hpp>
#include <Atema/Shader/Atsl/AtslUtils.hpp>
#include <Atema/Shader/Glsl/GlslShaderWriter.hpp>
#include <Atema/Shader/Spirv/SpirvShaderWriter.hpp>

#endif