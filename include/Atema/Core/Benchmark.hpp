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

#ifndef ATEMA_CORE_BENCHMARK_HPP
#define ATEMA_CORE_BENCHMARK_HPP

#include <Atema/Core/Config.hpp>
#include <Atema/Core/ScopedTimer.hpp>
#include <Atema/Core/Pointer.hpp>
#include <Atema/Core/NonCopyable.hpp>

#include <string>

#define ATEMA_BENCHMARK_ENABLE 1

#if ATEMA_BENCHMARK_ENABLE
#define ATEMA_BENCHMARK(at_label) ATEMA_BENCHMARK_TAG(__atBenchmark, at_label);
#define ATEMA_BENCHMARK_TAG(at_tag, at_label) at::Benchmark at_tag(at_label);
#else
#define ATEMA_BENCHMARK(at_label)
#define ATEMA_BENCHMARK_TAG(at_tag, at_label)
#endif

namespace at
{
	struct ATEMA_CORE_API BenchmarkData
	{
		BenchmarkData();

		BenchmarkData* parent;
		uint32_t indent;
		std::string label;
		TimeStep timeStep;
		std::vector<BenchmarkData*> children;
	};

	class ATEMA_CORE_API Benchmark final : public ScopedTimer
	{
	public:
		Benchmark() = delete;
		Benchmark(const std::string& label);
		~Benchmark();
		
	private:
		Ptr<BenchmarkData> m_data;
	};

	class ATEMA_CORE_API BenchmarkManager final : public NonCopyable
	{
		friend class Benchmark;
		
	public:
		virtual ~BenchmarkManager();

		static BenchmarkManager& instance() noexcept;
		
		void reset();
		
		TimeStep getTimeStep(const std::string& label);

		void print(uint32_t factor = 1);
		
	private:
		BenchmarkManager();

		void print(BenchmarkData* data, float factor, float rootTime);
		
		Ptr<BenchmarkData> getData(const std::string& label);

		void decrement();

		std::unordered_map<std::string, Ptr<BenchmarkData>> m_data;
		std::unordered_map<std::string, WPtr<Benchmark>> m_tmp;

		std::vector<BenchmarkData*> m_roots;
		BenchmarkData* m_currentData;
		uint32_t m_currentIndent;
	};
}

#endif
