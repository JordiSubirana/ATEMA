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

#include <Atema/Renderer/UI/UiContext.hpp>

#include "../Resources.hpp"
#include "../Settings.hpp"
#include "../Stats.hpp"

#include <imgui/imgui_internal.h>

#include <sstream>
#include <locale>

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

	ImVec4 getBenchmarkColor(unsigned percent)
	{
		if (percent >= 30)
			return ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
		else if (percent >= 20)
			return ImVec4(1.0f, 0.5f, 0.0f, 1.0f);
		else if (percent >= 10)
			return ImVec4(1.0f, 0.8f, 0.0f, 1.0f);

		return ImGui::GetStyleColorVec4(ImGuiCol_Text);
	}
}

GuiSystem::GuiSystem(const at::Ptr<at::RenderWindow>& renderWindow) :
	System(),
	m_elapsedFrames(0),
	m_elapsedTime(0.0f)
{
	UiContext::Settings uiSettings;
	uiSettings.renderWindow = renderWindow.get();
	uiSettings.defaultFont = ResourcePath::fonts() / "Roboto-Medium.ttf";
	uiSettings.defaultFontSize = 16.0f;

	m_uiContext = UiContext::create(uiSettings);
	m_uiContext->setActive();

	// Setup Dear ImGui style
	//ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();
}

GuiSystem::~GuiSystem()
{
	m_uiContext->shutdown();
}

void GuiSystem::update(at::TimeStep timeStep)
{
	m_timeStep = timeStep;

	m_uiContext->newFrame();

	ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

	updateUI();

	m_uiContext->renderFrame();
}

void GuiSystem::onEvent(at::Event& event)
{
	System::onEvent(event);
}

void GuiSystem::updateBenchmarks(int elapsedFrames)
{
	m_benchmarks = BenchmarkManager::instance().getRootBenchmarks();

	m_elapsedFrames = elapsedFrames;

	m_elapsedTime = 0.0f;
	for (const auto& benchmark : m_benchmarks)
		m_elapsedTime += benchmark->timeStep.getSeconds();
}

void GuiSystem::updateStats(int elapsedFrames)
{
	auto& stats = Stats::instance();
	m_stats = stats;

	for (auto& stat : m_stats)
		stat.second /= elapsedFrames;

	stats.clear();
}

void GuiSystem::updateUI()
{
	showSettings();

	showOverlay();

	//showDemo();
}

void GuiSystem::showSettings()
{
	auto& settings = Settings::instance();

	const ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDocking |
		//ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoSavedSettings |
		//ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove;

	const float PAD = 10.0f;
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
	ImVec2 window_pos, window_pos_pivot;
	window_pos.x = work_pos.x + PAD;
	window_pos.y = work_pos.y + PAD;
	window_pos_pivot.x = 0.0f;
	window_pos_pivot.y = 0.0f;

	ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
	ImGui::SetNextWindowViewport(viewport->ID);

	ImGui::SetNextItemOpen(false, ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Settings", nullptr, windowFlags))
	{
		// Scene
		ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);

		if (ImGui::CollapsingHeader("Scene"))
		{
			ImGui::BeginTable("Properties", 2);

			// Scene selection
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);

				ImGui::AlignTextToFramePadding();
				ImGui::Text("Scene");

				ImGui::TableNextColumn();

				ImGui::SetNextItemWidth(-FLT_MIN);

				static const std::vector<const char*> shadowMapSizeItems =
				{
					"None",
					"Default",
					"Test Scene",
					"PBR Spheres"
				};
				static int shadowMapCurrentItem = static_cast<int>(settings.sceneType);
				ImGui::Combo("##Scene", &shadowMapCurrentItem, shadowMapSizeItems.data(), static_cast<int>(shadowMapSizeItems.size()));

				settings.sceneType = static_cast<Settings::SceneType>(shadowMapCurrentItem);
			}

			// Object rows
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);

				ImGui::AlignTextToFramePadding();
				ImGui::Text("Object rows");

				ImGui::TableNextColumn();

				ImGui::SetNextItemWidth(-FLT_MIN);

				static uint32_t step = 1;
				static uint32_t stepFast = 2;
				ImGui::InputScalar("##Object rows", ImGuiDataType_U32, &settings.objectRows, &step, &stepFast);
				settings.objectRows = std::clamp(settings.objectRows, 1u, 500u);
			}

			// Move objects
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);

				ImGui::AlignTextToFramePadding();
				ImGui::Text("Move objects");

				ImGui::TableNextColumn();

				ImGui::SetNextItemWidth(-FLT_MIN);

				ImGui::Checkbox("##Move objects", &settings.moveObjects);
			}

			ImGui::EndTable();
		}

		// Application
		ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);

		if (ImGui::CollapsingHeader("Application"))
		{
			ImGui::BeginTable("Properties", 2);

			// Enable FPS limit
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);

				ImGui::AlignTextToFramePadding();
				ImGui::Text("Enable FPS limit");

				ImGui::TableNextColumn();

				ImGui::SetNextItemWidth(-FLT_MIN);

				ImGui::Checkbox("##Enable FPS limit", &settings.enableFpsLimit);
			}

			// FPS limit
			{
				ImGui::BeginDisabled(!settings.enableFpsLimit);

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);

				ImGui::AlignTextToFramePadding();
				ImGui::Text("FPS limit");

				ImGui::TableNextColumn();

				ImGui::SetNextItemWidth(-FLT_MIN);

				static uint32_t step = 1;
				ImGui::InputScalar("##FPS limit", ImGuiDataType_U32, &settings.fpsLimit, &step);
				settings.fpsLimit = std::clamp(settings.fpsLimit, 1u, 100000u);

				ImGui::EndDisabled();
			}

			ImGui::EndTable();
		}

		// Camera
		ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);

		if (ImGui::CollapsingHeader("Camera"))
		{
			ImGui::BeginTable("Properties", 2);

			// Speed
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);

				ImGui::AlignTextToFramePadding();
				ImGui::Text("Speed (m/s)");

				ImGui::TableNextColumn();

				ImGui::SetNextItemWidth(-FLT_MIN);

				ImGui::InputFloat("##Speed (m/s)", &settings.cameraSpeed, 1.0f, 10.0f, "%.3f");
				settings.cameraSpeed = std::clamp(settings.cameraSpeed, 0.0f, 1000.0f);
			}

			ImGui::EndTable();
		}

		// ShadowMap
		ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);

		if (ImGui::CollapsingHeader("ShadowMap"))
		{
			ImGui::BeginTable("Properties", 2);

			// ShadowMap cascades
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);

				ImGui::AlignTextToFramePadding();
				ImGui::Text("Cascades");

				ImGui::TableNextColumn();

				ImGui::SetNextItemWidth(-FLT_MIN);

				static const std::vector<const char*> shadowMapCascadeItems =
				{
					"1",
					"2",
					"4",
					"8",
					"16"
				};
				static int shadowMapCascadeCurrentItem = 3;
				ImGui::Combo("##Cascades", &shadowMapCascadeCurrentItem, shadowMapCascadeItems.data(), static_cast<int>(shadowMapCascadeItems.size()));

				settings.shadowCascadeCount = std::stoul(shadowMapCascadeItems[shadowMapCascadeCurrentItem]);
			}

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
					"1024",
					"2048",
					"4096",
					"8192"
				};
				static int shadowMapCurrentItem = 2;
				ImGui::Combo("##Size", &shadowMapCurrentItem, shadowMapSizeItems.data(), static_cast<int>(shadowMapSizeItems.size()));

				settings.shadowMapSize = std::stoul(shadowMapSizeItems[shadowMapCurrentItem]);
			}

			// ShadowMap base depth bias
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);

				ImGui::AlignTextToFramePadding();
				ImGui::Text("Base depth bias");

				ImGui::TableNextColumn();

				ImGui::SetNextItemWidth(-FLT_MIN);

				ImGui::InputFloat("##Base depth bias", &settings.baseDepthBias, 0.01f, 0.1f, "%.3f");
				settings.baseDepthBias = std::clamp(settings.baseDepthBias, 0.0f, 1000.0f);
			}

			ImGui::EndTable();
		}

		// Tone Mapping
		ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);

		if (ImGui::CollapsingHeader("Tone Mapping"))
		{
			ImGui::BeginTable("Properties", 2);

			// Enable Tone Mapping
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);

				ImGui::AlignTextToFramePadding();
				ImGui::Text("Enable");

				ImGui::TableNextColumn();

				ImGui::SetNextItemWidth(-FLT_MIN);

				ImGui::Checkbox("##Enable", &settings.enableToneMapping);
			}

			// Exposure
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);

				ImGui::AlignTextToFramePadding();
				ImGui::Text("Exposure");

				ImGui::TableNextColumn();

				ImGui::SetNextItemWidth(-FLT_MIN);

				ImGui::InputFloat("##Exposure", &settings.toneMappingExposure, 0.1f, 0.2f, "%.2f");
				settings.toneMappingExposure = std::clamp(settings.toneMappingExposure, 0.0f, 10.0f);
			}

			// Gamma
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);

				ImGui::AlignTextToFramePadding();
				ImGui::Text("Gamma");

				ImGui::TableNextColumn();

				ImGui::SetNextItemWidth(-FLT_MIN);

				ImGui::InputFloat("##Gamma", &settings.toneMappingGamma, 0.1f, 0.2f, "%.2f");
				settings.toneMappingGamma = std::clamp(settings.toneMappingGamma, 0.0f, 10.0f);
			}

			ImGui::EndTable();
		}

		// Debug views
		ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);

		if (ImGui::CollapsingHeader("Debug"))
		{
			ImGui::BeginTable("Properties", 2);

			// Debug Renderer
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);

				ImGui::AlignTextToFramePadding();
				ImGui::Text("Enable DebugRenderer");

				ImGui::TableNextColumn();

				ImGui::SetNextItemWidth(-FLT_MIN);

				ImGui::Checkbox("##Enable DebugRenderer", &settings.enableDebugRenderer);
			}

			// Debug GBuffer
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);

				ImGui::AlignTextToFramePadding();
				ImGui::Text("Show GBuffer");

				ImGui::TableNextColumn();

				ImGui::SetNextItemWidth(-FLT_MIN);

				ImGui::Checkbox("##Show GBuffer", &settings.enableDebugGBuffer);
			}

			// Debug ShadowMaps
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);

				ImGui::AlignTextToFramePadding();
				ImGui::Text("Show ShadowMaps");

				ImGui::TableNextColumn();

				ImGui::SetNextItemWidth(-FLT_MIN);

				ImGui::Checkbox("##Show ShadowMaps", &settings.enableDebugShadowMaps);
			}

			ImGui::EndTable();
		}

		// Metrics
		ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);

		if (ImGui::CollapsingHeader("Metrics"))
		{
			ImGui::BeginTable("Properties", 2);

			// Enable benchmarks
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);

				ImGui::AlignTextToFramePadding();
				ImGui::Text("Update time");

				ImGui::TableNextColumn();

				ImGui::SetNextItemWidth(-FLT_MIN);

				ImGui::InputFloat("##Update time", &settings.metricsUpdateTime, 0.5f, 0.5f, "%.3fs");
				settings.metricsUpdateTime = std::clamp(settings.metricsUpdateTime, 0.001f, 1000.0f);
			}

			// Enable benchmarks
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);

				ImGui::AlignTextToFramePadding();
				ImGui::Text("Enable benchmarks");

				ImGui::TableNextColumn();

				ImGui::SetNextItemWidth(-FLT_MIN);

				ImGui::Checkbox("##Enable benchmarks", &settings.enableBenchmarks);
			}

			ImGui::EndTable();
		}
	}

	ImGui::End();
}

void GuiSystem::showOverlay()
{
	const ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoDocking |
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoFocusOnAppearing |
		ImGuiWindowFlags_NoNav |
		ImGuiWindowFlags_NoMove;

	const float PAD = 10.0f;
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
	ImVec2 work_size = viewport->WorkSize;
	ImVec2 window_pos, window_pos_pivot;
	window_pos.x = work_pos.x + work_size.x - PAD;
	window_pos.y = work_pos.y + PAD;
	window_pos_pivot.x = 1.0f;
	window_pos_pivot.y = 0.0f;
	ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background

	if (ImGui::Begin("Overlay", nullptr, windowFlags))
	{
		const auto frameTime = m_elapsedTime / static_cast<float>(m_elapsedFrames);
		const auto fps = static_cast<unsigned>(1.0f / frameTime);

		ImGui::Text("%u fps", fps);
		ImGui::Text("%.03f ms", frameTime * 1000.0f);

		if (m_stats.size() > 0)
		{
			ImGui::Separator();

			for (const auto& stat : m_stats)
			{
				ImGui::Text("%s :", stat.first.c_str());

				ImGui::SameLine();

				std::ostringstream ss;

				ss.imbue(std::locale(""));
				
				ss << stat.second;

				ImGui::Text("%s", ss.str().c_str());
			}
		}

		if (Settings::instance().enableBenchmarks)
		{
			ImGui::Separator();

			ImGui::Text("Benchmark");

			for (auto& benchmark : m_benchmarks)
			{
				showBenchmark(*benchmark);
			}
		}
	}

	ImGui::End();
}

void GuiSystem::showBenchmark(const BenchmarkData& benchmark)
{
	ImGui::PushID(benchmark.label.c_str());

	ImGui::Indent();

	const auto percent = static_cast<unsigned>((benchmark.timeStep.getSeconds() * 100.0f) / m_elapsedTime);

	ImGui::TextColored(getBenchmarkColor(percent), "[%u%%]", percent);

	ImGui::SameLine();

	ImGui::Text(benchmark.label.c_str());

	ImGui::SameLine();

	ImGui::Text("%.03fms", benchmark.timeStep.getMilliSeconds() / static_cast<float>(m_elapsedFrames));

	for (auto& child : benchmark.children)
		showBenchmark(*child);

	ImGui::Unindent();

	ImGui::PopID();
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
