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

#ifndef ATEMA_OPENGL_BUFFER_HEADER
#define ATEMA_OPENGL_BUFFER_HEADER

#include <Atema/OpenGL/Config.hpp>
#include <Atema/Renderer/AbstractBuffer.hpp>

namespace at
{
	class ATEMA_OPENGL_API OpenGLBuffer : public virtual AbstractBuffer
	{
		public:
			~OpenGLBuffer();
			
			bool is_mapped() const;
			
			void* map();
			void unmap() const;
			
			size_t get_byte_size() const noexcept;
			
		protected:
			OpenGLBuffer() = delete;
			OpenGLBuffer(int usage);
			
			void set_data(const void *data, size_t size, int update_mode);
			void get_data(void *data) const;
			
			unsigned int m_id;
			
		private:
			int m_usage;
			int m_update_mode;
			size_t m_byte_size;
			mutable void* m_mapped_data;
	};
}

#endif