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

#include "Settings.hpp"

Settings::Settings() :
	enableFpsLimit(false),
	fpsLimit(60),
	cameraSpeed(20.0f),
	sceneType(SceneType::Test),
	objectRows(1),
	moveObjects(true),
	shadowMapSize(2048),
	baseDepthBias(0.07f),
	shadowCascadeCount(8),
	enableToneMapping(true),
	toneMappingExposure(1.0f),
	toneMappingGamma(2.2f),
	enableDebugRenderer(false),
	enableDebugGBuffer(false),
	enableDebugShadowMaps(false),
	metricsUpdateTime(1.0f),
	enableBenchmarks(true)
{
}

Settings::~Settings()
{
}

Settings& Settings::instance()
{
	static Settings s_settings;

	return s_settings;
}
