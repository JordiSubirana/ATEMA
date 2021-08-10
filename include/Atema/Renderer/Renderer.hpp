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

#ifndef ATEMA_RENDERER_RENDERER_HPP
#define ATEMA_RENDERER_RENDERER_HPP

#include <Atema/Renderer/Config.hpp>
#include <Atema/Core/Pointer.hpp>
#include <Atema/Core/Window.hpp>
#include <Atema/Renderer/CommandPool.hpp>
#include <Atema/Renderer/Image.hpp>
#include <Atema/Renderer/SwapChain.hpp>
#include <Atema/Renderer/RenderPass.hpp>
#include <Atema/Renderer/Framebuffer.hpp>
#include <Atema/Renderer/GraphicsPipeline.hpp>
#include <Atema/Renderer/Shader.hpp>

namespace at
{
	class ATEMA_RENDERER_API Renderer
	{
	public:
		struct Settings
		{
			Window::Settings mainWindowSettings;

			size_t maxFramesInFlight = 2;

			bool sampleShading = true;
		};
		
		Renderer() = delete;
		virtual ~Renderer();

		template <typename T>
		static Renderer& create(const Settings& settings);

		static Renderer& getInstance();

		static void destroy();

		virtual void initialize() = 0;

		const Settings& getSettings() const noexcept;
		
		Ptr<Window> getMainWindow() const noexcept;

		virtual void registerWindow(Ptr<Window> window) = 0;
		virtual void unregisterWindow(Ptr<Window> window) = 0;

		// Object creation
		virtual Ptr<Image> createImage(const Image::Settings& settings) = 0;
		virtual Ptr<SwapChain> createSwapChain(const SwapChain::Settings& settings) = 0;
		virtual Ptr<RenderPass> createRenderPass(const RenderPass::Settings& settings) = 0;
		virtual Ptr<Framebuffer> createFramebuffer(const Framebuffer::Settings& settings) = 0;
		virtual Ptr<Shader> createShader(const Shader::Settings& settings) = 0;
		virtual Ptr<DescriptorSetLayout> createDescriptorSetLayout(const DescriptorSetLayout::Settings& settings) = 0;
		virtual Ptr<GraphicsPipeline> createGraphicsPipeline(const GraphicsPipeline::Settings& settings) = 0;
		virtual Ptr<CommandPool> createCommandPool(const CommandPool::Settings& settings) = 0;

	protected:
		Renderer(const Settings& settings);

	private:
		static Ptr<Renderer> s_renderer;
		Settings m_settings;
		Ptr<Window> m_mainWindow;
	};

	template <typename T>
	Renderer& Renderer::create(const Settings& settings)
	{
		static_assert(std::is_base_of<Renderer, T>::value, "Invalid Renderer type");

		if (s_renderer)
		{
			ATEMA_ERROR("A Renderer already exists");

			return *s_renderer;
		}

		auto renderer = std::make_shared<T>(settings);
		
		s_renderer = std::static_pointer_cast<Renderer>(renderer);

		s_renderer->initialize();

		return *s_renderer;
	}
}

#endif
