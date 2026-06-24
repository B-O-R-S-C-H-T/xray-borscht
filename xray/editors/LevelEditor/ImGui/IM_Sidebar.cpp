#include "stdafx.h"
#pragma hdrstop

#include "IM_Sidebar.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_user.h>

#include "../ECore/ImGui/utf8.h"
#include "../Edit/CustomObject.h"
#include "../Edit/UI_LevelMain.h"
#include "../Edit/UI_LevelTools.h"
#include "../Edit/scene.h"
#ifndef NO_VCL
#include "FormAbout.h"
#include "FrmDBXPacker.h"
#endif

extern XRCORE_API u32	 build_id;
extern XRCORE_API LPCSTR build_date;

void IM_Sidebar::OnAdd() {
	fraObject.OnAdd();
	fraLight.OnAdd();
	fraFogVol.OnAdd();
	fraDetail.OnAdd();
	fraSector.OnAdd();
	fraPortal.OnAdd();
	fraShape.OnAdd();
	fraPS.OnAdd();
	fraWayPoint.OnAdd();
	fraSpawn.OnAdd();
	fraGroup.OnAdd();
	fraAIMap.OnAdd();

	IM_Storage& s			= EPrefs->GetSectionStorage("im_sidebar");

	m_enable_snap_list		= s.GetBool("enable_snap_list");
	m_select_snap_objs_mode = s.GetBool("select_snap_objs_mode");
	m_detach_tool_frame		= s.GetBool("detach_tool_frame");
	m_show_snaplist			= s.GetBool("show_snaplist_panel", true);
}

void IM_Sidebar::OnRemove() {
	fraObject.OnRemove();
	fraLight.OnRemove();
	fraFogVol.OnRemove();
	fraDetail.OnRemove();
	fraSector.OnRemove();
	fraPortal.OnRemove();
	fraShape.OnRemove();
	fraPS.OnRemove();
	fraWayPoint.OnRemove();
	fraWallmark.OnRemove();
	fraSpawn.OnRemove();
	fraGroup.OnRemove();
	fraAIMap.OnRemove();

	IM_Storage& s = EPrefs->GetSectionStorage("im_sidebar");

	s.PutBool("enable_snap_list", m_enable_snap_list);
	s.PutBool("select_snap_objs_mode", m_select_snap_objs_mode);
	s.PutBool("detach_tool_frame", m_detach_tool_frame);
	s.PutBool("show_snaplist_panel", m_show_snaplist);

	s.Save();
}

void IM_Sidebar::Render() {
	ImGuiIO&	io			   = ImGui::GetIO();
	const float initial_height = ImGui::GetFrameHeight();

	ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - m_sidebar_width, initial_height), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Level Editor", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse)) {

		{ // fixme: высота листбокса не учитывает масштабирование шрифта
			ObjClassID		curr_id		  = LTools->CurrentClassID();
			ESceneToolBase* curr_tool	  = Scene->GetTool(curr_id);
			const char*		preview		  = curr_tool ? curr_tool->ClassDesc() : "Unknown Mode";
			xr_string		combo_preview = preview;
			if (curr_tool && !curr_tool->IsVisible())
				combo_preview += " (hidden)";

			int item_count = 0;
			for (ObjClassID i = OBJCLASS_FIRST_CLASS; i < OBJCLASS_COUNT; i++) {
				if (Scene->GetTool(i))
					item_count++;
			}

			if (ImGui::Button(combo_preview.c_str(), ImVec2(-1, 0))) {
				ImGui::OpenPopup("Edit Mode##sidebar_mode_popup");
			}

			if (ImGui::BeginPopup("Edit Mode##sidebar_mode_popup")) {
				ImGui::SetNextWindowSize(ImVec2(320.0f, 0.0f));

				const ImGuiStyle& style			  = ImGui::GetStyle();
				float			  line_height	  = ImGui::GetTextLineHeight();
				float			  item_spacing	  = style.ItemSpacing.y;
				float			  frame_padding	  = style.FramePadding.y * 2.0f;
				float			  window_padding  = style.WindowPadding.y * 2.0f;
				float			  border_size	  = style.WindowBorderSize * 2.0f;

				float			  required_height = (line_height + item_spacing) * (item_count + 3) + frame_padding + window_padding + border_size;

			float max_height = ImGui::GetIO().DisplaySize.y * 0.80f;
			required_height  = (required_height < max_height) ? required_height : max_height;

				if (ImGui::BeginListBox("##editmode_listbox", ImVec2(400.0f, required_height))) {
					ImGui::Columns(2, "##editmode_cols_popup", false);
					ImGui::SetColumnWidth(0, 260.0f);

					for (ObjClassID i = OBJCLASS_FIRST_CLASS; i < OBJCLASS_COUNT; i++) {
						ESceneToolBase* tool = Scene->GetTool(i);
						if (!tool)
							continue;

						ImGui::PushID((int)i);
						bool is_selected = (i == curr_id);

						if (ImGui::Selectable(tool->ClassDesc(), is_selected)) {
							ExecCommand(COMMAND_CHANGE_TARGET, i);
							ImGui::CloseCurrentPopup();
						}

						ImGui::NextColumn();

						bool visible = tool->IsVisible();
						if (ImGui::Checkbox("##vis", &visible)) {
							ExecCommand(COMMAND_SHOW_TARGET, i, visible);
						}

						ImGui::NextColumn();
						ImGui::PopID();

						if (is_selected)
							ImGui::SetItemDefaultFocus();
					}

					ImGui::Columns(1);
				}
				ImGui::EndListBox();

				ImGui::EndPopup();
			}
		}

		if (ImGui::CollapsingPanel("Snap list", &m_show_snaplist)) {
			if (ImGui::BeginMenu("Commands##snap_commands_sidebar")) {
				if (ImGui::MenuItem("Make list from selected##snap_make_list"))
					ExecCommand(COMMAND_SET_SNAP_OBJECTS);
				if (ImGui::MenuItem("Select objects from list##snap_select_list"))
					ExecCommand(COMMAND_SELECT_SNAP_OBJECTS);
				ImGui::Separator();
				if (ImGui::MenuItem("Add selected to list##snap_add_selected"))
					ExecCommand(COMMAND_ADD_SEL_SNAP_OBJECTS);
				if (ImGui::MenuItem("Remove selected from list##snap_remove_selected"))
					ExecCommand(COMMAND_DEL_SEL_SNAP_OBJECTS);
				ImGui::Separator();
				if (ImGui::MenuItem("Clear list##snap_clear_list")) {
					if (ELog.DlgMsg(mtConfirmation, TMsgDlgButtons() << mbYes << mbNo, "Are you sure to clear snap objects?") == mrYes)
						ExecCommand(COMMAND_CLEAR_SNAP_OBJECTS);
				}
				ImGui::EndMenu();
			}
			ImGui::Selectable("Enable/Show snap list##snap_enable", &m_enable_snap_list);
			ImGui::Columns(3, NULL, false);
			if (ImGui::Selectable("+/- Mode##snap_mode", &m_select_snap_objs_mode))
				if (m_select_snap_objs_mode && (etaSelect != Tools->GetAction())) {
					ELog.Msg(mtError, "Before modify snap list activate select mode!");
					m_select_snap_objs_mode = false;
				}
			ImGui::NextColumn();
			if (ImGui::MenuItem("Clear##snap_clear_sidebar"))
				ExecCommand(COMMAND_CLEAR_SNAP_OBJECTS);
			ImGui::NextColumn();
			if (ImGui::MenuItem("Edit...##snap_edit"))
				ExecCommand(COMMAND_EDIT_SNAP_OBJECTS);
			ImGui::Columns(1);
			ObjectList* ol = Scene->GetSnapList(true);
			int			i  = 0;
			ImGui::BeginChild("snap list", ImVec2(0, 150), true);
			for (ObjectIt it = ol->begin(), end = ol->end(); it != end; it++) {
				ImGui::PushID(i);
				ImGui::Text("%u: %s", i, (*it)->FName.c_str());
				ImGui::PopID();
				i++;
			}
			ImGui::EndChild();
		}

		if (m_detach_tool_frame) {
			ImGui::Begin("Tool Options", &m_detach_tool_frame);
			RenderToolFrame();
			ImGui::End();
		} else {
			bool detach = ImGui::Button("##detach_tool_opts", ImVec2(-1, 5));
			RenderToolFrame();
			m_detach_tool_frame = detach;
		}

		m_sidebar_width = ImGui::GetWindowSize().x;

		ImVec2 curr_pos = ImGui::GetWindowPos();
		ImGui::SetWindowPos(ImVec2(io.DisplaySize.x - m_sidebar_width, curr_pos.y));

		ImGui::End();
	}

	// Такой вот вынужденный финт, чтобы строка меню не перекрывала боковую панель,
	// и чтобы при этом такое решение не конфликтовало с модальными окнами.
	if (!ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopup)) {
		ImGui::BringWindowToDisplayFront(ImGui::FindWindowByName("Level Editor"));
	}
}

void IM_Sidebar::RenderToolFrame() {
	switch (LTools->CurrentClassID()) {
		case OBJCLASS_SCENEOBJECT: 	fraObject.Render();   break;
		case OBJCLASS_LIGHT:        fraLight.Render();    break;
		case OBJCLASS_FOG_VOL:		fraFogVol.Render();   break;
		case OBJCLASS_DO:			fraDetail.Render();   break;
		case OBJCLASS_SECTOR:		fraSector.Render();   break;
		case OBJCLASS_PORTAL:		fraPortal.Render();   break;
		case OBJCLASS_SHAPE:		fraShape.Render();    break;
		case OBJCLASS_PS:			fraPS.Render();       break;
		case OBJCLASS_WAY:			fraWayPoint.Render(); break;
		case OBJCLASS_WM:			fraWallmark.Render(); break;
		case OBJCLASS_SPAWNPOINT:	fraSpawn.Render();    break;
		case OBJCLASS_GROUP:		fraGroup.Render();    break;
		case OBJCLASS_AIMAP:		fraAIMap.Render();    break;
	}
}

IM_Sidebar g_imSidebar;
