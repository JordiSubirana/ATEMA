/*
	Copyright 2023 Jordi SUBIRANA

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

#include <Atema/Renderer/Renderer.hpp>
#include <Atema/Renderer/UI/UiContext.hpp>

#define GLFW_INCLUDE_NONE
#include <imgui/imgui_impl_glfw.h>

// Dear ImGui implementation
#include <imgui/imgui.cpp>
#include <imgui/imgui_demo.cpp>
#include <imgui/imgui_draw.cpp>
#include <imgui/imgui_tables.cpp>
#include <imgui/imgui_widgets.cpp>
#include <imgui/imgui_impl_glfw.cpp>

using namespace at;

namespace
{
	UiContext* s_activeContext = nullptr;
}

UiContext::UiContext(const Settings& settings) :
	m_context(nullptr)
{
	ATEMA_ASSERT(settings.renderWindow, "Invalid RenderWindow");

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	m_context = ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
	//io.ConfigViewportsNoAutoMerge = true;
	//io.ConfigViewportsNoTaskBarIcon = true;
	if (settings.defaultFont.empty())
		io.Fonts->AddFontDefault();
	else
		io.Fonts->AddFontFromFileTTF(settings.defaultFont.string().c_str(), settings.defaultFontSize);

	ImGui_ImplGlfw_InitForVulkan(static_cast<GLFWwindow*>(settings.renderWindow->getGLFWHandle()), true);
}

UiContext::~UiContext()
{
	shutdown();

	if (s_activeContext == this)
		s_activeContext = nullptr;
}

Ptr<UiContext> UiContext::create(const Settings& settings)
{
	return Renderer::instance().createUiContext(settings);
}

UiContext* UiContext::getActive()
{
	return s_activeContext;
}

void UiContext::setActive()
{
	s_activeContext = this;
}

void UiContext::shutdown()
{
	if (m_context)
	{
		Renderer::instance().waitForIdle();

		shutdownPlatform();

		ImGui_ImplGlfw_Shutdown();

		ImGui::DestroyContext(m_context);

		m_context = nullptr;
	}
}

ImGuiContext* UiContext::getImGuiContext() const
{
	return m_context;
}

void UiContext::newFrame()
{
	newPlatformFrame();

	ImGui_ImplGlfw_NewFrame();

	ImGui::NewFrame();
}

void UiContext::renderFrame()
{
	ImGui::Render();

	// Update and Render additional Platform Windows
	ImGui::UpdatePlatformWindows();
	ImGui::RenderPlatformWindowsDefault();
}
