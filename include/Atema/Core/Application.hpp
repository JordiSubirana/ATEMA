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

#ifndef ATEMA_CORE_APPLICATION_HPP
#define ATEMA_CORE_APPLICATION_HPP

#include <Atema/Core/Config.hpp>
#include <Atema/Core/NonCopyable.hpp>
#include <Atema/Core/Event.hpp>

#include <queue>

namespace at
{
	class TimeStep;
	
	class ATEMA_CORE_API Application : public NonCopyable
	{
	public:
		virtual ~Application();

		static Application& instance();

		// Runs the main loop
		void run();

		// Request the application to close after the current loop execution
		void close();

		// Push an event that will be executed on the next loop
		void pushEvent(Event& event);

		// 0 : no limit
		void setFpsLimit(uint32_t fpsLimit);

	protected:
		Application();
		
		virtual void onEvent(Event& event) = 0;
		virtual void update(TimeStep ms) = 0;
		
	private:
		void processEvents();

		bool m_close;
		std::queue<Event> m_currentEvents;
		std::queue<Event> m_nextEvents;

		uint32_t m_fpsLimit;
	};
}

#endif
