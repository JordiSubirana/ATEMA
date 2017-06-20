/*
	Copyright 2017 Jordi SUBIRANA

	Permission is hereby granted, free of charge, to any person obtaining a copy of
	this software and associated documentation files (the "Software"), to deal in
	the Software without restriction, including without limitation the rights to use,
	copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
	Software, and to permit persons to whom the Software is furnished to do so, subject
	to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
	INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
	PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
	HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
	CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
	OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef ATEMA_OPENGL_FRAMEBUFFER_HPP
#define ATEMA_OPENGL_FRAMEBUFFER_HPP

#include <Atema/OpenGL/Config.hpp>
#include <Atema/Renderer/Color.hpp>
#include <Atema/OpenGL/Context.hpp>
#include <vector>

namespace at
{
	class ATEMA_OPENGL_API OpenGLFrameBuffer
	{
	public:
		virtual ~OpenGLFrameBuffer();

		unsigned getId() const;
		OpenGLContext* getContext();

		void bind() const;

		void setDrawArea(int x, int y, unsigned w, unsigned h);

		void clearColor(const Color& color, int index);
		void clearColor(const Color& color);
		void clearDepth();

	protected:
		OpenGLFrameBuffer();
		void initialize(unsigned id);
		bool isValid(int index) const;

		std::vector<unsigned> m_drawBuffers;

	private:
		unsigned m_id;
		OpenGLContext *m_context;
		struct { int x, y, w, h; } m_viewport;
	};
}

#endif