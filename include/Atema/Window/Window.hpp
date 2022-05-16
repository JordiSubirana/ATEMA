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

#ifndef ATEMA_WINDOW_WINDOW_HPP
#define ATEMA_WINDOW_WINDOW_HPP

#include <Atema/Window/Config.hpp>
#include <Atema/Core/NonCopyable.hpp>
#include <Atema/Core/Pointer.hpp>
#include <Atema/Math/Vector.hpp>
#include <Atema/Core/EventDispatcher.hpp>

#include <string>
#include <vector>

namespace at
{
	class ATEMA_WINDOW_API Window : NonCopyable
	{
	public:
		struct Settings
		{
			Settings()
			{
				width = 640;
				height = 480;

				resizable = true;
				
				title = "Atema";
			}

			unsigned int width;
			unsigned int height;

			bool resizable;

			std::string title;
		};

		Window() = delete;
		virtual ~Window();

		static Ptr<Window> create(const Settings& settings);

		void setTitle(const std::string& title);

		void setCursorEnabled(bool enable); // Default : true
		bool isCursorEnabled() const noexcept;
		
		bool shouldClose() const noexcept;

		void processEvents();

		void swapBuffers();

		Vector2u getSize() const noexcept;

		EventDispatcher& getEventDispatcher() noexcept;
		const EventDispatcher& getEventDispatcher() const noexcept;

		// Platform specific
		void* getHandle() const;
		
		// Vulkan specific
		static const std::vector<const char*>& getVulkanExtensions();

	protected:
		Window(const Settings& settings);

	private:
		void resizedCallback(unsigned int width, unsigned int height);
		
		class Implementation;

		UPtr<Implementation> m_implementation;

		Vector2u m_size;

		EventDispatcher m_eventDispatcher;
	};
}

#endif
