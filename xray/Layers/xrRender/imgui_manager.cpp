///////////////////////////////////////////////////////////////////////////////
// imgui_manager.cpp
// Менеджер для управления жизненным циклом ImGui и её расширений
// (C) 2026 Проект Б.О.Р.Ж.Т.Ч. (xray-engine.org)
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "stdafx.h"

#include "imgui_manager.h"

imgui_manager& imgui_manager::instance() {
	static imgui_manager instance;
	return instance;
}

imgui_manager::imgui_manager() : m_implot_ctx(nullptr, nullptr) {}

imgui_manager::~imgui_manager() {
	shutdown();
}

void imgui_manager::initialize() {
	Msg("* [ImGui] Initializing contexts...");
	if (ImGui::GetCurrentContext() != nullptr) {
		Msg("! [ImGui] Context already exists, this is an error!");
		return;
	}
	ImGui::CreateContext();
	if (ImGui::GetCurrentContext() == nullptr) {
		Msg("! [ImGui] Failed to create ImGui context!");
		return;
	}
	Msg("* [ImGui] ImGui context created successfully.");

	ImPlot::SetImGuiContext(ImGui::GetCurrentContext());
	m_implot_ctx = {ImPlot::CreateContext(), ImPlot::DestroyContext};
	if (m_implot_ctx) {
		ImPlot::SetCurrentContext(m_implot_ctx.get());
		Msg("* [ImGui] ImPlot global context created and activated.");
	} else {
		Msg("! [ImGui] Failed to create ImPlot context!");
	}
}

void imgui_manager::build_fonts() {
	Msg("* [ImGui] Building fonts...");

	ImGuiIO&							   io = ImGui::GetIO();
	xr_vector<std::pair<xr_string, float>> fonts;
	fonts.emplace_back("fonts\\IBMPlexSans-Regular.ttf", 24.0f);
	fonts.emplace_back("fonts\\IBMPlexSans-Bold.ttf", 24.0f);

	u32 loaded = 0;
	for (const auto& [path, size] : fonts) {
		ImFont* font = io.Fonts->AddFontFromFileTTF(path.c_str(), size, nullptr, io.Fonts->GetGlyphRangesCyrillic());
		if (font)
			++loaded;
		else
			Msg("~ [ImGui] Failed to load font: %s", path.c_str());
	}

	if (loaded == fonts.size())
		Msg("* [ImGui] All fonts loaded successfully.");
	else
		Msg("* [ImGui] Fonts built (%u/%zu).", loaded, fonts.size());
}

void imgui_manager::init_platform(HWND hwnd) {
	Msg("* [ImGui] Initializing platform backend (Win32)...");
	ImGui_ImplWin32_Init(hwnd);
	Msg("* [ImGui] Platform backend initialized.");
}

void imgui_manager::shutdown() {
	if (ImGui::GetCurrentContext() == nullptr) {
		Msg("! [ImGui] No context to shut down, this is an error!");
		return;
	}
	Msg("* [ImGui] Shutting down...");
	m_implot_ctx.reset();
	Msg("* [ImGui] ImPlot context destroyed.");
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	Msg("* [ImGui] ImGui context destroyed.");
}

ImPlotContext* imgui_manager::get_implot_context() const {
	return m_implot_ctx.get();
}
