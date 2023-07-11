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

#define SHADOW_CASCADE_COUNT 8

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
		ShadowCascade1,
		ShadowCascade2,
		ShadowCascade3,
		ShadowCascade4,
		ShadowCascade5,
		ShadowCascade6,
		ShadowCascade7,
		ShadowCascade8,
	};

	Settings();
	~Settings();

	static Settings& instance();

	// Application
	bool enableFpsLimit;
	uint32_t fpsLimit;

	// Camera
	float cameraSpeed;

	//Scene
	uint32_t objectRows;

	// Culling
	bool customFrustumCulling;

	// ShadowMap
	uint32_t shadowMapSize;
	float baseDepthBias;
	uint32_t shadowCascadeCount;

	// Debug views
	bool enableDebugRenderer;
	bool enableDebugGBuffer;
	bool enableDebugShadowMaps;

	// Metrics
	float metricsUpdateTime;
	bool enableBenchmarks;

private:
};

#endif
