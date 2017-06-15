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

#ifndef ATEMA_RENDERER_RENDERTARGET_HPP
#define ATEMA_RENDERER_RENDERTARGET_HPP

#include <Atema/Renderer/Config.hpp>
#include <Atema/Renderer/RendererDependent.hpp>
#include <Atema/Renderer/Texture.hpp>

namespace at
{
	class ATEMA_RENDERER_API AbstractRenderTarget
	{
	public:
		class ATEMA_RENDERER_API Implementation : public RendererDependent
		{
		public:
			Implementation();
			virtual ~Implementation() = default;
		};

		AbstractRenderTarget();
		virtual ~AbstractRenderTarget();

		Implementation* getImplementation();
		const Implementation* getImplementation() const;

		virtual void setDrawArea(int x, int y, unsigned w, unsigned h) = 0;

		virtual void clearColor(const Color& color) = 0;
		virtual void clearDepth() = 0;

	protected:
		Implementation *m_abstractImpl;
	};
	
	class ATEMA_RENDERER_API RenderTarget : public AbstractRenderTarget
	{
	public:
		class ATEMA_RENDERER_API Implementation : public AbstractRenderTarget::Implementation
		{
		public:
			Implementation();
			virtual ~Implementation() = default;

			virtual int addAttachment(const Texture& texture, int index = -1) = 0;
			virtual int addAttachment(unsigned width, unsigned height, int index = -1) = 0;

			virtual void removeAttachment(int index) = 0;
			virtual void removeAttachments() = 0;

			virtual void setDrawArea(int x, int y, unsigned w, unsigned h) = 0;

			virtual void clearColor(const Color& color, int index) = 0;
			virtual void clearColor(const Color& color) = 0;
			virtual void clearDepth() = 0;

			virtual bool isValid(int index) const = 0;
		};

		RenderTarget(RenderSystem *renderer = nullptr);
		virtual ~RenderTarget();

		Implementation* getImplementation();
		const Implementation* getImplementation() const;

		int addAttachment(const Texture& texture, int index = -1);
		int addAttachment(unsigned width, unsigned height, int index = -1);

		void removeAttachment(int index);
		void removeAttachments();

		bool isValid(int index) const;

		void setDrawArea(int x, int y, unsigned w, unsigned h) override;

		void clearColor(const Color& color, int index);
		void clearColor(const Color& color) override;
		void clearDepth() override;

	private:
		Implementation *m_impl;
	};
}

#endif