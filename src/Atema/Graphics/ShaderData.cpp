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

#include <Atema/Core/MemoryMapper.hpp>
#include <Atema/Core/Utils.hpp>
#include <Atema/Graphics/ShaderData.hpp>

using namespace at;

namespace
{
	constexpr size_t StructLayoutCount = static_cast<size_t>(StructLayout::_COUNT);

	template <typename T>
	std::vector<T> initializeLayouts()
	{
		std::vector<T> layouts;

		for (size_t i = 0; i < StructLayoutCount; i++)
			layouts.emplace_back(static_cast<StructLayout>(i));

		return layouts;
	}
}

ShaderData::Layout::Layout() :
	m_size(0)
{
}

size_t ShaderData::Layout::getByteSize() const noexcept
{
	return m_size;
}

void ShaderData::Layout::initialize(const BufferLayout& bufferLayout)
{
	m_size = bufferLayout.getSize();
}

ShaderData::ShaderData()
{
}

// FrameData
FrameData::Layout::Layout(StructLayout structLayout)
{
	BufferLayout bufferLayout(structLayout);

	projectionOffset = bufferLayout.addMatrix(BufferElementType::Float, 4, 4);
	viewOffset = bufferLayout.addMatrix(BufferElementType::Float, 4, 4);
	cameraPositionOffset = bufferLayout.add(BufferElementType::Float3);

	initialize(bufferLayout);
}

const FrameData::Layout& FrameData::getLayout(StructLayout structLayout)
{
	static std::vector<Layout> s_layouts = initializeLayouts<Layout>();

	return s_layouts[static_cast<size_t>(structLayout)];
}

void FrameData::copyTo(void* dstData, StructLayout structLayout)
{
	const auto& layout = getLayout(structLayout);

	mapMemory<Matrix4f>(dstData, layout.projectionOffset) = projection;
	mapMemory<Matrix4f>(dstData, layout.viewOffset) = view;
	mapMemory<Vector3f>(dstData, layout.cameraPositionOffset) = cameraPosition;
}

// ObjectData
ObjectData::Layout::Layout(StructLayout structLayout)
{
	BufferLayout bufferLayout(structLayout);

	modelOffset = bufferLayout.addMatrix(BufferElementType::Float, 4, 4);

	initialize(bufferLayout);
}

const ObjectData::Layout& ObjectData::getLayout(StructLayout structLayout)
{
	static std::vector<Layout> s_layouts = initializeLayouts<Layout>();

	return s_layouts[static_cast<size_t>(structLayout)];
}

void ObjectData::copyTo(void* dstData, StructLayout structLayout)
{
	const auto& layout = getLayout(structLayout);

	mapMemory<Matrix4f>(dstData, layout.modelOffset) = model;
}

// ShadowData
ShadowData::Layout::Layout(StructLayout structLayout)
{
	BufferLayout bufferLayout(structLayout);

	viewProjectionOffset = bufferLayout.addMatrix(BufferElementType::Float, 4, 4);
	depthOffset = bufferLayout.add(BufferElementType::Float);
	depthBiasOffset = bufferLayout.add(BufferElementType::Float);

	initialize(bufferLayout);
}

ShadowData::ShadowData() :
	depth(0.0f),
	depthBias(0.0f)
{
}

const ShadowData::Layout& ShadowData::getLayout(StructLayout structLayout)
{
	static std::vector<Layout> s_layouts = initializeLayouts<Layout>();

	return s_layouts[static_cast<size_t>(structLayout)];
}

void ShadowData::copyTo(void* dstData, StructLayout structLayout)
{
	const auto& layout = getLayout(structLayout);

	mapMemory<Matrix4f>(dstData, layout.viewProjectionOffset) = viewProjection;
	mapMemory<float>(dstData, layout.depthOffset) = depth;
	mapMemory<float>(dstData, layout.depthBiasOffset) = depthBias;
}

// CascadedShadowData
CascadedShadowData::Layout::Layout(StructLayout structLayout)
{
	BufferLayout bufferLayout(structLayout);

	countOffset = bufferLayout.add(BufferElementType::UInt);
	viewProjectionOffset = bufferLayout.addMatrixArray(BufferElementType::Float, 4, 4, true, MaxCascadeCount);
	depthOffset = bufferLayout.addArray(BufferElementType::Float, MaxCascadeCount);
	depthBiasOffset = bufferLayout.addArray(BufferElementType::Float, MaxCascadeCount);

	initialize(bufferLayout);
}

const CascadedShadowData::Layout& CascadedShadowData::getLayout(StructLayout structLayout)
{
	static std::vector<Layout> s_layouts = initializeLayouts<Layout>();

	return s_layouts[static_cast<size_t>(structLayout)];
}

void CascadedShadowData::copyTo(void* dstData, StructLayout structLayout)
{
	const auto& layout = getLayout(structLayout);

	const size_t cascadeCount = std::min(cascades.size(), MaxCascadeCount);

	mapMemory<uint32_t>(dstData, layout.countOffset) = static_cast<uint32_t>(cascadeCount);
	MemoryMapper viewProjections(dstData, layout.viewProjectionOffset, sizeof(Matrix4f));
	MemoryMapper depths(dstData, layout.depthOffset, BufferLayout::getArrayAlignment(BufferElementType::Float));
	MemoryMapper depthBiases(dstData, layout.depthBiasOffset, BufferLayout::getArrayAlignment(BufferElementType::Float));

	for (size_t i = 0; i < cascadeCount; i++)
	{
		viewProjections.map<Matrix4f>(i) = cascades[i].viewProjection;
		depths.map<float>(i) = cascades[i].depth;
		depthBiases.map<float>(i) = cascades[i].depthBias;
	}
}

// DirectionalLightData
DirectionalLightData::Layout::Layout(StructLayout structLayout)
{
	BufferLayout bufferLayout(structLayout);

	directionOffset = bufferLayout.add(BufferElementType::Float3);
	colorOffset = bufferLayout.add(BufferElementType::Float3);
	ambientStrengthOffset = bufferLayout.add(BufferElementType::Float);
	diffuseStrengthOffset = bufferLayout.add(BufferElementType::Float);

	initialize(bufferLayout);
}

DirectionalLightData::DirectionalLightData() :
	ambientStrength(0.0f),
	diffuseStrength(0.0f)
{
}

const DirectionalLightData::Layout& DirectionalLightData::getLayout(StructLayout structLayout)
{
	static std::vector<Layout> s_layouts = initializeLayouts<Layout>();

	return s_layouts[static_cast<size_t>(structLayout)];
}

void DirectionalLightData::copyTo(void* dstData, StructLayout structLayout)
{
	const auto& layout = getLayout(structLayout);

	mapMemory<Vector3f>(dstData, layout.directionOffset) = direction;
	mapMemory<Vector3f>(dstData, layout.colorOffset) = color.toVector3f();
	mapMemory<float>(dstData, layout.ambientStrengthOffset) = ambientStrength;
	mapMemory<float>(dstData, layout.diffuseStrengthOffset) = diffuseStrength;
}
