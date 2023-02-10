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

#ifndef ATEMA_SANDBOX_SANDBOXAPPLICATION_HPP
#define ATEMA_SANDBOX_SANDBOXAPPLICATION_HPP

#include <Atema/Atema.hpp>

class System;
struct MaterialData;
struct ModelData;

class SandboxApplication : public at::Application
{
public:
	SandboxApplication();
	~SandboxApplication();

	void onEvent(at::Event& event) override;

	void update(at::TimeStep ms) override;

private:
	void checkSettings();

	void createScene();
	void createCamera();
	void createPlayer();

	void updateScene();
	
	at::Ptr<at::RenderWindow> m_window;

	at::EntityManager m_entityManager;

	std::vector<at::Ptr<System>> m_systems;

	at::Ptr<ModelData> m_modelData;
	std::vector<at::Ptr<MaterialData>> m_materialData;

	std::vector<at::EntityHandle> m_objects;

	int m_frameCount;
	float m_frameDuration;

	// Settings
	uint32_t m_objectRows;
};

#endif
