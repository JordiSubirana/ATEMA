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

#ifndef ATEMA_GRAPHICS_SHADERDATA_HPP
#define ATEMA_GRAPHICS_SHADERDATA_HPP

#include <Atema/Graphics/Config.hpp>
#include <Atema/Math/Matrix.hpp>
#include <Atema/Math/Vector.hpp>
#include <Atema/Renderer/BufferLayout.hpp>
#include <Atema/Shader/Ast/Enums.hpp>

namespace at
{
	class Light;

	class ATEMA_GRAPHICS_API ShaderData
	{
	public:
		class Layout
		{
		public:
			Layout();

			size_t getByteSize() const noexcept;

		protected:
			void initialize(const BufferLayout& bufferLayout);

		private:
			size_t m_size;
		};

		ShaderData();
		ShaderData(const ShaderData& other) = default;
		ShaderData(ShaderData&& other) noexcept = default;
		virtual ~ShaderData() = default;

		virtual size_t getByteSize(StructLayout structLayout = StructLayout::Default) const noexcept = 0;
		virtual void copyTo(void* dstData, StructLayout structLayout = StructLayout::Default) const = 0;

		ShaderData& operator=(const ShaderData& other) = default;
		ShaderData& operator=(ShaderData&& other) noexcept = default;
	};

	// Default ShaderData structs

	class ATEMA_GRAPHICS_API FrameData : public ShaderData
	{
	public:
		class Layout : public ShaderData::Layout
		{
		public:
			Layout() = delete;
			Layout(StructLayout structLayout = StructLayout::Default);

			size_t projectionOffset;
			size_t viewOffset;
			size_t cameraPositionOffset;
			size_t screenSizeOffset;
		};

		FrameData() = default;
		FrameData(const FrameData& other) = default;
		FrameData(FrameData&& other) noexcept = default;
		virtual ~FrameData() = default;

		static const Layout& getLayout(StructLayout structLayout = StructLayout::Default);

		size_t getByteSize(StructLayout structLayout) const noexcept override;
		void copyTo(void* dstData, StructLayout structLayout = StructLayout::Default) const override;

		Matrix4f projection;
		Matrix4f view;
		Vector3f cameraPosition;
		Vector2<uint32_t> screenSize;

		FrameData& operator=(const FrameData& other) = default;
		FrameData& operator=(FrameData&& other) noexcept = default;
	};

	class ATEMA_GRAPHICS_API TransformData : public ShaderData
	{
	public:
		class Layout : public ShaderData::Layout
		{
		public:
			Layout() = delete;
			Layout(StructLayout structLayout = StructLayout::Default);

			size_t modelOffset;
		};

		TransformData() = default;
		TransformData(const TransformData& other) = default;
		TransformData(TransformData&& other) noexcept = default;
		virtual ~TransformData() = default;

		static const Layout& getLayout(StructLayout structLayout = StructLayout::Default);

		size_t getByteSize(StructLayout structLayout) const noexcept override;
		void copyTo(void* dstData, StructLayout structLayout = StructLayout::Default) const override;

		Matrix4f model;

		TransformData& operator=(const TransformData& other) = default;
		TransformData& operator=(TransformData&& other) noexcept = default;
	};

	class ATEMA_GRAPHICS_API ShadowData : public ShaderData
	{
	public:
		class Layout : public ShaderData::Layout
		{
		public:
			Layout() = delete;
			Layout(StructLayout structLayout = StructLayout::Default);

			size_t viewProjectionOffset;
			size_t depthOffset;
			size_t depthBiasOffset;
		};

		ShadowData();
		ShadowData(const ShadowData& other) = default;
		ShadowData(ShadowData&& other) noexcept = default;
		virtual ~ShadowData() = default;

		static const Layout& getLayout(StructLayout structLayout = StructLayout::Default);

		size_t getByteSize(StructLayout structLayout) const noexcept override;
		void copyTo(void* dstData, StructLayout structLayout = StructLayout::Default) const override;

		Matrix4f viewProjection;
		float depth;
		float depthBias;

		ShadowData& operator=(const ShadowData& other) = default;
		ShadowData& operator=(ShadowData&& other) noexcept = default;
	};

	class ATEMA_GRAPHICS_API CascadedShadowData : public ShaderData
	{
	public:
		static constexpr size_t MaxCascadeCount = 16;

		class Layout : public ShaderData::Layout
		{
		public:
			Layout() = delete;
			Layout(StructLayout structLayout = StructLayout::Default);

			size_t countOffset;
			size_t viewProjectionOffset;
			size_t depthOffset;
			size_t depthBiasOffset;
		};

		CascadedShadowData() = default;
		CascadedShadowData(const CascadedShadowData& other) = default;
		CascadedShadowData(CascadedShadowData&& other) noexcept = default;
		virtual ~CascadedShadowData() = default;

		static const Layout& getLayout(StructLayout structLayout = StructLayout::Default);

		size_t getByteSize(StructLayout structLayout) const noexcept override;
		void copyTo(void* dstData, StructLayout structLayout = StructLayout::Default) const override;

		std::vector<ShadowData> cascades;

		CascadedShadowData& operator=(const CascadedShadowData& other) = default;
		CascadedShadowData& operator=(CascadedShadowData&& other) noexcept = default;
	};

	class ATEMA_GRAPHICS_API LightData : public ShaderData
	{
	public:
		class Layout : public ShaderData::Layout
		{
		public:
			Layout() = delete;
			Layout(StructLayout structLayout = StructLayout::Default);

			size_t typeOffset;
			size_t transformOffset;
			size_t colorOffset;
			size_t intensityOffset;
			size_t indirectIntensityOffset;
			size_t parameter0Offset;
			size_t parameter1Offset;
		};

		LightData();
		LightData(const LightData& other) = default;
		LightData(LightData&& other) noexcept = default;
		virtual ~LightData() = default;

		static const Layout& getLayout(StructLayout structLayout = StructLayout::Default);

		size_t getByteSize(StructLayout structLayout) const noexcept override;
		void copyTo(void* dstData, StructLayout structLayout = StructLayout::Default) const override;

		const Light* light;

		LightData& operator=(const LightData& other) = default;
		LightData& operator=(LightData&& other) noexcept = default;
	};
}

#endif
