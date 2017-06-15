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
	// VERTEX ATTRIBUTE INFOS
	//--------------------------

	VertexAttributeInfos::VertexAttributeInfos()
		: type(VertexAttributeType::Undefined), usage(VertexAttributeUsage::Undefined), size(0), byteSize(0), offset(0)
	{
	}

	bool VertexAttributeInfos::operator==(const VertexAttributeInfos& attribute) const
	{
		if (type != attribute.type)
			return false;

		if (usage != attribute.usage)
			return false;

		if (size != attribute.size)
			return false;

		if (byteSize != attribute.byteSize)
			return false;

		if (offset != attribute.offset)
			return false;

		if (name.compare(attribute.name) != 0)
			return false;

		return true;
	}

	bool VertexAttributeInfos::operator!=(const VertexAttributeInfos& attribute) const
	{
		return !operator==(attribute);
	}

	//--------------------------
	// VERTEX FORMAT
	//--------------------------

	VertexFormat::Iterator VertexFormat::begin()
	{
		return m_format.begin();
	}

	VertexFormat::ConstIterator VertexFormat::begin() const
	{
		return m_format.begin();
	}

	VertexFormat::Iterator VertexFormat::end()
	{
		return m_format.end();
	}

	VertexFormat::ConstIterator VertexFormat::end() const
	{
		return m_format.end();
	}

	void VertexFormat::add(const VertexAttributeInfos& info)
	{
		m_format.push_back(info);
	}

	size_t VertexFormat::getSize() const
	{
		return m_format.size();
	}

	size_t VertexFormat::getByteSize() const
	{
		size_t byteSize = 0;

		for (auto& attrib : m_format)
			byteSize += attrib.byteSize;

		return byteSize;
	}

	VertexAttributeInfos& VertexFormat::operator[](size_t index)
	{
		return m_format[index];
	}

	const VertexAttributeInfos& VertexFormat::operator[](size_t index) const
	{
		return m_format[index];
	}

	bool VertexFormat::operator==(const VertexFormat& format) const
	{
		size_t size = getSize();
		
		if (format.getSize() == size)
		{
			for (size_t i = 0; i < size; i++)
			{
				if (m_format[i] != format.m_format[i])
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
}
