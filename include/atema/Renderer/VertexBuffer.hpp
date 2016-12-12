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

#ifndef ATEMA_RENDERER_VERTEX_BUFFER_HEADER
#define ATEMA_RENDERER_VERTEX_BUFFER_HEADER

#include <Atema/Renderer/Config.hpp>
#include <Atema/Renderer/AbstractBuffer.hpp>
#include <Atema/Renderer/RendererDependent.hpp>
#include <Atema/Renderer/Vertex.hpp>
#include <Atema/Core/NonCopyable.hpp>
#include <Atema/Core/Ref.hpp>
#include <Atema/Storage/BufferStorage.hpp>

namespace at
{
	class ATEMA_RENDERER_API VertexBuffer : public AbstractBuffer, public RendererDependent, public NonCopyable
	{
		public:
			static Ref<VertexBuffer> create(Renderer *renderer = nullptr);
			static Ref<VertexBuffer> create(const Ref<VertexBuffer>& buffer, Renderer *renderer = nullptr);
			static Ref<VertexBuffer> create(void *data, size_t size, const VertexFormat& format, Renderer *renderer = nullptr);
			template <typename T>
			static Ref<VertexBuffer> create(const VertexBufferStorage<T>& storage, const VertexFormat& format = VertexFormat::get<T>(), Renderer *renderer = nullptr);
			
		public:
			virtual ~VertexBuffer() = default;
			
			virtual void reset(const Ref<VertexBuffer>& buffer) = 0;
			virtual void reset(void *data, size_t size, const VertexFormat& format) = 0;
			template <typename T>
			void reset(const VertexBufferStorage<T>& storage, const VertexFormat& format = VertexFormat::get<T>());
			
			template <typename T>
			void upload(const VertexBufferStorage<T>& storage);
			template <typename T>
			void download(VertexBufferStorage<T>& storage) const;
			
			virtual const VertexFormat& get_vertex_format() const = 0;
			virtual size_t get_size() const = 0;
			
		protected:
			VertexBuffer() = default;
			virtual void upload(void *data, size_t size, const VertexFormat& format) = 0;
			virtual void download(void *data) const = 0;
	};
}

#include <Atema/Renderer/VertexBuffer.tpp>

#endif