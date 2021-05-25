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

#include <Atema/Core/ScopedTimer.hpp>

#include <iostream>

using namespace at;

namespace
{
	void defaultCallback(ScopedTimer::TimeCount time)
	{
		std::cout << "ScopedTimer : " << static_cast<float>(time) * 0.001f << " ms\n";
	}
}

ScopedTimer::ScopedTimer() :
	m_callback(defaultCallback)
{
	m_start = std::chrono::high_resolution_clock::now();
}

ScopedTimer::ScopedTimer(const std::function<void(TimeCount)>& callback) :
	ScopedTimer()
{
	m_callback = callback;
}

ScopedTimer::~ScopedTimer()
{
	if (m_callback)
	{
		const auto end = std::chrono::high_resolution_clock::now();

		const auto delta = std::chrono::duration_cast<std::chrono::microseconds>(end - m_start).count();

		m_callback(delta);
	}
}
