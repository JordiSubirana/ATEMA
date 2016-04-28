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

#include <atema/core/Error.hpp>

namespace at
{
	Texture::Texture()
	try :
		m_id(0),
		m_width(0),
		m_height(0),
		m_pixels(),
		m_internalformat(0),
		m_format(0),
		m_min_filter(filter::nearest),
		m_mag_filter(filter::nearest)
	{
		glGenTextures(1, &m_id);
		
		if (!glIsTexture(m_id) || glGetError() != GL_NO_ERROR)
			ATEMA_ERROR("OpenGL could not create a texture")
		
		glBindTexture(GL_TEXTURE_2D, m_id);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	catch (const Error& e)
	{
		glBindTexture(GL_TEXTURE_2D, 0);
		glDeleteTextures(1, &m_id);
		
		m_id = 0;
		
		throw;
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
	
	void Texture::create(unsigned int width, unsigned int height, Texture::filter min_filter, Texture::filter mag_filter)
	{
		m_pixels.reserve(width*height);
		
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, static_cast<int>(width), static_cast<int>(height), 0, GL_RGBA, GL_FLOAT, nullptr);
		
		set_filters(min_filter, mag_filter);
		
		m_width = width;
		m_height = height;
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
	
	void Texture::download()
	{
		glBindTexture(GL_TEXTURE_2D, m_id);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, reinterpret_cast<float*>(m_pixels.data()));
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	
	void Texture::upload()
	{
		glBindTexture(GL_TEXTURE_2D, m_id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_FLOAT, reinterpret_cast<float*>(m_pixels.data()));
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
}