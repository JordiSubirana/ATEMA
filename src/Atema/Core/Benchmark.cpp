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

#include <Atema/Core/Benchmark.hpp>

#include <iostream>

using namespace at;

// BenchmarkData
BenchmarkData::BenchmarkData() :
	parent(nullptr),
	indent(0),
	label(""),
	timeStep(0.0f)
{
}

// Benchmark
Benchmark::Benchmark(const std::string& label) :
	ScopedTimer()
{
	m_data = BenchmarkManager::getInstance().getData(label);

	auto data = m_data;
	
	auto callback = [data](TimeStep timeStep)
	{
		data->timeStep += timeStep;
	};

	setCallback(callback);
}

Benchmark::~Benchmark()
{
	BenchmarkManager::getInstance().decrement();
}

// BenchmarkManager
BenchmarkManager::BenchmarkManager() :
	m_currentData(nullptr),
	m_currentIndent(0)
{
	reset();
}

BenchmarkManager::~BenchmarkManager()
{
	reset();
}

BenchmarkManager& BenchmarkManager::getInstance() noexcept
{
	thread_local BenchmarkManager s_instance;

	return s_instance;
}

void BenchmarkManager::reset()
{
	m_currentData = nullptr;
	m_currentIndent = 0;
	m_data.clear();
	m_roots.clear();
	m_tmp.clear();
}

TimeStep BenchmarkManager::getTimeStep(const std::string& label)
{
	return getData(label)->timeStep;
}

void BenchmarkManager::print(uint32_t factor)
{
	const float clampFactor = factor ? static_cast<float>(factor) : 1.0f;

	for (auto& data : m_roots)
		print(data, clampFactor, data->timeStep.getMilliSeconds());
}

void BenchmarkManager::print(BenchmarkData* data, float factor, float rootTime)
{
	if (data->indent > 0)
		std::cout << std::string(data->indent * 4, ' ');

	const auto normalizedTime = data->timeStep.getMilliSeconds() / factor;
	const auto percent = static_cast<unsigned>((data->timeStep.getMilliSeconds() * 100.0f) / rootTime);
	
	//std::cout << data->label << " : " << normalizedTime << "ms [" << percent << "%]\n";
	std::cout << "[" << percent << "%] " << data->label << " : " << normalizedTime << "ms\n";

	for (auto& child : data->children)
		print(child, factor, rootTime);
}

Ptr<BenchmarkData> BenchmarkManager::getData(const std::string& label)
{
	Ptr<BenchmarkData> data;

	// Find if the entry exists
	const auto it = m_data.find(label);

	if (it != m_data.end())
	{
		data = it->second;
	}
	// Otherwise create it and increment indent/data
	else
	{
		data = std::make_shared<BenchmarkData>();
		data->parent = m_currentData;
		data->indent = m_currentIndent;
		data->label = label;

		m_data[label] = data;

		if (m_currentIndent == 0)
			m_roots.push_back(data.get());
		else
			data->parent->children.push_back(data.get());
	}

	// Increment indent/data
	m_currentData = data.get();
	m_currentIndent++;

	return data;
}

void BenchmarkManager::decrement()
{
	if (m_currentData)
	{
		m_currentData = m_currentData->parent;
		m_currentIndent--;
	}
}
