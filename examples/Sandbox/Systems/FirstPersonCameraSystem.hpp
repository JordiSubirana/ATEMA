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

#ifndef ATEMA_SANDBOX_FIRSTPERSONCAMERASYSTEM_HPP
#define ATEMA_SANDBOX_FIRSTPERSONCAMERASYSTEM_HPP

#include "System.hpp"

class FirstPersonCameraSystem : public System
{
public:
	FirstPersonCameraSystem() = delete;
	FirstPersonCameraSystem(const at::Ptr<at::RenderWindow>& renderWindow);
	virtual ~FirstPersonCameraSystem();

	void update(at::TimeStep timeStep) override;
	void onEvent(at::Event& event) override;

private:
	void rotate(at::Transform& transform, const at::Vector2f& mousePosition);

	at::RenderWindow* m_window;
	bool m_front, m_back, m_right, m_left, m_up, m_down;
	bool m_cameraRotationEnabled;
	bool m_initMousePosition;
	at::Vector2f m_lastPosition;
};

#endif
