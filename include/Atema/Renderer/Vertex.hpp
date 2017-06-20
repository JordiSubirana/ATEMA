/*
Copyright 2017 Jordi SUBIRANA

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

#ifndef ATEMA_RENDERER_VERTEX_HPP
#define ATEMA_RENDERER_VERTEX_HPP

#include <Atema/Renderer/Config.hpp>
#include <string>
#include <vector>

namespace at
{
	// Type of VertexAttribute
	enum class VertexAttributeType
	{
		Int, Int2, Int3, Int4,
		Unsigned, Unsigned2, Unsigned3, Unsigned4,
		Float, Float2, Float3, Float4,
		Double, Double2, Double3, Double4,
		Undefined
	};

	// Usage of VertexAttribute
	enum class VertexAttributeUsage
	{
		Position, Color, Texture, Normal, Tangent, Binormal,
		BoneID, BoneWeight, Undefined
	};

	// Type to enum converter
	namespace detail
	{
		template <typename T> struct VertexTypeToEnum {};
		template <> struct VertexTypeToEnum<int> { static constexpr VertexAttributeType type = VertexAttributeType::Int; };
		template <> struct VertexTypeToEnum<unsigned> { static constexpr VertexAttributeType type = VertexAttributeType::Unsigned; };
		template <> struct VertexTypeToEnum<float> { static constexpr VertexAttributeType type = VertexAttributeType::Float; };
		template <> struct VertexTypeToEnum<double> { static constexpr VertexAttributeType type = VertexAttributeType::Double; };
	}

	// VertexAttribute class
	template <typename T, VertexAttributeUsage U = VertexAttributeUsage::Undefined>
	struct VertexAttribute
	{
		static const VertexAttributeType baseType = detail::VertexTypeToEnum<T>::type;
		static const size_t baseTypeSize = sizeof(T);
		static const VertexAttributeUsage usage = U;
	};

	// VertexAttribute name
	template <typename T>
	struct VertexAttributeName { static const std::string value; };

	// VertexAttribute informations
	struct ATEMA_RENDERER_API VertexAttributeInfos
	{
		VertexAttributeInfos();

		bool operator==(const VertexAttributeInfos& attribute) const;
		bool operator!=(const VertexAttributeInfos& attribute) const;

		VertexAttributeType type;
		VertexAttributeUsage usage;
		std::string name;
		size_t size;
		size_t byteSize;
		size_t offset;
	};

	// VertexFormat
	class ATEMA_RENDERER_API VertexFormat
	{
	public:
		using Iterator = std::vector<VertexAttributeInfos>::iterator;
		using ConstIterator = std::vector<VertexAttributeInfos>::const_iterator;

		VertexFormat() = default;
		virtual ~VertexFormat() = default;

		Iterator begin();
		ConstIterator begin() const;
		Iterator end();
		ConstIterator end() const;

		void add(const VertexAttributeInfos& info);

		size_t getSize() const;
		size_t getByteSize() const;

		VertexAttributeInfos& operator[](size_t index);
		const VertexAttributeInfos& operator[](size_t index) const;

		bool operator==(const VertexFormat& format) const;
		bool operator!=(const VertexFormat& format) const;

	private:
		std::vector<VertexAttributeInfos> m_format;
	};

	// Vertex class
	template <typename...Attributes>
	struct Vertex
	{
		static VertexFormat& getFormat()
		{
			return format;
		}

		static VertexFormat getDefaultFormat()
		{
			VertexFormat format;
			auto init = { getAttributeInfos<Attributes>()... };

			size_t offset = 0;

			for (auto& attrib : init)
			{
				auto attribute = attrib;
				attribute.offset = offset;
				offset += attrib.byteSize;
				format.add(attribute);
			}

			return format;
		}

	private:
		template <typename T>
		static VertexAttributeInfos getAttributeInfos()
		{
			VertexAttributeInfos infos;

			infos.type = static_cast<VertexAttributeType>(static_cast<int>(T::baseType) + (sizeof(T) / T::baseTypeSize - 1));
			infos.usage = T::usage;
			infos.size = sizeof(T) / T::baseTypeSize;
			infos.byteSize = sizeof(T);
			infos.name = VertexAttributeName<T>::value;

			return infos;
		}

		static VertexFormat format;
	};
}

#endif