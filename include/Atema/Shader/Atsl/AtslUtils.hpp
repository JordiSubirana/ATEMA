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

#ifndef ATEMA_SHADER_ASTLUTILS_HPP
#define ATEMA_SHADER_ASTLUTILS_HPP

#include <Atema/Shader/Config.hpp>
#include <Atema/Core/NonCopyable.hpp>
#include <Atema/Shader/Ast/Enums.hpp>
#include <Atema/Shader/Ast/Type.hpp>
#include <Atema/Shader/Atsl/AtslToken.hpp>

#include <string>

namespace at::atsl
{
	ATEMA_SHADER_API char getSymbol(AtslSymbol symbol);
	ATEMA_SHADER_API AtslSymbol getSymbol(char symbol);

	ATEMA_SHADER_API const std::string& getKeyword(AtslKeyword keyword);
	ATEMA_SHADER_API AtslKeyword getKeyword(const std::string& keyword);
	ATEMA_SHADER_API bool isKeyword(const std::string& keyword);

	ATEMA_SHADER_API bool isType(const std::string& typeStr);
	ATEMA_SHADER_API Type getType(const std::string& typeStr);
	ATEMA_SHADER_API std::string getTypeStr(const Type& type);
	ATEMA_SHADER_API std::string getTypeStr(const ArrayType::ComponentType& type);

	ATEMA_SHADER_API bool isBuiltInFunction(const std::string& str);
	ATEMA_SHADER_API BuiltInFunction getBuiltInFunction(const std::string& str);
	ATEMA_SHADER_API std::string getBuiltInFunctionStr(BuiltInFunction function);

	ATEMA_SHADER_API AstShaderStage getShaderStage(const std::string& stage);
	ATEMA_SHADER_API std::string getShaderStageStr(AstShaderStage stage);

	ATEMA_SHADER_API bool isExpressionDelimiter(AtslSymbol symbol);
}

#endif
