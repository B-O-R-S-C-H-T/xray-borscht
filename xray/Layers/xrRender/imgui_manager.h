///////////////////////////////////////////////////////////////////////////////
// imgui_manager.h
// Менеджер для управления жизненным циклом ImGui и её расширений
// (C) 2026 Проект Б.О.Р.Ж.Т.Ч. (xray-engine.org)
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <imgui.h>
#include <addons/ImGuizmo/ImGuizmo.h>
#include <addons/implot/implot.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx9.h>
#include <backends/imgui_impl_dx11.h>

class ENGINE_API imgui_manager {
public:
	static imgui_manager& instance();

	void				 initialize();			   // только контексты ImGui + ImPlot
	void				 build_fonts();			   // построение шрифтов
	void				 init_platform(HWND hwnd); // бэкенд Win32
	void				 shutdown();			   // ImPlot + Win32 + ImGui

	ImPlotContext*		 get_implot_context() const;

private:
	imgui_manager();
	~imgui_manager();
	imgui_manager(const imgui_manager&)									 = delete;
	imgui_manager&						 operator=(const imgui_manager&) = delete;

	std::unique_ptr<ImPlotContext, void (*)(ImPlotContext*)> m_implot_ctx;
};
