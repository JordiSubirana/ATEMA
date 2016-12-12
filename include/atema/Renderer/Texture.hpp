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

#ifndef ATEMA_RENDERER_TEXTURE_HEADER
#define ATEMA_RENDERER_TEXTURE_HEADER

#include <Atema/Renderer/Config.hpp>
#include <Atema/Renderer/RendererDependent.hpp>
#include <Atema/Core/NonCopyable.hpp>
#include <Atema/Core/Ref.hpp>
#include <Atema/Storage/TextureStorage.hpp>

namespace at
{
	class ATEMA_RENDERER_API Texture : public RendererDependent, public NonCopyable
	{
		public:
			static Ref<Texture> create(Renderer *renderer = nullptr);
			static Ref<Texture> create(const Ref<Texture>& texture, Renderer *renderer = nullptr);
			static Ref<Texture> create(const TextureStorage& storage, Renderer *renderer = nullptr);
			static Ref<Texture> create(unsigned int width, unsigned int height, Renderer *renderer = nullptr);
			
		public:
			virtual ~Texture() = default;
			
			virtual void reset(const Ref<Texture>& texture) = 0;
			virtual void reset(const TextureStorage& storage) = 0;
			virtual void reset(unsigned int width, unsigned int height) = 0;
			
			virtual void upload(const TextureStorage& storage) = 0;
			virtual void download(TextureStorage& storage) const = 0;
			
			virtual unsigned int get_width() const = 0;
			virtual unsigned int get_height() const = 0;
			
		protected:
			Texture() = default;
	};
}

#endif