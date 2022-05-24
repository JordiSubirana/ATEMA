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

#ifndef ATEMA_RENDERER_ENUMS_HPP
#define ATEMA_RENDERER_ENUMS_HPP

#include <Atema/Core/Flags.hpp>

namespace at
{
	enum class ImageFormat
	{
		// Color (8 bit components)
		R8_UNORM,
		R8_SNORM,
		R8_USCALED,
		R8_SSCALED,
		R8_UINT,
		R8_SINT,
		R8_SRGB,

		RG8_UNORM,
		RG8_SNORM,
		RG8_USCALED,
		RG8_SSCALED,
		RG8_UINT,
		RG8_SINT,
		RG8_SRGB,

		RGB8_UNORM,
		RGB8_SNORM,
		RGB8_USCALED,
		RGB8_SSCALED,
		RGB8_UINT,
		RGB8_SINT,
		RGB8_SRGB,

		BGR8_UNORM,
		BGR8_SNORM,
		BGR8_USCALED,
		BGR8_SSCALED,
		BGR8_UINT,
		BGR8_SINT,
		BGR8_SRGB,

		RGBA8_UNORM,
		RGBA8_SNORM,
		RGBA8_USCALED,
		RGBA8_SSCALED,
		RGBA8_UINT,
		RGBA8_SINT,
		RGBA8_SRGB,

		BGRA8_UNORM,
		BGRA8_SNORM,
		BGRA8_USCALED,
		BGRA8_SSCALED,
		BGRA8_UINT,
		BGRA8_SINT,
		BGRA8_SRGB,

		// Color (16 bit components)
		R16_UNORM,
		R16_SNORM,
		R16_USCALED,
		R16_SSCALED,
		R16_UINT,
		R16_SINT,
		R16_SFLOAT,

		RG16_UNORM,
		RG16_SNORM,
		RG16_USCALED,
		RG16_SSCALED,
		RG16_UINT,
		RG16_SINT,
		RG16_SFLOAT,

		RGB16_UNORM,
		RGB16_SNORM,
		RGB16_USCALED,
		RGB16_SSCALED,
		RGB16_UINT,
		RGB16_SINT,
		RGB16_SFLOAT,

		RGBA16_UNORM,
		RGBA16_SNORM,
		RGBA16_USCALED,
		RGBA16_SSCALED,
		RGBA16_UINT,
		RGBA16_SINT,
		RGBA16_SFLOAT,

		// Color (32 bit components)
		R32_UINT,
		R32_SINT,
		R32_SFLOAT,

		RG32_UINT,
		RG32_SINT,
		RG32_SFLOAT,

		RGB32_UINT,
		RGB32_SINT,
		RGB32_SFLOAT,

		RGBA32_UINT,
		RGBA32_SINT,
		RGBA32_SFLOAT,

		// Color (64 bit components)
		R64_UINT,
		R64_SINT,
		R64_SFLOAT,

		RG64_UINT,
		RG64_SINT,
		RG64_SFLOAT,

		RGB64_UINT,
		RGB64_SINT,
		RGB64_SFLOAT,

		RGBA64_UINT,
		RGBA64_SINT,
		RGBA64_SFLOAT,
		
		// DepthStencil
		D32F,
		D32F_S8U,
		D24U_S8U
	};

	enum class ImageSamples
	{
		S1 = 0x0001,
		S2 = 0x0002,
		S4 = 0x0004,
		S8 = 0x0008,
		S16 = 0x0010,
		S32 = 0x0020,
		S64 = 0x0040,
		Max = S64
	};

	ATEMA_DECLARE_FLAGS(ImageSamples);

	enum class ImageTiling
	{
		Optimal, // Implementation dependent
		Linear // Laid out in memory to allow user access
	};

	enum class ImageUsage
	{
		RenderTarget = 0x0001,
		ShaderInput = 0x0002,
		TransferSrc = 0x0004,
		TransferDst = 0x0008,

		All = RenderTarget | ShaderInput | TransferDst | TransferSrc
	};

	ATEMA_DECLARE_FLAGS(ImageUsage);

	enum class ImageLayout
	{
		Undefined,
		Attachment,
		ShaderInput,
		TransferSrc,
		TransferDst,
		Present,
		
		All
	};

	enum class AttachmentLoading
	{
		Undefined,
		Clear,
		Load
	};

	enum class AttachmentStoring
	{
		Undefined,
		Store
	};

	enum class PrimitiveTopology
	{
		PointList,
		LineList,
		LineStrip,
		TriangleList,
		TriangleStrip,
		TriangleFan,
		LineListAdjacency,
		LineStripAdjacency,
		TriangleListAdjacency,
		TriangleStripAdjacency,
		PatchList
	};

	enum class PolygonMode
	{
		Fill,
		Line,
		Point
	};

	enum class CullMode
	{
		None = 0x0000,
		Front = 0x0001,
		Back = 0x0002,
		All = Front | Back
	};

	ATEMA_DECLARE_FLAGS(CullMode);

	enum class FrontFace
	{
		Clockwise,
		CounterClockwise
	};

	enum class BlendOperation
	{
		Add,
		Subtract,
		ReverseSubtract,
		Min,
		Max
	};

	enum class BlendFactor
	{
		Zero,
		One,
		SrcColor,
		OneMinusSrcColor,
		DstColor,
		OneMinusDstColor,
		SrcAlpha,
		OneMinusSrcAlpha,
		DstAlpha,
		OneMinusDstAlpha,
		ConstantColor,
		OneMinusConstantColor,
		ConstantAlpha,
		OneMinusConstantAlpha
	};

	enum class CompareOperation
	{
		Never,
		Less,
		Equal,
		LessOrEqual,
		Greater,
		NotEqual,
		GreaterOrEqual,
		Always
	};

	enum class StencilOperation
	{
		Keep,
		Zero,
		Replace,
		IncrementAndClamp,
		DecrementAndClamp,
		Invert,
		IncrementAndWrap,
		DecrementAndWrap
	};

	enum class DescriptorType
	{
		Sampler,
		CombinedImageSampler,
		SampledImage,
		StorageImage,
		UniformTexelBuffer,
		StorageTexelBuffer,
		UniformBuffer,
		StorageBuffer,
		UniformBufferDynamic,
		StorageBufferDynamic,
		InputAttachment
	};

	enum class ShaderStage
	{
		Vertex = 0x0001,
		TessellationControl = 0x0002,
		TessellationEvaluation = 0x0004,
		Geometry = 0x0008,
		Fragment = 0x0010,
		Compute = 0x0020,

		AllGraphics = Vertex | TessellationControl | TessellationEvaluation | Geometry | Fragment,
		All = 0xFFFF
	};

	ATEMA_DECLARE_FLAGS(ShaderStage);

	enum class PipelineStage
	{
		TopOfPipe = 0x0001,
		DrawIndirect = 0x0002,
		VertexInput = 0x0004,
		VertexShader = 0x0008,
		TessellationControl = 0x0010,
		TessellationEvaluation = 0x0020,
		GeometryShader = 0x0040,
		FragmentShader = 0x0080,
		EarlyFragmentTests = 0x0100,
		LateFragmentTests = 0x0200,
		ColorAttachmentOutput = 0x0400,
		ComputeShader = 0x0800,
		Transfer = 0x1000,
		BottomOfPipe = 0x2000,
		Host = 0x4000
	};

	ATEMA_DECLARE_FLAGS(PipelineStage);

	enum class BufferUsage
	{
		Vertex,
		Index,
		Uniform,
		Transfer
	};

	enum class IndexType
	{
		U16,
		U32
	};

	enum class SamplerFilter
	{
		Nearest,
		Linear
	};

	enum class SamplerAddressMode
	{
		Repeat,
		MirroredRepeat,
		ClampToEdge,
		ClampToBorder
	};

	enum class SwapChainResult
	{
		// Success
		Success,
		// Fence/Semaphore/Query not completed yet
		NotReady,
		// Can still be used but the swapchain and the window surface properties don't match
		Suboptimal,
		// Need to recreate the swapchain
		OutOfDate,
		// Unknown error
		Error
	};

	enum class QueueType
	{
		// Support Graphics / Compute / Transfer operations
		Graphics,
		// Support Compute / Transfer operations
		Compute,
		// Support Transfer operations
		Transfer,
	};
}

#endif
