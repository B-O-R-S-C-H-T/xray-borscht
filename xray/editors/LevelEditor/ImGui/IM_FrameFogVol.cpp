#include "stdafx.h"
#pragma hdrstop

#include "IM_FrameFogVol.h"

#include <imgui.h>
#include <imgui_user.h>

#include "../Edit/ESceneFogVolumeTools.h"
#include "../Edit/Scene.h"

#pragma package(smart_init)

void IM_FrameFogVol::OnAdd() {
	m_parent_tool = dynamic_cast<ESceneFogVolumeTool*>(Scene->GetOTool(OBJCLASS_FOG_VOL));
	VERIFY(m_parent_tool);

	IM_Storage& storage = EPrefs->GetSectionStorage("im_frame_fog");
	m_show_commands		= storage.GetBool("show_commands_panel", true);
}

void IM_FrameFogVol::OnRemove() {
	IM_Storage& storage = EPrefs->GetSectionStorage("im_frame_fog");
	storage.PutBool("show_commands_panel", m_show_commands);
	storage.Save();
}

void IM_FrameFogVol::Render() {
	if (ImGui::CollapsingPanel("Commands", &m_show_commands)) {
		ImGui::Columns(2, NULL, false);

		if (ImGui::MenuItem("Group Selected##fogvol_group"))
			m_parent_tool->GroupSelected();

		ImGui::NextColumn();

		if (ImGui::MenuItem("UnGroup Selected##fogvol_ungroup"))
			m_parent_tool->UnGroupCurrent();

		ImGui::Columns(1);
	}
}
