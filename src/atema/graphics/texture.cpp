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

#include <atema/graphics/texture.hpp>
#include <atema/core/error.hpp>

using namespace std;

namespace at
{
	Texture::Texture() :
		m_id(0),
		m_tex_ok(false),
		m_filled(false),
		m_width(0),
		m_height(0),
		m_pixels(),
		m_internalformat(0),
		m_format(0),
		m_min_filter(filter::nearest),
		m_mag_filter(filter::nearest),
		m_fbo(0),
		m_rbo(0),
		m_framebuffer_ok(false)
	{
		ensure_texture();
	}
	
	Texture::Texture(unsigned int width, unsigned int height, Texture::filter min_filter, Texture::filter mag_filter)
	try :
		Texture()
	{
		create(width, height, min_filter, mag_filter);
	}
	catch (const Error& e)
	{
		glBindTexture(GL_TEXTURE_2D, 0);
		glDeleteTextures(1, &m_id);
		
		m_id = 0;
		
		throw;
	}
	
	Texture::~Texture()
	{
		glBindTexture(GL_TEXTURE_2D, 0);
		glDeleteTextures(1, &m_id);
	}
	
	bool Texture::is_valid() const noexcept
	{
		return (m_tex_ok && m_filled);
	}
	
	void Texture::create(unsigned int width, unsigned int height, Texture::filter min_filter, Texture::filter mag_filter)
	{
		try
		{
			ensure_texture();
			
			m_filled = false;
			
			glBindTexture(GL_TEXTURE_2D, m_id);
			
			m_pixels.resize(width*height);
			m_pixels.assign(width*height, Color(0.0f, 0.0f, 0.0f, 1.0f));
			
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0, GL_RGBA, GL_FLOAT, reinterpret_cast<float*>(m_pixels.data()));
			
			if (glGetError() != GL_NO_ERROR)
				ATEMA_ERROR("OpenGL internal error: Texture creation failed.")
			
			set_filters(min_filter, mag_filter);
			
			m_width = width;
			m_height = height;
			
			m_filled = true;
		}
		catch (const Error& e)
		{
			m_pixels.resize(0);
			
			throw;
		}
	}
	
	unsigned int Texture::get_width() const noexcept
	{
		return (m_width);
	}
	
	unsigned int Texture::get_height() const noexcept
	{
		return (m_height);
	}
	
	GLuint Texture::get_gl_id() const
	{
		return (m_id);
	}
	
	void Texture::set_filters(filter min_filter, filter mag_filter)
	{
		glBindTexture(GL_TEXTURE_2D, m_id);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLenum>(min_filter));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<GLenum>(mag_filter));
		
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	
	void Texture::to_cpu()
	{
		glBindTexture(GL_TEXTURE_2D, m_id);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, reinterpret_cast<float*>(m_pixels.data()));
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	
	void Texture::to_gpu()
	{
		glBindTexture(GL_TEXTURE_2D, m_id);
		// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, static_cast<GLsizei>(m_width), static_cast<GLsizei>(m_height), 0, GL_RGBA, GL_FLOAT, reinterpret_cast<float*>(m_pixels.data()));
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, static_cast<GLsizei>(m_width), static_cast<GLsizei>(m_height), GL_RGBA, GL_FLOAT, reinterpret_cast<float*>(m_pixels.data()));
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	
	Color& Texture::operator[](size_t index)
	{
		return (m_pixels[index]);
	}
	
	const Color& Texture::operator[](size_t index) const
	{
		return (m_pixels[index]);
	}
	
	void Texture::ensure_texture()
	{
		if (m_tex_ok)
			return;
		
		try
		{
			glGenTextures(1, &m_id);
			glBindTexture(GL_TEXTURE_2D, m_id);
			
			if (glIsTexture(m_id) == GL_FALSE || glGetError() != GL_NO_ERROR)
				ATEMA_ERROR("OpenGL could not create a texture")
			
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			
			glBindTexture(GL_TEXTURE_2D, 0);
			
			m_tex_ok = true;
		}
		catch (const Error& e)
		{
			glBindTexture(GL_TEXTURE_2D, 0);
			glDeleteTextures(1, &m_id);
			
			m_id = 0;
			
			m_tex_ok = false;
			
			throw;
		}
	}
	
	//RenderTarget
	GLuint Texture::get_gl_framebuffer_id() const
	{
		const_cast<Texture*>(this)->ensure_framebuffer_exists();
		
		return (m_fbo);
	}
	
	void Texture::ensure_framebuffer_exists()
	{
		if (m_framebuffer_ok)
			return;
		
		if (!is_valid())
			ATEMA_ERROR("The texture is invalid.")
		
		try
		{
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			
			if (glIsFramebuffer(m_fbo) != GL_FALSE)
				glDeleteFramebuffers(1, &m_fbo);
			
			if (glIsRenderbuffer(m_rbo) != GL_FALSE)
				glDeleteRenderbuffers(1, &m_rbo);
			
			m_fbo = 0;
			m_rbo = 0;
			
			glGenFramebuffers(1, &m_fbo);
			glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
			
			if ((glIsFramebuffer(m_fbo) == GL_FALSE) || (glGetError() != GL_NO_ERROR))
				ATEMA_ERROR("OpenGL internal error: FBO creation failed.")
			
			glGenRenderbuffers(1, &m_rbo);
			glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
			
			if ((glIsRenderbuffer(m_rbo) == GL_FALSE) || (glGetError() != GL_NO_ERROR))
				ATEMA_ERROR("OpenGL internal error: RBO creation failed.")
			
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, static_cast<GLsizei>(get_width()), static_cast<GLsizei>(get_height()));
			
			if (glGetError() != GL_NO_ERROR)
				ATEMA_ERROR("OpenGL internal error: RBO allocation failed.")
			
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rbo);
			
			if (glGetError() != GL_NO_ERROR)
				ATEMA_ERROR("OpenGL internal error: FBO allocation failed.")
			
			glBindTexture(GL_TEXTURE_2D, m_id);
			
			// glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_id, 0);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_id, 0);
			
			// cout << get_width() << ", " << get_height() << " : tex " << get_gl_id() << "fbo : " << m_fbo << endl;
			//TODO: fix this
			if ((glGetError() != GL_NO_ERROR) || (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE))
			{
				switch(glCheckFramebufferStatus(GL_FRAMEBUFFER))
				{
					case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT :
						ATEMA_ERROR("OpenGL internal error: FBO attachment is incomplete.")
					case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT :
						ATEMA_ERROR("OpenGL internal error: FBO has no attached texture.")
					case GL_FRAMEBUFFER_UNSUPPORTED :
						ATEMA_ERROR("OpenGL internal error: FBO is incompatible with texture format.")
					default :
						ATEMA_ERROR("OpenGL internal error: FBO configuration failed.")
				}
			}
			
			glDrawBuffer(GL_COLOR_ATTACHMENT0);
			
			if (glGetError() != GL_NO_ERROR)
				ATEMA_ERROR("OpenGL internal error.")
			
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glBindTexture(GL_TEXTURE_2D, 0);
			
			m_framebuffer_ok = true;
		}
		catch (const Error& e)
		{
			glBindTexture(GL_TEXTURE_2D, 0);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			
			glDeleteFramebuffers(1, &m_fbo);
			glDeleteRenderbuffers(1, &m_rbo);
			
			m_fbo = 0;
			m_rbo = 0;
			
			throw;
		}
	}
}