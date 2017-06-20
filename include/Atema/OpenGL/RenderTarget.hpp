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

#ifndef ATEMA_OPENGL_RENDERTARGET_HPP
#define ATEMA_OPENGL_RENDERTARGET_HPP

#include <Atema/OpenGL/Config.hpp>
#include <Atema/Renderer/RenderTarget.hpp>
#include <Atema/OpenGL/Context.hpp>
#include <Atema/OpenGL/FrameBuffer.hpp>
#include <vector>
#include <map>

namespace at
{
	class ATEMA_OPENGL_API OpenGLRenderTarget : public RenderTarget::Implementation, public OpenGLFrameBuffer
	{
		friend class OpenGLWindow;

	public:
		OpenGLRenderTarget();
		virtual ~OpenGLRenderTarget();

		int addAttachment(const Texture& texture, int index = -1) override;
		int addAttachment(unsigned width, unsigned height, int index = -1) override;

		void removeAttachment(int index) override;
		void removeAttachments() override;

		void setDrawArea(int x, int y, unsigned w, unsigned h) override;

		void clearColor(const Color& color, int index) override;
		void clearColor(const Color& color) override;
		void clearDepth() override;

		bool isValid(int index) const override;

		static std::shared_ptr<RenderTarget> fromFrameBuffer(RenderSystem *system, unsigned glId);

	private:
		int getFirstAvailable() const;

		std::map<int, unsigned> m_rbos;
		bool m_ownsFbo;
	};
}

#endif