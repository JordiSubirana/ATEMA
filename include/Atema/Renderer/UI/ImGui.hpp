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

#ifndef ATEMA_RENDERER_IMGUI_HPP
#define ATEMA_RENDERER_IMGUI_HPP

#include <Atema/Renderer/Config.hpp>
#include <Atema/Core/Error.hpp>
#include <Atema/Math/Vector.hpp>

#define IM_ASSERT(at_expr)  ATEMA_ASSERT(at_expr)

#define IMGUI_API ATEMA_RENDERER_API

#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS
#define IMGUI_DISABLE_OBSOLETE_KEYIO

#define IM_VEC2_CLASS_EXTRA \
	constexpr ImVec2(const at::Vector2f& f) : x(f.x), y(f.y) {} \
	operator at::Vector2f() const { return at::Vector2f(x, y); }

#define IM_VEC4_CLASS_EXTRA                                                     \
        constexpr ImVec4(const at::Vector4f& f) : x(f.x), y(f.y), z(f.z), w(f.w) {}   \
        operator at::Vector4f() const { return at::Vector4f(x, y, z, w); }

#include <imgui/imgui.h>

#endif
