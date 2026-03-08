#include "stdafx.h"
#pragma hdrstop

#include "IM_ContextMenu.h"

#include <ImGui.h>

#include "../Edit/UI_LevelMain.h"
#include "../Edit/UI_LevelTools.h"

void IM_ContextMenu::Render() {
	if (m_open) {
		ImGui::OpenPopup("im_contextmenu");
		m_open = false;
	}

	if (ImGui::BeginPopup("im_contextmenu")) {
		bool detach = false;

		if (ImGui::Selectable("##tearoff", false, ImGuiSelectableFlags_SpanAllColumns, ImVec2(0.0f, TEAR_OFF_HEIGHT)))
			detach = true;

		{
			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			ImVec2		p_min	  = ImGui::GetItemRectMin();
			ImVec2		p_max	  = ImGui::GetItemRectMax();
			float		y		  = (p_min.y + p_max.y) * 0.5f;
			draw_list->AddLine(ImVec2(p_min.x + LINE_PADDING, y),
							   ImVec2(p_max.x - LINE_PADDING, y),
							   ImGui::GetColorU32(ImGuiCol_Separator),
							   LINE_THICKNESS);
		}

		ShowMenu();

		ImGui::EndPopup();

		if (detach) {
			m_detach = true;
			ImGui::SetNextWindowPos(ImGui::GetIO().MousePos, ImGuiCond_Always);
		}
	}

	if (m_detach) {
		ImGui::SetNextWindowSize(INITIAL_WINDOW_SIZE, ImGuiCond_FirstUseEver);
		if (ImGui::Begin("Context Menu", &m_detach, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoCollapse)) {
			ShowMenu();
		}
		ImGui::End();
	}
}

void IM_ContextMenu::Open() {
	m_open = true;
}

void IM_ContextMenu::ShowMenu() const
{
	if (ImGui::MenuItem("Copy (selection)"))
		ExecCommand(COMMAND_COPY);

	if (ImGui::MenuItem("Paste (selection)"))
		ExecCommand(COMMAND_PASTE);

	if (ImGui::MenuItem("Cut (selection)"))
		ExecCommand(COMMAND_CUT);

	if (ImGui::MenuItem("Duplicate (selection)"))
		ExecCommand(COMMAND_DUPLICATE);

	ImGui::SeparatorText("Visibility");

	if (ImGui::MenuItem("Hide Selected"))
		ExecCommand(COMMAND_HIDE_SEL, FALSE);

	if (ImGui::MenuItem("Hide Unselected"))
		ExecCommand(COMMAND_HIDE_UNSEL);

	if (ImGui::MenuItem("Hide All"))
		ExecCommand(COMMAND_HIDE_ALL, FALSE);

	if (ImGui::MenuItem("Unhide All"))
		ExecCommand(COMMAND_HIDE_ALL, TRUE);

	ImGui::SeparatorText("Locking");

	if (ImGui::MenuItem("Lock Selected"))
		ExecCommand(COMMAND_LOCK_SEL, TRUE);

	if (ImGui::MenuItem("Lock Unselected"))
		ExecCommand(COMMAND_LOCK_UNSEL, TRUE);

	if (ImGui::MenuItem("Lock All"))
		ExecCommand(COMMAND_LOCK_ALL, TRUE);

	if (ImGui::MenuItem("Unlock Selected"))
		ExecCommand(COMMAND_LOCK_SEL, FALSE);

	if (ImGui::MenuItem("Unlock Unselected"))
		ExecCommand(COMMAND_LOCK_UNSEL, FALSE);

	if (ImGui::MenuItem("Unlock All"))
		ExecCommand(COMMAND_LOCK_ALL, FALSE);

	ImGui::Separator();

	if (ImGui::MenuItem("Properties..."))
		ExecCommand(COMMAND_SHOW_PROPERTIES);
}

IM_ContextMenu imContextMenu;
