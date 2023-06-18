/*
	Copyright 2023 Jordi SUBIRANA

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

#ifndef ATEMA_RENDERER_UICONTEXT_HPP
#define ATEMA_RENDERER_UICONTEXT_HPP

#include <Atema/Renderer/Config.hpp>
#include <Atema/Renderer/UI/ImGui.hpp>
#include <Atema/Core/Pointer.hpp>

#include <filesystem>

namespace at
{
	class CommandBuffer;
	class RenderWindow;

	class ATEMA_RENDERER_API UiContext
	{
	public:
		struct Settings
		{
			RenderWindow* renderWindow = nullptr;
			std::filesystem::path defaultFont;
			float defaultFontSize = 13.0f;
		};

		UiContext() = delete;
		UiContext(const UiContext& other) = delete;
		UiContext(UiContext&& other) noexcept = delete;
		virtual ~UiContext();

		static Ptr<UiContext> create(const Settings& settings);

		static UiContext* getActive();
		void setActive();
		
		void shutdown();

		ImGuiContext* getImGuiContext() const;

		void newFrame();
		virtual void draw(ImDrawData* drawData, CommandBuffer& commandBuffer) = 0;
		void renderFrame();

		UiContext& operator=(const UiContext& other) = delete;
		UiContext& operator=(UiContext&& other) noexcept = delete;

	protected:
		UiContext(const Settings& settings);

		virtual void shutdownPlatform() = 0;
		virtual void newPlatformFrame() = 0;

	private:
		ImGuiContext* m_context;
	};
}

#endif
