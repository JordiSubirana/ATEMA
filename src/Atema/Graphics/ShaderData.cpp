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
#include <Atema/Graphics/DirectionalLight.hpp>
#include <Atema/Graphics/Light.hpp>
#include <Atema/Graphics/PointLight.hpp>
#include <Atema/Graphics/ShaderData.hpp>
#include <Atema/Graphics/SpotLight.hpp>
#include <Atema/Math/Transform.hpp>

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
	screenSizeOffset = bufferLayout.add(BufferElementType::Float2);

	initialize(bufferLayout);
}

const FrameData::Layout& FrameData::getLayout(StructLayout structLayout)
{
	static std::vector<Layout> s_layouts = initializeLayouts<Layout>();

	return s_layouts[static_cast<size_t>(structLayout)];
}

size_t FrameData::getByteSize(StructLayout structLayout) const noexcept
{
	return getLayout(structLayout).getByteSize();
}

void FrameData::copyTo(void* dstData, StructLayout structLayout) const
{
	const auto& layout = getLayout(structLayout);

	mapMemory<Matrix4f>(dstData, layout.projectionOffset) = projection;
	mapMemory<Matrix4f>(dstData, layout.viewOffset) = view;
	mapMemory<Vector3f>(dstData, layout.cameraPositionOffset) = cameraPosition;
	mapMemory<Vector2<uint32_t>>(dstData, layout.screenSizeOffset) = screenSize;
}

// TransformData
TransformData::Layout::Layout(StructLayout structLayout)
{
	BufferLayout bufferLayout(structLayout);

	modelOffset = bufferLayout.addMatrix(BufferElementType::Float, 4, 4);

	initialize(bufferLayout);
}

const TransformData::Layout& TransformData::getLayout(StructLayout structLayout)
{
	static std::vector<Layout> s_layouts = initializeLayouts<Layout>();

	return s_layouts[static_cast<size_t>(structLayout)];
}

size_t TransformData::getByteSize(StructLayout structLayout) const noexcept
{
	return getLayout(structLayout).getByteSize();
}

void TransformData::copyTo(void* dstData, StructLayout structLayout) const
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

size_t ShadowData::getByteSize(StructLayout structLayout) const noexcept
{
	return getLayout(structLayout).getByteSize();
}

void ShadowData::copyTo(void* dstData, StructLayout structLayout) const
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

size_t CascadedShadowData::getByteSize(StructLayout structLayout) const noexcept
{
	return getLayout(structLayout).getByteSize();
}

void CascadedShadowData::copyTo(void* dstData, StructLayout structLayout) const
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

// LightData
LightData::Layout::Layout(StructLayout structLayout)
{
	BufferLayout bufferLayout(structLayout);

	typeOffset = bufferLayout.add(BufferElementType::UInt);
	transformOffset = bufferLayout.addMatrix(BufferElementType::Float, 4, 4);
	colorOffset = bufferLayout.add(BufferElementType::Float3);
	intensityOffset = bufferLayout.add(BufferElementType::Float);
	indirectIntensityOffset = bufferLayout.add(BufferElementType::Float);
	parameter0Offset = bufferLayout.add(BufferElementType::Float4);
	parameter1Offset = bufferLayout.add(BufferElementType::Float4);

	initialize(bufferLayout);
}

LightData::LightData() :
	light(nullptr)
{
}

const LightData::Layout& LightData::getLayout(StructLayout structLayout)
{
	static std::vector<Layout> s_layouts = initializeLayouts<Layout>();

	return s_layouts[static_cast<size_t>(structLayout)];
}

size_t LightData::getByteSize(StructLayout structLayout) const noexcept
{
	return getLayout(structLayout).getByteSize();
}

void LightData::copyTo(void* dstData, StructLayout structLayout) const
{
	ATEMA_ASSERT(light, "Invalid Light");

	const auto& layout = getLayout(structLayout);

	mapMemory<uint32_t>(dstData, layout.typeOffset) = static_cast<uint32_t>(light->getType());
	mapMemory<Vector3f>(dstData, layout.colorOffset) = light->getColor().toVector3f();
	mapMemory<float>(dstData, layout.intensityOffset) = light->getIntensity();
	mapMemory<float>(dstData, layout.indirectIntensityOffset) = light->getIndirectIntensity();
	auto& transform = mapMemory<Matrix4f>(dstData, layout.transformOffset);

	switch (light->getType())
	{
		case LightType::Directional:
		{
			const auto directionalLight = static_cast<const DirectionalLight*>(light);

			mapMemory<Vector3f>(dstData, layout.parameter0Offset) = directionalLight->getDirection();
			transform = Matrix4f::createIdentity();

			break;
		}
		case LightType::Point:
		{
			const auto pointLight = static_cast<const PointLight*>(light);

			mapMemory<Vector3f>(dstData, layout.parameter0Offset) = pointLight->getPosition();
			mapMemory<float>(dstData, layout.parameter0Offset + sizeof(Vector3f)) = pointLight->getRadius();

			const float r = pointLight->getRadius();
			transform = Transform(pointLight->getPosition(), Vector3f(), Vector3f(r, r, r)).getMatrix();
			
			break;
		}
		case LightType::Spot:
		{
			const auto spotLight = static_cast<const SpotLight*>(light);

			mapMemory<Vector3f>(dstData, layout.parameter0Offset) = spotLight->getPosition();
			mapMemory<float>(dstData, layout.parameter0Offset + sizeof(Vector3f)) = spotLight->getRange();

			mapMemory<Vector3f>(dstData, layout.parameter1Offset) = spotLight->getDirection();
			mapMemory<float>(dstData, layout.parameter1Offset + sizeof(Vector3f)) = std::cos(spotLight->getAngle() / 2.0f);

			const float angle = spotLight->getAngle();
			const float range = spotLight->getRange();
			const float radius = range * std::tan(angle / 2.0f);

			const Vector3f& translation = spotLight->getPosition();
			const Vector3f rotation = Quaternionf(Vector3f(0.0f, 0.0f, -1.0f), spotLight->getDirection()).toEulerAngles();
			const Vector3f scale(radius, radius, range);

			transform = Transform(translation, rotation, scale).getMatrix();

			break;
		}
		default:
		{
			ATEMA_ERROR("Unhandled Light type");
		}
	}
}
