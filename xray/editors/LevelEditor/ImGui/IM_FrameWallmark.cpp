#include "stdafx.h"
#pragma hdrstop

#include "IM_FrameWallmark.h"

#include <imgui.h>

#include "../ECore/Editor/ui_main.h"
#include "../Edit/ESceneWallmarkTools.h"
#include "../Edit/Scene.h"

void IM_FrameWallmark::Render() {
	if (!parent_tool) {
		parent_tool = dynamic_cast<ESceneWallmarkTool*>(Scene->GetTool(OBJCLASS_WM));
		VERIFY(parent_tool);
	}

//	if (ImGui::CollapsingHeader("Wallmark Commands", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) {
	// возможно, когда-нибудь кто-то напишет систему автоматической расстановки декалей...
// }

	ImGui::Separator();

	if (ImGui::CollapsingHeader("Wallmark Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
		if (!m_props_initialized) {
			PropItemVec items;
			parent_tool->FillProp("Wallmark", items);
			m_wallmark_props.AssignItems(std::move(items), true, false);
			m_props_initialized = true;
		}

		ImGui::BeginChild("##wallmark_properties_child", ImVec2(0, 0), false);
		m_wallmark_props.Render();
		ImGui::EndChild();
	}
}
