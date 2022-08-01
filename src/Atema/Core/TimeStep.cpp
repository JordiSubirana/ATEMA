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

#include <Atema/Core/TimeStep.hpp>

using namespace at;
using namespace std::chrono_literals;

TimeStep::TimeStep() :
	m_time(0us)
{
}

TimeStep::TimeStep(const std::chrono::microseconds& us) :
	m_time(us)
{
}

float TimeStep::getSeconds() const noexcept
{
	return static_cast<float>(m_time.count()) * 0.000001f;
}

float TimeStep::getMilliSeconds() const noexcept
{
	return static_cast<float>(m_time.count()) * 0.001f;
}

float TimeStep::getMicroSeconds() const noexcept
{
	return static_cast<float>(m_time.count());
}

std::chrono::seconds TimeStep::getStdSeconds() const noexcept
{
	return std::chrono::duration_cast<std::chrono::seconds>(m_time);
}

std::chrono::milliseconds TimeStep::getStdMilliSeconds() const noexcept
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(m_time);
}

std::chrono::microseconds TimeStep::getStdMicroSeconds() const noexcept
{
	return m_time;
}

TimeStep TimeStep::operator+(const TimeStep& other) const noexcept
{
	return TimeStep(m_time + other.m_time);
}

TimeStep& TimeStep::operator+=(const TimeStep& other) noexcept
{
	m_time += other.m_time;

	return *this;
}
