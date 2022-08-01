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

#ifndef ATEMA_SANDBOX_SETTINGS_HPP
#define ATEMA_SANDBOX_SETTINGS_HPP

#include <cstdint>
#include <vector>

class Settings
{
public:
	enum class DebugViewMode
	{
		Disabled,
		Corner,
		Full
	};

	enum class DebugView
	{
		GBufferPosition,
		GBufferNormal,
		GBufferColor,
		GBufferAO,
		GBufferEmissive,
		GBufferMetalness,
		GBufferRoughness,
		ShadowMap,
	};

	Settings();
	~Settings();

	static Settings& instance();

	// Application
	bool enableFpsLimit;
	uint32_t fpsLimit;

	//Scene
	uint32_t objectRows;

	// ShadowMap
	uint32_t shadowMapSize;
	float shadowBoxMinSize;
	float shadowBoxMaxSize;

	// Debug views
	DebugViewMode debugViewMode;
	std::vector<DebugView> debugViews;

	// Metrics
	float metricsUpdateTime;
	bool enableBenchmarks;

private:
};

#endif
