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

#include "GuiSystem.hpp"

#include <Atema/UI/UiContext.hpp>

#include "../Resources.hpp"
#include "../Settings.hpp"

#include <imgui/imgui_internal.h>

#define IMGUI_LEFT_LABEL(func, label, ...) (ImGui::AlignTextToFramePadding(), ImGui::TextUnformatted(label), ImGui::SameLine(50), func("##" label, __VA_ARGS__))

using namespace at;

namespace
{
	bool show_demo_window = true;
	bool show_another_window = true;
	float clear_color[3] = { 0.0f, 0.0f, 0.0f };

	void checkVkResult(VkResult err)
	{
		if (err == 0)
			return;
		std::cout << "[vulkan] Error: VkResult = " << err << std::endl;
		if (err < 0)
			abort();
	}

	void showDemo()
	{
		// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
		if (show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);

		return;

		ImGui::GetStyle().ScrollbarSize = 10.0f;

		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

			ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
			ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
			ImGui::Checkbox("Another Window", &show_another_window);

			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
			ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

			if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
				counter++;
			ImGui::SameLine();
			ImGui::Text("counter = %d", counter);

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}

		// 3. Show another simple window.
		if (show_another_window)
		{
			ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
			ImGui::Text("Hello from another window!");
			if (ImGui::Button("Close Me"))
				show_another_window = false;
			ImGui::End();
		}
	}
}

GuiSystem::GuiSystem(const at::Ptr<at::RenderWindow>& renderWindow) :
	System(),
	m_timeStep(0.0f)
{
	UiContext::Settings uiSettings;
	uiSettings.renderWindow = renderWindow;
	uiSettings.defaultFont = fontPath / "Roboto-Medium.ttf";
	uiSettings.defaultFontSize = 16.0f;

	UiContext::instance().initialize(uiSettings);

	// Setup Dear ImGui style
	//ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();
}

GuiSystem::~GuiSystem()
{
	UiContext::instance().shutdown();
}

void GuiSystem::update(at::TimeStep timeStep)
{
	m_timeStep = timeStep;

	UiContext::instance().newFrame();

	ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

	updateUI();

	showDemo();

	UiContext::instance().renderFrame();
}

void GuiSystem::onEvent(at::Event& event)
{
	System::onEvent(event);
}

void GuiSystem::updateUI()
{
	auto& settings = Settings::instance();

	ImGui::SetNextItemOpen(false, ImGuiCond_FirstUseEver);

	ImGui::SetNextWindowPos({ 10, 10 }, ImGuiCond_FirstUseEver);

	ImGui::Begin("Settings");

	// Scene
	ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);

	if (ImGui::CollapsingHeader("Scene"))
	{
		ImGui::BeginTable("Properties", 2);

		// Object rows
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);

			ImGui::AlignTextToFramePadding();
			ImGui::Text("Object rows");

			ImGui::TableNextColumn();

			ImGui::SetNextItemWidth(-FLT_MIN);

			static uint32_t step = 1;
			ImGui::InputScalar("##Object rows", ImGuiDataType_U32, &settings.objectRows, &step);
			settings.objectRows = std::clamp(settings.objectRows, 1u, 500u);
		}

		ImGui::EndTable();
	}

	// ShadowMap
	ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);

	if (ImGui::CollapsingHeader("ShadowMap"))
	{
		ImGui::BeginTable("Properties", 2);

		// ShadowMap size
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);

			ImGui::AlignTextToFramePadding();
			ImGui::Text("Size");

			ImGui::TableNextColumn();

			ImGui::SetNextItemWidth(-FLT_MIN);

			static const std::vector<const char*> shadowMapSizeItems =
			{
				"512",
				"1024",
				"2048",
				"4096",
				"8192",
				"16384",
				"32768"
			};
			static int shadowMapCurrentItem = 2;
			ImGui::Combo("##Size", &shadowMapCurrentItem, shadowMapSizeItems.data(), static_cast<int>(shadowMapSizeItems.size()));

			settings.shadowMapSize = std::stoul(shadowMapSizeItems[shadowMapCurrentItem]);
		}

		// ShadowMap box size
		{
			static float step = 1.0f;
			static float fastStep = 10.0f;

			// Min
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);

			ImGui::AlignTextToFramePadding();
			ImGui::Text("Min box size");

			ImGui::TableNextColumn();

			ImGui::SetNextItemWidth(-FLT_MIN);

			ImGui::InputFloat("##Min box size", &settings.shadowBoxMinSize, step, fastStep);
			settings.shadowBoxMinSize = std::clamp(settings.shadowBoxMinSize, 1.0f, settings.shadowBoxMaxSize);

			// Max
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);

			ImGui::AlignTextToFramePadding();
			ImGui::Text("Max box size");

			ImGui::TableNextColumn();

			ImGui::SetNextItemWidth(-FLT_MIN);

			ImGui::InputFloat("##Max box size", &settings.shadowBoxMaxSize, step, fastStep);
			settings.shadowBoxMaxSize = std::clamp(settings.shadowBoxMaxSize, settings.shadowBoxMinSize, 10000.0f);
		}

		ImGui::EndTable();
	}

	// Debug views
	ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);

	if (ImGui::CollapsingHeader("Debug Views"))
	{
		ImGui::BeginTable("Properties", 2);

		static int debugViewMode = 0;

		static int views[] =
		{
			static_cast<int>(Settings::DebugView::ShadowMap),
			static_cast<int>(Settings::DebugView::GBufferNormal),
			static_cast<int>(Settings::DebugView::GBufferColor),
			static_cast<int>(Settings::DebugView::GBufferAO)
		};

		static const std::vector<const char*> viewItems =
		{
			"GBuffer position",
			"GBuffer normal",
			"GBuffer color",
			"GBuffer AO",
			"GBuffer emissive",
			"GBuffer metalness",
			"GBuffer roughness",
			"ShadowMap"
		};

		// Mode
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);

			ImGui::AlignTextToFramePadding();
			ImGui::Text("Mode");

			ImGui::TableNextColumn();

			ImGui::SetNextItemWidth(-FLT_MIN);

			static const std::vector<const char*> items =
			{
				"Disabled",
				"Corner",
				"Full"
			};
			ImGui::Combo("##Mode", &debugViewMode, items.data(), static_cast<int>(items.size()));

			settings.debugViewMode = static_cast<Settings::DebugViewMode>(debugViewMode);
		}

		// Debug views
		{
			int disableRef = 1;

			for (int i = 0; i < 4; i++)
			{
				ImGui::PushID(i);

				if (debugViewMode < disableRef)
					ImGui::BeginDisabled();

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);

				ImGui::AlignTextToFramePadding();
				ImGui::Text(std::string("View #" + std::to_string(i)).c_str());

				ImGui::TableNextColumn();

				ImGui::SetNextItemWidth(-FLT_MIN);

				ImGui::Combo("##View", &views[i], viewItems.data(), static_cast<int>(viewItems.size()));

				settings.debugViews[i] = static_cast<Settings::DebugView>(views[i]);

				if (debugViewMode < disableRef)
					ImGui::EndDisabled();

				ImGui::PopID();

				disableRef = 2;
			}
		}

		ImGui::EndTable();
	}

	ImGui::End();
}

ImGuiID GuiSystem::getWidgetID(const char* label) const
{
	return ImGui::GetCurrentWindow()->GetID(label);
}

GuiSystem::WidgetState& GuiSystem::getWidgetState(const char* label)
{
	const auto id = getWidgetID(label);

	const auto it = m_widgetStates.find(id);

	if (it != m_widgetStates.end())
		return it->second;

	return m_widgetStates[id];
}

ImVec4& GuiSystem::getColor(ImGuiCol_ color)
{
	return ImGui::GetStyle().Colors[color];
}

bool GuiSystem::button(const char* label, const ImVec2& size)
{
	auto& state = getWidgetState(label);

	pushState(state);

	const auto result = ImGui::Button(label, size);

	popState(state);

	state.active = ImGui::IsItemActive();
	state.hovered = ImGui::IsItemHovered();

	return result;
}

void GuiSystem::pushState(WidgetState& state)
{
	static constexpr float perSec = 4.0f;
	const auto delta = perSec * m_timeStep.getSeconds();

	ImVec4 color;

	if (state.active)
	{
		state.value += delta;

		color = getColor(ImGuiCol_ButtonActive);
	}
	else
	{
		if (state.hovered)
			state.value += delta;
		else
			state.value -= delta;

		const auto& c1 = getColor(ImGuiCol_Button);
		const auto& c2 = getColor(ImGuiCol_ButtonHovered);

		color.x = Math::lerp(c1.x, c2.x, state.value);
		color.y = Math::lerp(c1.y, c2.y, state.value);
		color.z = Math::lerp(c1.z, c2.z, state.value);
		color.w = Math::lerp(c1.w, c2.w, state.value);
	}

	state.value = std::clamp(state.value, 0.0f, 1.0f);

	const auto w = ImGui::CalcItemWidth();

	ImGui::SetNextItemWidth(Math::lerp(w, w + 6.0f, state.value));

	ImGui::PushStyleColor(ImGuiCol_Button, color);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color);
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);

	//ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, )
}

void GuiSystem::popState(WidgetState& state)
{
	ImGui::PopStyleColor(3);
}