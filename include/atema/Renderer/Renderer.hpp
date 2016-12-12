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

#ifndef ATEMA_RENDERER_RENDERER_HEADER
#define ATEMA_RENDERER_RENDERER_HEADER

#include <Atema/Renderer/Config.hpp>
#include <Atema/Core/NonCopyable.hpp>
#include <Atema/Core/Ref.hpp>
#include <Atema/Math/Vector.hpp>

namespace at
{
	class Context;
	class VertexBuffer;
	class IndexBuffer;
	class Texture;
	
	class ATEMA_RENDERER_API Renderer : public NonCopyable
	{
		friend class Context;
		friend class VertexBuffer;
		friend class IndexBuffer;
		friend class Texture;
		
		public:
			virtual ~Renderer();
			
			static Renderer* get_current();
			
			bool is_current();
			
			virtual void set_current();
			virtual bool is_compatible_with(const Renderer& renderer) const;
			
		protected:
			Renderer();
			
			virtual Ref<Context> create_context() = 0;
			virtual Ref<VertexBuffer> create_vertex_buffer() = 0;
			virtual Ref<IndexBuffer> create_index_buffer() = 0;
			virtual Ref<Texture> create_texture() = 0;
			
		private:			
			bool m_is_current;
	};
}

#endif