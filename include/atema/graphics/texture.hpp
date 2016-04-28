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

#ifndef ATEMA_GRAPHICS_TEXTURE_HEADER
#define ATEMA_GRAPHICS_TEXTURE_HEADER

#include <atema/graphics/config.hpp>
#include <atema/context/opengl.hpp>
#include <atema/graphics/color.hpp>
#include <atema/graphics/shared_object.hpp>

#include <vector>

namespace at
{
	class ATEMA_GRAPHICS_API Texture : public SharedObject
	{
		public:
			enum class filter : GLenum
			{
				nearest = GL_NEAREST,
				linear = GL_LINEAR
			};
		
		public:
			Texture();
			Texture(unsigned int width, unsigned int height, filter min_filter = filter::nearest, filter mag_filter = filter::nearest);
			virtual ~Texture();
			
			void create(unsigned int width, unsigned int height, filter min_filter = filter::nearest, filter mag_filter = filter::nearest);
			
			unsigned int get_width() const noexcept;
			unsigned int get_height() const noexcept;
			
			GLuint get_gl_id() const;
			
			void set_filters(filter min_filter, filter mag_filter);
			
			void download();
			void upload();
			
			Color& operator[](size_t index);
			const Color& operator[](size_t index) const;
			
		private:
			GLuint m_id;
			unsigned int m_width;
			unsigned int m_height;
			
			std::vector<Color> m_pixels;
			
			GLint m_internalformat;
			GLenum m_format;
			
			filter m_min_filter;
			filter m_mag_filter;
	};
}

#endif