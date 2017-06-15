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

#ifndef ATEMA_OPENGL_CONTEXTIMPL_WIN32_HPP
#define ATEMA_OPENGL_CONTEXTIMPL_WIN32_HPP

#include <Atema/Config.hpp>

#ifdef ATEMA_SYSTEM_WINDOWS

#include <Atema/OpenGL/Config.hpp>
#include <Atema/OpenGL/Context.hpp>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#undef NOMINMAX

namespace at
{
	struct OpenGLContext::Handle
	{
		Handle() : deviceContext(nullptr), context(nullptr) {}

		bool operator<(const Handle& other) const noexcept
		{
			return false; // We do not need sorting
		}

		bool isValid() const noexcept
		{
			return deviceContext && context;
		}

		HDC deviceContext;
		HGLRC context;
	};
	
	class OpenGLContext::Impl
	{
	public:
		Impl();
		virtual ~Impl();

		void makeCurrent(bool current);
		bool isCurrent() const;

		static Handle getCurrent();

	private:
		Handle m_handle;
	};
}

#endif

#endif