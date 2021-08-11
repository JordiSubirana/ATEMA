/*
	Copyright 2021 Jordi SUBIRANA

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

#ifndef ATEMA_RENDERER_COMMANDBUFFER_HPP
#define ATEMA_RENDERER_COMMANDBUFFER_HPP

#include <Atema/Renderer/Config.hpp>
#include <Atema/Core/NonCopyable.hpp>
#include <Atema/Core/Pointer.hpp>
#include <Atema/Core/Vector.hpp>

#include <vector>

namespace at
{
	class CommandPool;
	class RenderPass;
	class Framebuffer;
	class GraphicsPipeline;

	class ATEMA_RENDERER_API CommandBuffer : public NonCopyable
	{
	public:
		struct Settings
		{
			Ptr<CommandPool> commandPool;
		};
		
		typedef union ClearValue
		{
			float color[4];
			
			struct
			{
				float depth;
				uint32_t stencil;
			} depthStencil;
			
		} ClearValue;
		
		virtual ~CommandBuffer();

		static Ptr<CommandBuffer> create(const Settings& settings);

		virtual void begin() = 0;

		virtual void beginRenderPass(const Ptr<RenderPass>& renderPass, const Ptr<Framebuffer>& framebuffer, const std::vector<ClearValue>& clearValues) = 0;

		virtual void bindPipeline(const Ptr<GraphicsPipeline>& pipeline) = 0;

		virtual void endRenderPass() = 0;

		virtual void end() = 0;
		
	protected:
		CommandBuffer();
	};
}

#endif
