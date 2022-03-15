/*
	Copyright 2021 Jordi SUBIRANA

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

#include <Atema/Shader/Ast/Statement.hpp>
#include <Atema/Shader/Ast/Expression.hpp>

using namespace at;

Statement::Statement()
{
}

Statement::~Statement()
{
}

Statement::Type ConditionalStatement::getType() const noexcept
{
	return Type::Conditional;
}

Statement::Type VariableDeclarationStatement::getType() const noexcept
{
	return Type::VariableDeclaration;
}

Statement::Type StructDeclarationStatement::getType() const noexcept
{
	return Type::StructDeclaration;
}

Statement::Type InputDeclarationStatement::getType() const noexcept
{
	return Type::InputDeclaration;
}

Statement::Type OutputDeclarationStatement::getType() const noexcept
{
	return Type::OutputDeclaration;
}

Statement::Type ExternalDeclarationStatement::getType() const noexcept
{
	return Type::ExternalDeclaration;
}

Statement::Type OptionDeclarationStatement::getType() const noexcept
{
	return Type::OptionDeclaration;
}

Statement::Type FunctionDeclarationStatement::getType() const noexcept
{
	return Type::FunctionDeclaration;
}

Statement::Type EntryFunctionDeclarationStatement::getType() const noexcept
{
	return Type::EntryFunctionDeclaration;
}

Statement::Type ExpressionStatement::getType() const noexcept
{
	return Type::Expression;
}

Statement::Type ReturnStatement::getType() const noexcept
{
	return Type::Return;
}

Statement::Type SequenceStatement::getType() const noexcept
{
	return Type::Sequence;
}
