#include "stdafx.h"
#pragma hdrstop

#include "IM_FrameLight.h"

#include <imgui.h>
#include <imgui_user.h>

#include "../Edit/ELight.h"
#include "../Edit/Scene.h"
#include "../Edit/ESceneLightTools.h"
#include "editors/ECore/Editor/UI_MainCommand.h"

IM_FrameLight::IM_FrameLight() : m_sceneProps('\\', false, false) {}

void IM_FrameLight::UseInD3D(bool bAll, bool bFlag) {
	ObjectIt _F = Scene->FirstObj(OBJCLASS_LIGHT);
	ObjectIt _E = Scene->LastObj(OBJCLASS_LIGHT);
	for (; _F != _E; _F++) {
		CLight* L = (CLight*)*_F;
		if (bAll) {
			L->AffectD3D(bFlag);
		} else {
			if (L->Selected() && L->Visible())
				L->AffectD3D(bFlag);
		}
	}
	// UI->RedrawScene();
}

void IM_FrameLight::OnAdd() {
	IM_Storage& storage	  = EPrefs->GetSectionStorage("im_frame_light");
	m_show_useind3d		  = storage.GetBool("show_useind3d_panel", true);
	m_show_scene_lighting = storage.GetBool("show_scene_lighting_panel", true);
}

void IM_FrameLight::OnRemove() {
	IM_Storage& storage = EPrefs->GetSectionStorage("im_frame_light");
	storage.PutBool("show_useind3d_panel", m_show_useind3d);
	storage.PutBool("show_scene_lighting_panel", m_show_scene_lighting);
	storage.Save();
}

void IM_FrameLight::UpdateSceneLightingProps() {
	ESceneLightTool* lt = dynamic_cast<ESceneLightTool*>(Scene->GetTool(OBJCLASS_LIGHT));
	if (!lt)
		return;

	m_sceneToolItems.clear();
	lt->FillLightToolProp(m_sceneToolItems);

	m_sceneProps.AssignItems(m_sceneToolItems, true, false);
}

void IM_FrameLight::Render() {
	ESceneLightTool* lt = dynamic_cast<ESceneLightTool*>(Scene->GetTool(OBJCLASS_LIGHT));
	if (!lt)
		return;

	if (ImGui::CollapsingHeader("Scene Lighting", ImGuiTreeNodeFlags_DefaultOpen)) {
		if (m_show_scene_lighting) {
			UpdateSceneLightingProps();

			m_sceneProps.Render();
		}
	}

	if (ImGui::CollapsingHeader("Affect in D3D", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Columns(2, NULL, false);

		if (ImGui::MenuItem("Enable Sel##light_enable_sel"))
			UseInD3D(false, true);

		if (ImGui::MenuItem("Disable Sel##light_disable_sel"))
			UseInD3D(false, false);

		ImGui::NextColumn();

		if (ImGui::MenuItem("Enable All##light_enable_all"))
			UseInD3D(true, true);

		if (ImGui::MenuItem("Disable All##light_disable_all"))
			UseInD3D(true, false);

		ImGui::Columns(1);
	}
}
