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

#include <Atema/Shader/UberShader.hpp>
#include <Atema/Shader/Ast/AstPreprocessor.hpp>

using namespace at;

UberShader::UberShader(const Ptr<SequenceStatement>& ast) :
	m_ast(ast),
	m_extractorReady(false)
{
}

UberShader::UberShader(UPtr<SequenceStatement>&& ast) :
	m_ast(std::move(ast)),
	m_extractorReady(false)
{
}

UberShader::~UberShader()
{
}

Ptr<UberShader> UberShader::createInstance(const std::vector<Option>& options) const
{
	AstPreprocessor astPreprocessor;

	// Initialize preprocessor state
	m_ast->accept(astPreprocessor);

	size_t bufferElementIndex = 0;

	for (auto& option : options)
	{
		ATEMA_ASSERT(!option.name.empty(), "Invalid option name");

		astPreprocessor.setOption(option.name, option.value);
	}

	// Initialize shaders
	auto processedAst = astPreprocessor.process(*m_ast);

	if (!processedAst)
		ATEMA_ERROR("An error occurred during shader preprocessing");

	UPtr<SequenceStatement> sequence;

	if (processedAst->getType() == Statement::Type::Sequence)
	{
		sequence.reset(static_cast<SequenceStatement*>(processedAst.release()));
	}
	else
	{
		sequence = std::make_unique<SequenceStatement>();
		sequence->statements.emplace_back(std::move(processedAst));
	}

	return std::make_shared<UberShader>(std::move(sequence));
}

Ptr<UberShader> UberShader::extractStage(AstShaderStage stage)
{
	initializeExtractor();

	auto ast = m_astReflector.getAst(stage);

	if (!ast)
		ATEMA_ERROR("The required stage was not found");

	return std::make_shared<UberShader>(std::move(ast));
}

const AstReflection& UberShader::getReflection(AstShaderStage stage)
{
	// Check if stage reflection was already generated
	const auto it = m_stageReflections.find(stage);

	if (it != m_stageReflections.end())
		return it->second;

	// If not, create stage reflection
	initializeExtractor();

	m_stageReflections[stage] = m_astReflector.getReflection(stage);

	return m_stageReflections[stage];
}

const Ptr<SequenceStatement>& UberShader::getAst() const
{
	return m_ast;
}

void UberShader::initializeExtractor()
{
	if (m_extractorReady)
		return;

	m_ast->accept(m_astReflector);

	m_extractorReady = true;
}
