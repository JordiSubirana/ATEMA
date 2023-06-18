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

#ifndef ATEMA_SANDBOX_GUISYSTEM_HPP
#define ATEMA_SANDBOX_GUISYSTEM_HPP

#include "System.hpp"
#include "../Stats.hpp"

#include <Atema/Renderer/UI/ImGui.hpp>

class GuiSystem : public System
{
public:
	GuiSystem() = delete;
	GuiSystem(const at::Ptr<at::RenderWindow>& renderWindow);
	virtual ~GuiSystem();

	void update(at::TimeStep timeStep) override;
	void onEvent(at::Event& event) override;

	void updateBenchmarks(int elapsedFrames);
	void updateStats(int elapsedFrames);

private:
	struct WidgetState
	{
		float value = 0.0f;

		bool active = false;
		bool hovered = false;
	};

	void updateUI();
	void showSettings();
	void showOverlay();
	void showBenchmark(const at::BenchmarkData& benchmark);

	ImGuiID getWidgetID(const char* label) const;
	WidgetState& getWidgetState(const char* label);

	ImVec4& getColor(ImGuiCol_ color);

	bool button(const char* label, const ImVec2& size = ImVec2(0, 0));

	void pushState(WidgetState& state);
	void popState(WidgetState& state);

	at::Ptr<at::UiContext> m_uiContext;

	at::TimeStep m_timeStep;

	std::unordered_map<ImGuiID, WidgetState> m_widgetStates;

	std::vector<at::Ptr<at::BenchmarkData>> m_benchmarks;
	Stats m_stats;
	int m_elapsedFrames;
	float m_elapsedTime;
};

#endif
