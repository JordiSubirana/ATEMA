// ----------------------------------------------------------------------
// Copyright (C) 2016 Jordi SUBIRANA
//
// This file is part of ATEMA.
//
// ATEMA is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ATEMA is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ATEMA.  If not, see <http://www.gnu.org/licenses/>.
// ----------------------------------------------------------------------

//TODO: Check return value of windows functions
#include "ContextImpl_Win32.hpp"

#if defined(ATEMA_SYSTEM_WINDOWS)

#include <Atema/Core/Error.hpp>

#include <cstring> //memset
#include <mutex>

#include <glad/glad_wgl.h>

namespace at
{	
	OpenGLContext::Impl::Impl() :
		m_device_context(nullptr),
		m_context(nullptr),
		m_window(nullptr),
		m_owns_window(false)
	{
		
	}

	OpenGLContext::Impl::~Impl()
	{
		if (m_context)
		{
			if (wglGetCurrentContext() == m_context)
				wglMakeCurrent(nullptr, nullptr);
			
			wglDeleteContext(m_context);
		}
		
		if (m_device_context)
			ReleaseDC(m_window, m_device_context);
		
		if (m_owns_window)
			DestroyWindow(m_window);
	}

	void OpenGLContext::Impl::reset(unsigned int w, unsigned int h, const Context::Settings& settings)
	{
		m_window = CreateWindowA("STATIC", nullptr, WS_DISABLED | WS_POPUP, 0, 0, 1, 1, nullptr, nullptr, GetModuleHandle(nullptr), nullptr);
		if (!m_window)
			ATEMA_ERROR("Window creation failed.")

		ShowWindow(m_window, SW_HIDE);
		m_owns_window = true;
		
		reset(settings);
	}

	void OpenGLContext::Impl::reset(WindowHandle handle, const Context::Settings& settings)
	{
		m_window = static_cast<HWND>(handle);
		m_owns_window = false;
		
		reset(settings);
	}
	
	void OpenGLContext::Impl::reset(const Context::Settings& requested_settings)
	{
		Context::Settings settings = requested_settings;
		int pixel_format;
		HGLRC tmp_context;
		
		PIXELFORMATDESCRIPTOR descriptor;
		memset(&descriptor, 0, sizeof(PIXELFORMATDESCRIPTOR));
		descriptor.nSize = sizeof(PIXELFORMATDESCRIPTOR);
		descriptor.nVersion = 1;
		
		descriptor.cColorBits = settings.bits_per_pixel;
		descriptor.cDepthBits = settings.depth_bits;
		descriptor.cStencilBits = 0; //settings.stencil_bits;
		descriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		descriptor.iPixelType = PFD_TYPE_RGBA;
		
		if (settings.bits_per_pixel == 32)
			descriptor.cAlphaBits = 8;
		
		//Default OpenGL version : 3.3
		if (settings.version_major <= 0 || settings.version_minor <= 0)
		{
			settings.version_major = 3;
			settings.version_minor = 3;
		}
		
		m_device_context = GetDC(m_window);
		
		if (!m_device_context)
			ATEMA_ERROR("Invalid device context.")
		
		pixel_format = ChoosePixelFormat(m_device_context, &descriptor);
		
		if (!pixel_format)
			ATEMA_ERROR("Pixel format choice failed.")
		
		if (!SetPixelFormat(m_device_context, pixel_format, &descriptor))
			ATEMA_ERROR("Pixel format choice failed.")
		
		tmp_context = wglCreateContext(m_device_context);
		wglMakeCurrent(m_device_context, tmp_context);
		
		if (!gladLoadWGL(m_device_context))
			ATEMA_ERROR("Loading of WGL extensions failed.")
		
		wglMakeCurrent(nullptr, nullptr);
		wglDeleteContext(tmp_context);
		
		//Update of context settings about pixel format (needed?)
		/*
		if (DescribePixelFormat(m_device_context, pixel_format, sizeof(PIXELFORMATDESCRIPTOR), &descriptor) != 0)
		{
			settings.bitsPerPixel = descriptor.cColorBits + descriptor.cAlphaBits;
			settings.depthBits = descriptor.cDepthBits;
			settings.stencilBits = descriptor.cDepthBits;
		}//*/
		
		HGLRC shared_context = nullptr;
		if (settings.shared_context)
		{
			const OpenGLContext *context = dynamic_cast<const OpenGLContext*>(settings.shared_context.get());
			
			if (!context || !context->m_impl || !context->m_impl->m_context)
				ATEMA_ERROR("No OpenGL context to share.")
			
			shared_context = context->m_impl->m_context;
		}
		
		if (wglCreateContextAttribsARB)
		{
			int attributes[] =
			{
				WGL_CONTEXT_MAJOR_VERSION_ARB,	settings.version_major,
				WGL_CONTEXT_MINOR_VERSION_ARB,	settings.version_minor,
				WGL_CONTEXT_PROFILE_MASK_ARB,	WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
				0 //end of attributes
			};
			
			m_context = wglCreateContextAttribsARB(m_device_context, shared_context, attributes);
		}
		
		if (!m_context)
		{
			m_context = wglCreateContext(m_device_context);

			if (shared_context)
			{
				static std::mutex mutex;
				std::lock_guard<std::mutex> locker(mutex);

				if (!wglShareLists(shared_context, m_context))
					ATEMA_ERROR("Sharing OpenGL context failed.")
			}
		}
		
		if (!m_context)
			ATEMA_ERROR("OpenGL context creation failed.")
		
		set_current(true);
	}
	
	void OpenGLContext::Impl::set_current(bool current)
	{
		if (current)
			wglMakeCurrent(m_device_context, m_context);
		else
			wglMakeCurrent(nullptr, nullptr);
	}

	void OpenGLContext::Impl::swap_buffers()
	{
		if (!SwapBuffers(m_device_context))
			ATEMA_ERROR("Swapping buffers failed.")
	}
}

#endif