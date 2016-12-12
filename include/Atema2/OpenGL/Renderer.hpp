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

#ifndef ATEMA_OPENGL_RENDERER_HEADER
#define ATEMA_OPENGL_RENDERER_HEADER

#include <Atema/OpenGL/Config.hpp>
#include <Atema/Renderer/Renderer.hpp>

namespace at
{	
	class OpenGLContext;
	class OpenGLVertexBuffer;
	class OpenGLIndexBuffer;
	class OpenGLTexture;
	
	class ATEMA_OPENGL_API OpenGLRenderer : public Renderer
	{
		public:
			OpenGLRenderer();
			virtual ~OpenGLRenderer();
			
			static OpenGLRenderer* get_current();
			
			bool is_compatible_with(const Renderer& renderer) const;
			
		protected:
			Ref<Context> create_context();
			Ref<VertexBuffer> create_vertex_buffer();
			Ref<IndexBuffer> create_index_buffer();
			Ref<Texture> create_texture();
	};
}

#endif