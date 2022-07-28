/*
	Copyright 2022 Jordi SUBIRANA

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

#ifndef ATEMA_UI_UICONTEXT_HPP
#define ATEMA_UI_UICONTEXT_HPP

#include <Atema/UI/Config.hpp>
#include <Atema/UI/ImGui.hpp>

#include <filesystem>

namespace at
{
	namespace detail
	{
		class UiContextImplementation
		{
		public:
			UiContextImplementation();
			virtual ~UiContextImplementation();

			virtual void newFrame() = 0;
			virtual void renderDrawData(ImDrawData* drawData, CommandBuffer& commandBuffer) = 0;
		};
	}

	class RenderWindow;

	class ATEMA_UI_API UiContext
	{
	public:
		struct Settings
		{
			std::filesystem::path defaultFont;
			float defaultFontSize = 13.0f;
			Ptr<RenderWindow> renderWindow;
		};

		~UiContext();

		static UiContext& instance();

		void initialize(const Settings& settings);
		void shutdown();

		ImGuiContext* getImGuiContext() const;

		void newFrame();
		void renderFrame();
		void renderDrawData(ImDrawData* drawData, CommandBuffer& commandBuffer);

	private:
		UiContext();

		Ptr<detail::UiContextImplementation> m_implementation;
		ImGuiContext* m_context;
	};
}

#endif
