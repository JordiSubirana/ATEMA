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

#ifndef ATEMA_OPENGL_CONTEXTIMPL_LINUX_HPP
#define ATEMA_OPENGL_CONTEXTIMPL_LINUX_HPP

#include <Atema/Config.hpp>
#ifdef ATEMA_SYSTEM_LINUX

#include <Atema/OpenGL/Config.hpp>
#include <Atema/OpenGL/Context.hpp>

#include <glad/glad_glx.h>

namespace at
{
	struct OpenGLContext::Handle
	{
		Handle() : display(nullptr), drawable(nullptr), context(nullptr) {}

		bool operator<(const Handle& other) const noexcept
		{
			return false; // We do not need sorting
		}

		bool isValid() const noexcept
		{
			return display && drawable && context;
		}

		Display *display;
		GLXDrawable drawable;
		GLXContext context;
	};

	class OpenGLContext::Impl
	{
	public:
		Impl() = delete;
		explicit Impl(Handle handle);
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