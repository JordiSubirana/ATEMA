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

#ifndef ATEMA_CONTEXT_RENDER_TARGET_HEADER
#define ATEMA_CONTEXT_RENDER_TARGET_HEADER

#include <atema/context/config.hpp>
#include <atema/context/opengl.hpp>
#include <atema/graphics/color.hpp>
#include <atema/utility/flags.hpp>
#include <atema/math/rect.hpp>
#include <atema/context/object_gl.hpp>

namespace at
{
	class ATEMA_CONTEXT_API RenderTarget : public ObjectGL
	{
		public:
			enum class clear_targets : Flags
			{
				color	= GL_COLOR_BUFFER_BIT,
				depth	= GL_DEPTH_BUFFER_BIT,
				all		= GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
			};
			
		public:
			RenderTarget();
			virtual ~RenderTarget() noexcept = default;
			
			virtual bool is_current_render_target() const;
			virtual void make_current(bool value);
			
			virtual GLuint get_gl_framebuffer_id() const = 0;
			
			virtual unsigned int get_width() const = 0;
			virtual unsigned int get_height() const = 0;
			
			void draw(const RenderTarget& render_target);
			
			void set_viewport(const Rect& rect);
			Rect& get_viewport() noexcept;
			const Rect& get_viewport() const noexcept;
			
			void set_clear_color(const Color& color) noexcept;
			Color& get_clear_color() noexcept;
			const Color& get_clear_color() const noexcept;
			
			void clear(Flags targets = static_cast<Flags>(clear_targets::all));
			
		protected:
			virtual void ensure_framebuffer_exists() = 0;
			
		private:
			Rect m_rect;
			Color m_clear_color;
	};
}

ATEMA_ENUM_FLAGS(at::RenderTarget::clear_targets)

#endif