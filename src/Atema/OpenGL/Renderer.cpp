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

#include <Atema/OpenGL/Renderer.hpp>
#include <Atema/OpenGL/Context.hpp>
#include <Atema/OpenGL/VertexBuffer.hpp>
#include <Atema/OpenGL/IndexBuffer.hpp>
#include <Atema/OpenGL/Texture.hpp>

namespace at
{
	OpenGLRenderer::OpenGLRenderer()
	{
		
	}
	
	OpenGLRenderer::~OpenGLRenderer()
	{
		
	}
	
	OpenGLRenderer* OpenGLRenderer::get_current()
	{
		return (nullptr);
	}
	
	bool OpenGLRenderer::is_compatible_with(const Renderer& renderer) const
	{
		return (false);
	}
	
	//Protected creation methods
	Ref<Context> OpenGLRenderer::create_context()
	{
		Ref<Context> ref(new OpenGLContext());
		
		return (ref);
	}
	
	Ref<VertexBuffer> OpenGLRenderer::create_vertex_buffer()
	{
		Ref<VertexBuffer> ref(new OpenGLVertexBuffer());
		
		return (ref);
	}
	
	Ref<IndexBuffer> OpenGLRenderer::create_index_buffer()
	{
		Ref<IndexBuffer> ref(new OpenGLIndexBuffer());
		
		return (ref);
	}
	
	Ref<Texture> OpenGLRenderer::create_texture()
	{
		Ref<Texture> ref(new OpenGLTexture());
		
		return (ref);
	}
}