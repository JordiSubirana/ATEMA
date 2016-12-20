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

#include <Atema/OpenGL/Texture.hpp>
#include <Atema/Core/Error.hpp>

#include <glad/glad.h>

namespace at
{
	OpenGLTexture::OpenGLTexture()
	{
		glGenTextures(1, &m_id);
		glBindTexture(GL_TEXTURE_2D, m_id);
		
		if (glIsTexture(m_id) == GL_FALSE)
			ATEMA_ERROR("OpenGL could not create a texture")
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT); //GL_CLAMP_TO_EDGE - GL_REPEAT
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);
		
		glBindTexture(GL_TEXTURE_2D, 0);
		
		reset(2, 2);
	}
	
	OpenGLTexture::~OpenGLTexture()
	{
		glBindTexture(GL_TEXTURE_2D, 0);
		glDeleteTextures(1, &m_id);
	}
	
	void OpenGLTexture::reset(const Ref<Texture>& texture)
	{
		TextureStorage storage;
		
		texture->download(storage);
		
		reset(storage);
	}
	
	void OpenGLTexture::reset(const TextureStorage& storage)
	{
		m_width = storage.width;
		m_height = storage.height;
		
		glBindTexture(GL_TEXTURE_2D, m_id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, static_cast<GLsizei>(m_width), static_cast<GLsizei>(m_height), 0, GL_RGBA, GL_FLOAT, reinterpret_cast<const float*>(storage.pixels.data()));
	}
	
	void OpenGLTexture::reset(unsigned int width, unsigned int height)
	{
		m_width = width;
		m_height = height;
		
		glBindTexture(GL_TEXTURE_2D, m_id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, static_cast<GLsizei>(m_width), static_cast<GLsizei>(m_height), 0, GL_RGBA, GL_FLOAT, nullptr);
	}
	
	void OpenGLTexture::upload(const TextureStorage& storage)
	{
		reset(storage);
	}
	
	void OpenGLTexture::download(TextureStorage& storage) const
	{
		storage.width = m_width;
		storage.height = m_height;
		
		storage.pixels.resize(m_width*m_height);
		
		glBindTexture(GL_TEXTURE_2D, m_id);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, reinterpret_cast<float*>(storage.pixels.data()));
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	
	unsigned int OpenGLTexture::get_width() const
	{
		return (m_width);
	}
	
	unsigned int OpenGLTexture::get_height() const
	{
		return (m_height);
	}
	
	unsigned int OpenGLTexture::get_gl_id() const
	{
		return (m_id);
	}
}