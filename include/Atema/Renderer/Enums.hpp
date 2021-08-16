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

#ifndef ATEMA_RENDERER_ENUMS_HPP
#define ATEMA_RENDERER_ENUMS_HPP

#include <Atema/Core/Flags.hpp>

namespace at
{
	enum class ImageFormat
	{
		// Color
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
		
		// DepthStencil
		D32F,
		D32F_S8U,
		D24U_S8U
	};

	inline bool hasDepth(ImageFormat format)
	{
		switch (format)
		{
		case ImageFormat::D32F:
		case ImageFormat::D32F_S8U:
		case ImageFormat::D24U_S8U:
		{
			return true;
		}
		default:
		{
			return false;
		}
		}

		return false;
	}

	inline bool hasStencil(ImageFormat format)
	{
		switch (format)
		{
		case ImageFormat::D32F_S8U:
		case ImageFormat::D24U_S8U:
		{
			return true;
		}
		default:
		{
			return false;
		}
		}

		return false;
	}

	enum class ImageSamples
	{
		S1,
		S2,
		S4,
		S8,
		S16,
		S32,
		S64,
		Max = S64
	};

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
}

#endif
