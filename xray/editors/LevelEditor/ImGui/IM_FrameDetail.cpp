#include "stdafx.h"
#pragma hdrstop

#include "IM_FrameDetail.h"

#include <imgui.h>

#include "../Edit/ESceneDOTools.h"
#include "../Edit/Scene.h"
#include "IM_DetailShuffle.h"
#include "../ECore/Editor/ui_main.h"

void IM_FrameDetail::Render() {
	if (!parent_tool) {
		parent_tool = dynamic_cast<EDetailManager*>(Scene->GetTool(OBJCLASS_DO));
		VERIFY(parent_tool);
	}

	if (ImGui::CollapsingHeader("Detail Objects Commands", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) {
		if (ImGui::MenuItem("First Initialize##detail_first_init")) {
			if (parent_tool->Initialize())
				Scene->UndoSave();
		}
		if (ImGui::MenuItem("Reinitialize All##detail_reinit_all")) {
			if (parent_tool->Reinitialize()) {
				Scene->UndoSave();
				RefreshProperties();
			}
		}
		if (ImGui::MenuItem("Reinitialize Objects Only##detail_reinit_objs")) {
			if (parent_tool->UpdateObjects(true, false))
				Scene->UndoSave();
		}
		if (ImGui::MenuItem("Reinitialize Selected Slot Objects##detail_reinit_sel_slots")) {
			if (parent_tool->UpdateObjects(false, true))
				Scene->UndoSave();
		}

		ImGui::Separator();

		if (ImGui::MenuItem("Update Renderer##detail_update_renderer")) {
			parent_tool->InvalidateCache();
			Scene->UndoSave();
		}

		ImGui::Separator();

		if (ImGui::MenuItem("Clear Slots##detail_clear_slots")) {
			if (ELog.DlgMsg(mtConfirmation, TMsgDlgButtons() << mbYes << mbNo, "Are you sure to reset slots?") == mrYes) {
				parent_tool->ClearSlots();
				Scene->UndoSave();
			}
		}
		if (ImGui::MenuItem("Clear Details##detail_clear_details")) {
			if (ELog.DlgMsg(mtConfirmation, TMsgDlgButtons() << mbYes << mbNo, "Are you sure to clear details?") == mrYes) {
				ExecCommand(COMMAND_UPDATE_PROPERTIES);
				parent_tool->Clear();
				Scene->UndoSave();
				RefreshProperties();
			}
		}

		ImGui::Separator();

		if (ImGui::MenuItem("Object List##detail_object_list")) {
			IM_DetailShuffle::Run(dynamic_cast<EDetailManager*>(Scene->GetTool(OBJCLASS_DO)));
		}
	}

	ImGui::Separator();

	if (ImGui::CollapsingHeader("Detail Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
		if (!m_props_initialized) {
			PropItemVec items;
			parent_tool->FillProp("Detail", items);
			m_detail_props.AssignItems(std::move(items), true, false);
			m_props_initialized = true;
		}

		ImGui::BeginChild("##detail_properties_child", ImVec2(0, 0), false);
		m_detail_props.Render();
		ImGui::EndChild();
	}
}

#pragma package(smart_init)
