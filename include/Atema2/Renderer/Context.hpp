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

#ifndef ATEMA_RENDERER_CONTEXT_HEADER
#define ATEMA_RENDERER_CONTEXT_HEADER

#include <Atema/Renderer/Config.hpp>
#include <Atema/Renderer/RendererDependent.hpp>
#include <Atema/Core/NonCopyable.hpp>
#include <Atema/Core/Ref.hpp>
#include <Atema/Core/WindowHandle.hpp>

#include <cstdint>

namespace at
{	
	class ATEMA_RENDERER_API Context : public RendererDependent, public NonCopyable
	{
		public:
			struct Settings
			{
				int8_t version_major;
				int8_t version_minor;
				uint8_t bits_per_pixel;
				uint8_t depth_bits;
				Ref<Context> shared_context;
				
				Settings() :
					version_major(-1),
					version_minor(-1),
					bits_per_pixel(32),
					depth_bits(24),
					shared_context()
				{
					
				}
			};
			
			static Ref<Context> create(unsigned int w, unsigned int h, const Context::Settings& settings = Context::Settings(), Renderer *renderer = nullptr);
			static Ref<Context> create(WindowHandle handle, const Context::Settings& settings = Context::Settings(), Renderer *renderer = nullptr);
			
		public:
			virtual ~Context() = default;
			
			virtual void reset(unsigned int w, unsigned int h, const Context::Settings& settings = Context::Settings()) = 0;
			virtual void reset(WindowHandle handle, const Context::Settings& settings = Context::Settings()) = 0;
			
			virtual void set_current(bool current = true) = 0;
			
			virtual void swap_buffers() = 0;
			
		protected:
			Context() = default;
	};
}

#endif