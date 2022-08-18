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

#ifndef ATEMA_GRAPHICS_ENUMS_HPP
#define ATEMA_GRAPHICS_ENUMS_HPP

#include <Atema/Graphics/Config.hpp>
#include <Atema/Core/Flags.hpp>

namespace at
{
	enum class TextureUsage
	{
		None = 0,
		Sampled = 1 << 0,
		Input = 1 << 1,
		Output = 1 << 3,
		Depth = 1 << 4,
		Clear = 1 << 5,

		Read = Sampled | Input,
		Write = Output | Depth | Clear,
	};

	ATEMA_DECLARE_FLAGS(TextureUsage);

	enum class VertexComponentType
	{
		Position,
		Color,
		TexCoords,
		Normal,
		Tangent,
		Bitangent,
		UserData
		//TODO: Add skeletal relative components
	};

	enum class DefaultVertexFormat
	{
		Pos2D,
		Pos2D_Color,
		Pos2D_TexCoords,
		Pos3D,
		Pos3D_Color,
		Pos3D_Color_Normal,
		Pos3D_Color_Normal_Tangent_Bitangent,
		Pos3D_TexCoords,
		Pos3D_TexCoords_Normal,
		Pos3D_TexCoords_Normal_Tangent_Bitangent
	};
}

#endif
