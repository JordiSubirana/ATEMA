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

#ifndef ATEMA_OPENGL_VERTEX_BUFFER_HEADER
#define ATEMA_OPENGL_VERTEX_BUFFER_HEADER

#include <Atema/OpenGL/Config.hpp>
#include <Atema/OpenGL/Buffer.hpp>
#include <Atema/Renderer/VertexBuffer.hpp>

namespace at
{
	class OpenGLRenderer;
	
	class ATEMA_OPENGL_API OpenGLVertexBuffer : public VertexBuffer, public OpenGLBuffer
	{
		friend class OpenGLRenderer;
		
		public:
			~OpenGLVertexBuffer();
			
			void reset(const Ref<VertexBuffer>& buffer);
			void reset(const void *data, size_t size, const VertexFormat& format);
			
			const VertexFormat& get_vertex_format() const;
			
		private:
			OpenGLVertexBuffer();
			void upload(const void *data, size_t size, const VertexFormat& format);
			void download(void *data) const;
			
			VertexFormat m_format;
	};
}

#endif