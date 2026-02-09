#pragma once

#include "stdafx.h"
#include "imgui_setup.h"

void imgui_create_context() {
	Msg("* [ImGui] Creating context...");
	ImGui::CreateContext();
	if (ImGui::GetCurrentContext() == nullptr) {
		Msg("! [ImGui] Failed to create context!");
		return;
	} else {
		Msg("* [ImGui] Context created successfully.");
	}
}

void imgui_build_fonts() {
	ImGuiIO& io = ImGui::GetIO();

	xr_vector<std::pair<xr_string, float>> fonts;
	fonts.reserve(2);
	fonts.push_back(std::make_pair("fonts\\IBMPlexSans-Regular.ttf", 24.0f));
	fonts.push_back(std::make_pair("fonts\\IBMPlexSans-Bold.ttf", 24.0f));

	u32 loaded_count = 0;

	for (const auto& config : fonts) {
		const xr_string& font_path = config.first;
		float			 font_size = config.second;

		Msg("* [ImGui] Attempting to load font from %s", font_path.c_str());
		ImFont* font = io.Fonts->AddFontFromFileTTF(font_path.c_str(), font_size, nullptr, io.Fonts->GetGlyphRangesCyrillic());
		if (font == nullptr) {
			Msg("~ [ImGui] Failed to load %s!", font_path.c_str());
		} else {
			loaded_count++;
		}
	}

	if (loaded_count == fonts.size()) {
		Msg("* [ImGui] Fonts built successfully.");
	} else if (loaded_count > 0) {
		Msg("* [ImGui] Fonts built (%u out of %u custom fonts loaded).", loaded_count, fonts.size());
	} else {
		Msg("* [ImGui] Fonts built using default ImGui font.");
	}

	if (loaded_count == 0) {
		Msg("~ [ImGui] All custom fonts failed to load! Check if 'fonts/' directory exists and contains the font files.");
	}
}

void imgui_destroy_context() {
	if (ImGui::GetCurrentContext() == nullptr) {
		Msg("! [ImGui] No context to shutdown!");
		return;
	}

	ImGui_ImplWin32_Shutdown();
	Msg("* [ImGui] Win32 shutdown complete.");

	Msg("* [ImGui] Destroying context...");
	ImGui::DestroyContext();
	Msg("* [ImGui] Context destroyed.");
}
