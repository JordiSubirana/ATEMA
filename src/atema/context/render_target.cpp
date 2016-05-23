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

#include <atema/context/render_target.hpp>
#include <atema/core/error.hpp>

namespace at
{
	thread_local RenderTarget *t_current_render_target = nullptr;
	thread_local GLint t_current_viewport[4];
		
	RenderTarget::RenderTarget() :
		m_rect{0, 0, 0, 0},
		m_clear_color(0.0f, 0.0f, 0.0f, 1.0f)
	{
		
	}
	
	bool RenderTarget::is_current_render_target() const
	{
		return (this == t_current_render_target);
	}
	
	void RenderTarget::make_current(bool value)
	{
		if (value)
		{
			this->ensure_framebuffer_exists();
			
			glBindFramebuffer(GL_FRAMEBUFFER, this->get_gl_framebuffer_id());
			glViewport(m_rect.x, m_rect.y, m_rect.w, m_rect.h);
			
			t_current_render_target = this;
			glGetIntegerv(GL_VIEWPORT, t_current_viewport);
		}
		else
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	}
	
	void RenderTarget::blit(const RenderTarget &render_target)
	{
		//TODO: Ensure framebuffers exist
		this->ensure_framebuffer_exists();
		render_target.get_gl_framebuffer_id();
		
		GLuint current = (t_current_render_target ? t_current_render_target->get_gl_framebuffer_id() : 0);
		const Rect& t_rect = render_target.m_rect;
		GLuint t_id = render_target.get_gl_framebuffer_id();
		GLuint m_id = this->get_gl_framebuffer_id();
		
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, this->get_gl_framebuffer_id());
		if (m_id == 0)
			glDrawBuffer(GL_BACK);
		else
			glDrawBuffer(GL_COLOR_ATTACHMENT0);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, render_target.get_gl_framebuffer_id());
		//TODO: Check parameter
		if (t_id == 0)
			glReadBuffer(GL_BACK);
		else
			glReadBuffer(GL_COLOR_ATTACHMENT0);

		glBlitFramebuffer(	t_rect.x, t_rect.y, t_rect.w, t_rect.h,
							m_rect.x, m_rect.y, m_rect.w, m_rect.h,
							GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
							GL_NEAREST);
		
		glBindFramebuffer(GL_FRAMEBUFFER, current);
	}
	
	void RenderTarget::set_viewport(const Rect& rect)
	{
		m_rect = rect;
	}
	
	Rect& RenderTarget::get_viewport() noexcept
	{
		return (m_rect);
	}
	
	const Rect& RenderTarget::get_viewport() const noexcept
	{
		return (m_rect);
	}
	
	void RenderTarget::set_clear_color(const Color& color) noexcept
	{
		m_clear_color = color;
	}
	
	Color& RenderTarget::get_clear_color() noexcept
	{
		return (m_clear_color);
	}
	
	const Color& RenderTarget::get_clear_color() const noexcept
	{
		return (m_clear_color);
	}
	
	void RenderTarget::clear(Flags targets)
	{
		this->ensure_framebuffer_exists();
		
		GLuint current_id = (t_current_render_target ? t_current_render_target->get_gl_framebuffer_id() : 0);
		
		glGetIntegerv(GL_VIEWPORT, t_current_viewport);
		
		glBindFramebuffer(GL_FRAMEBUFFER, this->get_gl_framebuffer_id());
		glViewport(m_rect.x, m_rect.y, m_rect.w, m_rect.h);
		
		glClearColor(m_clear_color.r, m_clear_color.g, m_clear_color.b, m_clear_color.a);
		
		// glClear(static_cast<GLenum>(targets));
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		glBindFramebuffer(GL_FRAMEBUFFER, current_id);
		glViewport(t_current_viewport[0], t_current_viewport[1], t_current_viewport[2], t_current_viewport[3]);
	}
}