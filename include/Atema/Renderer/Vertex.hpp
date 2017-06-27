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
	// VertexAttribute
	class ATEMA_RENDERER_API VertexAttribute
	{
	public:
		enum class Type
		{
			Int, Int2, Int3, Int4,
			Unsigned, Unsigned2, Unsigned3, Unsigned4,
			Float, Float2, Float3, Float4,
			Double, Double2, Double3, Double4,
			Undefined
		};

		enum class Usage
		{
			Position, Normal, Tangent, Binormal,
			Color, Texture,
			BoneID, BoneWeight,
			Undefined
		};

		VertexAttribute();
		VertexAttribute(const std::string& name, Type type, Usage usage = Usage::Undefined);

		void setByteOffset(size_t byteOffset);
		size_t getByteOffset() const;

		size_t count() const;
		size_t getByteSize() const;

		bool operator==(const VertexAttribute& attribute) const;
		bool operator!=(const VertexAttribute& attribute) const;

		std::string name;
		Type type;
		Usage usage;

	private:
		size_t m_offset;
	};

	// VertexFormat
	class ATEMA_RENDERER_API VertexFormat
	{
	public:
		using Iterator = std::vector<VertexAttribute>::iterator;
		using ConstIterator = std::vector<VertexAttribute>::const_iterator;

		VertexFormat();
		VertexFormat(std::initializer_list<VertexAttribute> attributes);
		virtual ~VertexFormat() = default;

		Iterator begin(); // Need to call update on attribute change
		ConstIterator begin() const; // Need to call update on attribute change
		Iterator end(); // Need to call update on attribute change
		ConstIterator end() const; // Need to call update on attribute change

		void add(const VertexAttribute& info);
		void clear();

		size_t getSize() const;
		size_t getByteSize() const;

		VertexFormat& operator=(std::initializer_list<VertexAttribute> attributes);
		VertexFormat& operator=(const VertexFormat& format);

		VertexAttribute& operator[](size_t index); // Need to call update on attribute change
		const VertexAttribute& operator[](size_t index) const; // Need to call update on attribute change

		bool operator==(const VertexFormat& format) const;
		bool operator!=(const VertexFormat& format) const;

		void update();

	private:
		std::vector<VertexAttribute> m_attributes;
		bool m_valid;
		size_t m_byteSize;
	};
}

#endif