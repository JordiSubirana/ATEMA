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

#include <Atema/Renderer/Vertex.hpp>

namespace at
{
	//--------------------------
	// VERTEX ATTRIBUTE
	//--------------------------

	VertexAttribute::VertexAttribute() : VertexAttribute("", Type::Undefined, Usage::Undefined)
	{
	}

	VertexAttribute::VertexAttribute(const std::string& name, Type type, Usage usage) :
		name(name), type(type), usage(usage), m_offset(0)
	{
	}

	void VertexAttribute::setByteOffset(size_t byteOffset)
	{
		m_offset = byteOffset;
	}

	size_t VertexAttribute::getByteOffset() const
	{
		return m_offset;
	}

	size_t VertexAttribute::count() const
	{
		if (type == Type::Undefined)
			return 0;

		return static_cast<size_t>(type) % 4 + 1;
	}

	size_t VertexAttribute::getByteSize() const
	{
		static size_t byteSizes[] =
		{
			sizeof(int), 2*sizeof(int), 3*sizeof(int), 4*sizeof(int),
			sizeof(unsigned), 2*sizeof(unsigned), 3*sizeof(unsigned), 4*sizeof(unsigned),
			sizeof(float), 2*sizeof(float), 3*sizeof(float), 4*sizeof(float),
			sizeof(double), 2*sizeof(double), 3*sizeof(double), 4*sizeof(double),
			0
		};

		return byteSizes[static_cast<size_t>(type)];
	}

	bool VertexAttribute::operator==(const VertexAttribute& a) const
	{
		return name.compare(a.name) == 0 && type == a.type;
	}

	bool VertexAttribute::operator!=(const VertexAttribute& attribute) const
	{
		return !operator==(attribute);
	}
	
	//--------------------------
	// VERTEX FORMAT
	//--------------------------

	VertexFormat::VertexFormat(): m_valid(true), m_byteSize(0)
	{
	}

	VertexFormat::VertexFormat(std::initializer_list<VertexAttribute> attributes) :
		m_attributes(attributes), m_valid(false), m_byteSize(0)
	{
		update();
	}

	VertexFormat::Iterator VertexFormat::begin()
	{
		return m_attributes.begin();
	}

	VertexFormat::ConstIterator VertexFormat::begin() const
	{
		return m_attributes.begin();
	}

	VertexFormat::Iterator VertexFormat::end()
	{
		return m_attributes.end();
	}

	VertexFormat::ConstIterator VertexFormat::end() const
	{
		return m_attributes.end();
	}

	void VertexFormat::add(const VertexAttribute& info)
	{
		m_attributes.push_back(info);

		update();
	}

	void VertexFormat::clear()
	{
		m_attributes.clear();

		m_byteSize = 0;
	}

	size_t VertexFormat::getSize() const
	{
		return m_attributes.size();
	}

	size_t VertexFormat::getByteSize() const
	{
		return m_byteSize;
	}

	VertexFormat& VertexFormat::operator=(std::initializer_list<VertexAttribute> attributes)
	{
		m_attributes = attributes;

		update();

		return *this;
	}

	VertexFormat& VertexFormat::operator=(const VertexFormat& format)
	{
		m_attributes = format.m_attributes;

		update();

		return *this;
	}

	VertexAttribute& VertexFormat::operator[](size_t index)
	{
		return m_attributes[index];
	}

	const VertexAttribute& VertexFormat::operator[](size_t index) const
	{
		return m_attributes[index];
	}

	bool VertexFormat::operator==(const VertexFormat& format) const
	{
		size_t size = getSize();
		
		if (format.getSize() == size)
		{
			for (size_t i = 0; i < size; i++)
			{
				if (m_attributes[i] != format.m_attributes[i])
					return false;
			}
		}
		else
		{
			return false;
		}

		return true;
	}

	bool VertexFormat::operator!=(const VertexFormat& format) const
	{
		return !operator==(format);
	}

	void VertexFormat::update()
	{
		m_byteSize = 0;

		for (auto& attrib : m_attributes)
		{
			attrib.setByteOffset(m_byteSize);
			m_byteSize += attrib.getByteSize();
		}
	}
}
