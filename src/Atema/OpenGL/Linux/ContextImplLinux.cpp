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

#include "ContextImplLinux.hpp"

#ifdef ATEMA_SYSTEM_LINUX

namespace at
{
	OpenGLContext::Impl::Impl(Handle handle) : m_handle(handle);
	{
	}

	OpenGLContext::Impl::~Impl()
	{
	}

	void OpenGLContext::Impl::makeCurrent(bool current)
	{
		if (current)
			glXMakeCurrent(m_handle.display, m_handle.drawable, m_handle.context);
		else
			glXMakeCurrent(m_handle.display, None, nullptr);
	}

	bool OpenGLContext::Impl::isCurrent() const
	{
		return glXGetCurrentContext() == m_handle.context;
	}

	OpenGLContext::Handle OpenGLContext::Impl::getCurrent()
	{
		Handle handle;

		handle.display = glXGetCurrentDisplay();
		handle.drawable = glXGetCurrentDrawable();
		handle.context = glXGetCurrentContext();

		return handle;
	}
}

#endif