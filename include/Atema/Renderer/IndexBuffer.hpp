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

#ifndef ATEMA_RENDERER_INDEX_BUFFER_HEADER
#define ATEMA_RENDERER_INDEX_BUFFER_HEADER

#include <Atema/Renderer/Config.hpp>
#include <Atema/Renderer/AbstractBuffer.hpp>
#include <Atema/Renderer/RendererDependent.hpp>
#include <Atema/Core/NonCopyable.hpp>
#include <Atema/Core/Ref.hpp>
#include <Atema/Storage/BufferStorage.hpp>

namespace at
{
	class ATEMA_RENDERER_API IndexBuffer : public virtual AbstractBuffer, public RendererDependent, public NonCopyable
	{
		public:
			static Ref<IndexBuffer> create(Renderer *renderer = nullptr);
			static Ref<IndexBuffer> create(const Ref<IndexBuffer>& buffer, Renderer *renderer = nullptr);
			static Ref<IndexBuffer> create(const IndexBufferStorage& storage, Renderer *renderer = nullptr);
			
		public:
			virtual ~IndexBuffer() = default;
			
			virtual void reset(const Ref<IndexBuffer>& buffer) = 0;
			virtual void reset(const IndexBufferStorage& storage) = 0;
			
			virtual void upload(const IndexBufferStorage& storage) = 0;
			virtual void download(IndexBufferStorage& storage) const = 0;
			
		protected:
			IndexBuffer() = default;
	};
}

#endif