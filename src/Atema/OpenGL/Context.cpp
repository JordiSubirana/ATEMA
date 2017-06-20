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

#include <Atema/OpenGL/Context.hpp>
#include <Atema/Core/Error.hpp>
#include <Atema/OpenGL/RenderTarget.hpp>

#ifdef ATEMA_SYSTEM_WINDOWS
#include "Win32/ContextImplWin32.hpp"
#include <glad/glad.h>
#elif defined ATEMA_SYSTEM_LINUX
#include "Linux/ContextImplLinux.hpp"
#include <glad/glad.h>
#else
#error Operating system not handled by this module.
#endif

namespace at
{
	std::map<OpenGLContext::Handle, std::unique_ptr<OpenGLContext>> OpenGLContext::s_contexts;

	OpenGLContext::OpenGLContext(Handle handle) : m_impl(new Impl(handle))
	{
		if (!gladLoadGL())
			ATEMA_ERROR("Failed to load OpenGL functions.");
	}

	OpenGLContext::~OpenGLContext()
	{
		m_impl->makeCurrent(true);

		invalidate();

		deleteResources();

		delete m_impl;
	}

	void OpenGLContext::makeCurrent(bool current)
	{
		m_impl->makeCurrent(current);

		if (current)
			deleteResources();
	}

	bool OpenGLContext::isCurrent() const
	{
		return m_impl->isCurrent();
	}

	void OpenGLContext::deleteVAO(unsigned vaoId)
	{
		m_deletedVaos.push_back(vaoId);
		deleteResources();
	}

	void OpenGLContext::deleteFBO(unsigned fboId)
	{
		m_deletedFbos.push_back(fboId);
		deleteResources();
	}

	OpenGLContext* OpenGLContext::getCurrent()
	{
		auto handle = Impl::getCurrent();

		if (!handle.isValid())
			return nullptr;

		auto it = s_contexts.find(handle);

		if (it != s_contexts.end())
			return it->second.get();

		auto context = new OpenGLContext(handle);

		s_contexts[handle].reset(context);

		return context;
	}

	void OpenGLContext::deleteResources()
	{
		if (m_impl->isCurrent())
		{
			for (auto vao : m_deletedVaos)
			{
				if (glIsVertexArray(vao) == GL_TRUE)
					glDeleteVertexArrays(1, &vao);
			}

			m_deletedVaos.clear();

			for (auto fbo : m_deletedFbos)
			{
				if (glIsFramebuffer(fbo) == GL_TRUE)
					glDeleteFramebuffers(1, &fbo);
			}

			m_deletedFbos.clear();
		}
	}
}
