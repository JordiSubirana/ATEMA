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

#include <Atema/Graphics/VertexTypes.hpp>

using namespace at;

const std::vector<VertexInput>& Vertex_XY::getVertexInput()
{
	static const std::vector<VertexInput> s_vertexInput =
	{
		{ 0, 0, VertexInputFormat::RG32_SFLOAT }
	};

	return s_vertexInput;
}

const std::vector<VertexInput>& Vertex_XY_RGB::getVertexInput()
{
	static const std::vector<VertexInput> s_vertexInput =
	{
		{ 0, 0, VertexInputFormat::RG32_SFLOAT },
		{ 0, 1, VertexInputFormat::RGB32_SFLOAT }
	};

	return s_vertexInput;
}

const std::vector<VertexInput>& Vertex_XY_RGBA::getVertexInput()
{
	static const std::vector<VertexInput> s_vertexInput =
	{
		{ 0, 0, VertexInputFormat::RG32_SFLOAT },
		{ 0, 1, VertexInputFormat::RGBA32_SFLOAT }
	};

	return s_vertexInput;
}

const std::vector<VertexInput>& Vertex_XY_UV::getVertexInput()
{
	static const std::vector<VertexInput> s_vertexInput =
	{
		{ 0, 0, VertexInputFormat::RG32_SFLOAT },
		{ 0, 1, VertexInputFormat::RG32_SFLOAT }
	};

	return s_vertexInput;
}

const std::vector<VertexInput>& Vertex_XYZ::getVertexInput()
{
	static const std::vector<VertexInput> s_vertexInput =
	{
		{ 0, 0, VertexInputFormat::RGB32_SFLOAT }
	};

	return s_vertexInput;
}

const std::vector<VertexInput>& Vertex_XYZ_RGB::getVertexInput()
{
	static const std::vector<VertexInput> s_vertexInput =
	{
		{ 0, 0, VertexInputFormat::RGB32_SFLOAT },
		{ 0, 1, VertexInputFormat::RGB32_SFLOAT }
	};

	return s_vertexInput;
}

const std::vector<VertexInput>& Vertex_XYZ_RGB_N::getVertexInput()
{
	static const std::vector<VertexInput> s_vertexInput =
	{
		{ 0, 0, VertexInputFormat::RGB32_SFLOAT },
		{ 0, 1, VertexInputFormat::RGB32_SFLOAT },
		{ 0, 2, VertexInputFormat::RGB32_SFLOAT }
	};

	return s_vertexInput;
}

const std::vector<VertexInput>& Vertex_XYZ_RGB_NT::getVertexInput()
{
	static const std::vector<VertexInput> s_vertexInput =
	{
		{ 0, 0, VertexInputFormat::RGB32_SFLOAT },
		{ 0, 1, VertexInputFormat::RGB32_SFLOAT },
		{ 0, 2, VertexInputFormat::RGB32_SFLOAT },
		{ 0, 3, VertexInputFormat::RGB32_SFLOAT }
	};

	return s_vertexInput;
}

const std::vector<VertexInput>& Vertex_XYZ_RGB_NTB::getVertexInput()
{
	static const std::vector<VertexInput> s_vertexInput =
	{
		{ 0, 0, VertexInputFormat::RGB32_SFLOAT },
		{ 0, 1, VertexInputFormat::RGB32_SFLOAT },
		{ 0, 2, VertexInputFormat::RGB32_SFLOAT },
		{ 0, 3, VertexInputFormat::RGB32_SFLOAT },
		{ 0, 4, VertexInputFormat::RGB32_SFLOAT }
	};

	return s_vertexInput;
}

const std::vector<VertexInput>& Vertex_XYZ_RGBA::getVertexInput()
{
	static const std::vector<VertexInput> s_vertexInput =
	{
		{ 0, 0, VertexInputFormat::RGB32_SFLOAT },
		{ 0, 1, VertexInputFormat::RGBA32_SFLOAT }
	};

	return s_vertexInput;
}

const std::vector<VertexInput>& Vertex_XYZ_RGBA_N::getVertexInput()
{
	static const std::vector<VertexInput> s_vertexInput =
	{
		{ 0, 0, VertexInputFormat::RGB32_SFLOAT },
		{ 0, 1, VertexInputFormat::RGBA32_SFLOAT },
		{ 0, 2, VertexInputFormat::RGB32_SFLOAT }
	};

	return s_vertexInput;
}

const std::vector<VertexInput>& Vertex_XYZ_RGBA_NT::getVertexInput()
{
	static const std::vector<VertexInput> s_vertexInput =
	{
		{ 0, 0, VertexInputFormat::RGB32_SFLOAT },
		{ 0, 1, VertexInputFormat::RGBA32_SFLOAT },
		{ 0, 2, VertexInputFormat::RGB32_SFLOAT },
		{ 0, 3, VertexInputFormat::RGB32_SFLOAT }
	};

	return s_vertexInput;
}

const std::vector<VertexInput>& Vertex_XYZ_RGBA_NTB::getVertexInput()
{
	static const std::vector<VertexInput> s_vertexInput =
	{
		{ 0, 0, VertexInputFormat::RGB32_SFLOAT },
		{ 0, 1, VertexInputFormat::RGBA32_SFLOAT },
		{ 0, 2, VertexInputFormat::RGB32_SFLOAT },
		{ 0, 3, VertexInputFormat::RGB32_SFLOAT },
		{ 0, 4, VertexInputFormat::RGB32_SFLOAT }
	};

	return s_vertexInput;
}

const std::vector<VertexInput>& Vertex_XYZ_UV::getVertexInput()
{
	static const std::vector<VertexInput> s_vertexInput =
	{
		{ 0, 0, VertexInputFormat::RGB32_SFLOAT },
		{ 0, 1, VertexInputFormat::RG32_SFLOAT }
	};

	return s_vertexInput;
}

const std::vector<VertexInput>& Vertex_XYZ_UV_N::getVertexInput()
{
	static const std::vector<VertexInput> s_vertexInput =
	{
		{ 0, 0, VertexInputFormat::RGB32_SFLOAT },
		{ 0, 1, VertexInputFormat::RG32_SFLOAT },
		{ 0, 2, VertexInputFormat::RGB32_SFLOAT }
	};

	return s_vertexInput;
}

const std::vector<VertexInput>& Vertex_XYZ_UV_NT::getVertexInput()
{
	static const std::vector<VertexInput> s_vertexInput =
	{
		{ 0, 0, VertexInputFormat::RGB32_SFLOAT },
		{ 0, 1, VertexInputFormat::RG32_SFLOAT },
		{ 0, 2, VertexInputFormat::RGB32_SFLOAT },
		{ 0, 3, VertexInputFormat::RGB32_SFLOAT }
	};

	return s_vertexInput;
}

const std::vector<VertexInput>& Vertex_XYZ_UV_NTB::getVertexInput()
{
	static const std::vector<VertexInput> s_vertexInput =
	{
		{ 0, 0, VertexInputFormat::RGB32_SFLOAT },
		{ 0, 1, VertexInputFormat::RG32_SFLOAT },
		{ 0, 2, VertexInputFormat::RGB32_SFLOAT },
		{ 0, 3, VertexInputFormat::RGB32_SFLOAT },
		{ 0, 4, VertexInputFormat::RGB32_SFLOAT }
	};

	return s_vertexInput;
}
