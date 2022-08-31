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

#include <Atema/Shader/Spirv/SpirvShaderWriter.hpp>

#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>

using namespace at;

namespace
{
	EShLanguage getEShLanguage(AstShaderStage stage)
	{
		switch (stage)
		{
			case AstShaderStage::Vertex: return EShLangVertex;
			case AstShaderStage::Fragment: return EShLangFragment;
			default:
			{
				ATEMA_ERROR("Invalid AstShaderStage");
			}
		}

		return EShLangVertex;
	}

	TBuiltInResource getDefaultResources()
	{
		TBuiltInResource resources;

		resources.maxLights = 32;
		resources.maxClipPlanes = 6;
		resources.maxTextureUnits = 32;
		resources.maxTextureCoords = 32;
		resources.maxVertexAttribs = 64;
		resources.maxVertexUniformComponents = 4096;
		resources.maxVaryingFloats = 64;
		resources.maxVertexTextureImageUnits = 32;
		resources.maxCombinedTextureImageUnits = 80;
		resources.maxTextureImageUnits = 32;
		resources.maxFragmentUniformComponents = 4096;
		resources.maxDrawBuffers = 32;
		resources.maxVertexUniformVectors = 128;
		resources.maxVaryingVectors = 8;
		resources.maxFragmentUniformVectors = 16;
		resources.maxVertexOutputVectors = 16;
		resources.maxFragmentInputVectors = 15;
		resources.minProgramTexelOffset = -8;
		resources.maxProgramTexelOffset = 7;
		resources.maxClipDistances = 8;
		resources.maxComputeWorkGroupCountX = 65535;
		resources.maxComputeWorkGroupCountY = 65535;
		resources.maxComputeWorkGroupCountZ = 65535;
		resources.maxComputeWorkGroupSizeX = 1024;
		resources.maxComputeWorkGroupSizeY = 1024;
		resources.maxComputeWorkGroupSizeZ = 64;
		resources.maxComputeUniformComponents = 1024;
		resources.maxComputeTextureImageUnits = 16;
		resources.maxComputeImageUniforms = 8;
		resources.maxComputeAtomicCounters = 8;
		resources.maxComputeAtomicCounterBuffers = 1;
		resources.maxVaryingComponents = 60;
		resources.maxVertexOutputComponents = 64;
		resources.maxGeometryInputComponents = 64;
		resources.maxGeometryOutputComponents = 128;
		resources.maxFragmentInputComponents = 128;
		resources.maxImageUnits = 8;
		resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
		resources.maxCombinedShaderOutputResources = 8;
		resources.maxImageSamples = 0;
		resources.maxVertexImageUniforms = 0;
		resources.maxTessControlImageUniforms = 0;
		resources.maxTessEvaluationImageUniforms = 0;
		resources.maxGeometryImageUniforms = 0;
		resources.maxFragmentImageUniforms = 8;
		resources.maxCombinedImageUniforms = 8;
		resources.maxGeometryTextureImageUnits = 16;
		resources.maxGeometryOutputVertices = 256;
		resources.maxGeometryTotalOutputComponents = 1024;
		resources.maxGeometryUniformComponents = 1024;
		resources.maxGeometryVaryingComponents = 64;
		resources.maxTessControlInputComponents = 128;
		resources.maxTessControlOutputComponents = 128;
		resources.maxTessControlTextureImageUnits = 16;
		resources.maxTessControlUniformComponents = 1024;
		resources.maxTessControlTotalOutputComponents = 4096;
		resources.maxTessEvaluationInputComponents = 128;
		resources.maxTessEvaluationOutputComponents = 128;
		resources.maxTessEvaluationTextureImageUnits = 16;
		resources.maxTessEvaluationUniformComponents = 1024;
		resources.maxTessPatchComponents = 120;
		resources.maxPatchVertices = 32;
		resources.maxTessGenLevel = 64;
		resources.maxViewports = 16;
		resources.maxVertexAtomicCounters = 0;
		resources.maxTessControlAtomicCounters = 0;
		resources.maxTessEvaluationAtomicCounters = 0;
		resources.maxGeometryAtomicCounters = 0;
		resources.maxFragmentAtomicCounters = 8;
		resources.maxCombinedAtomicCounters = 8;
		resources.maxAtomicCounterBindings = 1;
		resources.maxVertexAtomicCounterBuffers = 0;
		resources.maxTessControlAtomicCounterBuffers = 0;
		resources.maxTessEvaluationAtomicCounterBuffers = 0;
		resources.maxGeometryAtomicCounterBuffers = 0;
		resources.maxFragmentAtomicCounterBuffers = 1;
		resources.maxCombinedAtomicCounterBuffers = 1;
		resources.maxAtomicCounterBufferSize = 16384;
		resources.maxTransformFeedbackBuffers = 4;
		resources.maxTransformFeedbackInterleavedComponents = 64;
		resources.maxCullDistances = 8;
		resources.maxCombinedClipAndCullDistances = 8;
		resources.maxSamples = 4;
		resources.maxMeshOutputVerticesNV = 256;
		resources.maxMeshOutputPrimitivesNV = 512;
		resources.maxMeshWorkGroupSizeX_NV = 32;
		resources.maxMeshWorkGroupSizeY_NV = 1;
		resources.maxMeshWorkGroupSizeZ_NV = 1;
		resources.maxTaskWorkGroupSizeX_NV = 32;
		resources.maxTaskWorkGroupSizeY_NV = 1;
		resources.maxTaskWorkGroupSizeZ_NV = 1;
		resources.maxMeshViewCountNV = 4;

		resources.limits.nonInductiveForLoops = true;
		resources.limits.whileLoops = true;
		resources.limits.doWhileLoops = true;
		resources.limits.generalUniformIndexing = true;
		resources.limits.generalAttributeMatrixVectorIndexing = true;
		resources.limits.generalVaryingIndexing = true;
		resources.limits.generalSamplerIndexing = true;
		resources.limits.generalVariableIndexing = true;
		resources.limits.generalConstantMatrixVectorIndexing = true;

		return resources;
	}

	const TBuiltInResource defaultBuiltInResource = getDefaultResources();
}

SpirvShaderWriter::SpirvShaderWriter() :
	SpirvShaderWriter(Settings())
{
}

SpirvShaderWriter::SpirvShaderWriter(const Settings& settings) :
	GlslShaderWriter(m_glslStream, { settings.stage }),
	m_requestedStage(settings.stage),
	m_entryFound(false),
	m_stage(AstShaderStage::Vertex)
{
}

SpirvShaderWriter::~SpirvShaderWriter()
{
}

void SpirvShaderWriter::compile(std::vector<uint32_t>& spirv)
{
	compileSpirv(spirv);
}

void SpirvShaderWriter::compile(std::ostream& ostream)
{
	std::vector<uint32_t> spirv;

	compileSpirv(spirv);

	const auto codeData = reinterpret_cast<char*>(spirv.data());
	const auto codeSize = spirv.size() * sizeof(uint32_t);

	for (size_t i = 0; i < codeSize; i++)
		ostream << codeData[i];
}

void SpirvShaderWriter::visit(const EntryFunctionDeclarationStatement& statement)
{
	if (m_requestedStage.has_value())
	{
		if (statement.stage == m_requestedStage.value())
		{
			if (m_entryFound)
				ATEMA_ERROR("Only one shader entry must be defined");

			m_stage = statement.stage;
			m_entryFound = true;
		}
	}
	else
	{
		if (!m_entryFound)
		{
			m_stage = statement.stage;
			m_entryFound = true;
		}
		else
		{
			ATEMA_ERROR("Only one shader entry must be defined");
		}
	}

	GlslShaderWriter::visit(statement);
}

void SpirvShaderWriter::compileSpirv(std::vector<uint32_t>& spirv)
{
	if (!m_entryFound)
		ATEMA_ERROR("A shader entry must be defined");

	glslang::InitializeProcess();

	const auto eShLanguage = getEShLanguage(m_stage);

	// Create shader
	glslang::TShader shader(eShLanguage);

	const auto str = m_glslStream.str();
	const auto cstr = str.c_str();
	const int length = static_cast<int>(str.length());

	shader.setStringsWithLengths(&cstr, &length, 1);

	shader.setEnvInput(glslang::EShSourceGlsl, eShLanguage, glslang::EShClientVulkan, 100);
	shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_1);
	shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_1);

	auto success = shader.parse(&defaultBuiltInResource, 100, ECoreProfile, false, false, EShMsgDefault);

	// Compilation failed
	if (!success)
	{
		const auto infoLog = shader.getInfoLog();

		ATEMA_ERROR("Compilation failed :\n" + std::string(infoLog));
	}

	// Create program
	glslang::TProgram program;

	program.addShader(&shader);

	success = program.link(EShMsgDefault);

	// Linking failed
	if (!success)
	{
		const auto infoLog = program.getInfoLog();

		ATEMA_ERROR("Linking failed :\n" + std::string(infoLog));
	}

	const auto intermediate = program.getIntermediate(eShLanguage);

	spv::SpvBuildLogger logger;

	glslang::SpvOptions spvOptions;

	spvOptions.generateDebugInfo = true;
	spvOptions.stripDebugInfo = true;
	spvOptions.disableOptimizer = false;
	spvOptions.optimizeSize = true;
	spvOptions.disassemble = false;
	spvOptions.validate = false;
	glslang::GlslangToSpv(*intermediate, spirv, &logger, &spvOptions);

	glslang::FinalizeProcess();
}
