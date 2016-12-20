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

#ifndef ATEMA_OPENGL_TEXTURE_HEADER
#define ATEMA_OPENGL_TEXTURE_HEADER

#include <Atema/OpenGL/Config.hpp>
#include <Atema/Renderer/Texture.hpp>

namespace at
{	
	class OpenGLRenderer;
	
	class ATEMA_OPENGL_API OpenGLTexture : public Texture
	{
		friend class OpenGLRenderer;
		
		public:
			~OpenGLTexture();
			
			void reset(const Ref<Texture>& texture);
			void reset(const TextureStorage& storage);
			void reset(unsigned int width, unsigned int height);
			
			void upload(const TextureStorage& storage);
			void download(TextureStorage& storage) const;
			
			unsigned int get_width() const;
			unsigned int get_height() const;
			
			unsigned int get_gl_id() const;
			
		private:
			OpenGLTexture();
			
			unsigned int m_id;
			unsigned int m_width;
			unsigned int m_height;
	};
}

#endif