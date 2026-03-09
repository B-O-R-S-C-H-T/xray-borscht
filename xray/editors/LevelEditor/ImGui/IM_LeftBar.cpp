#include "stdafx.h"
#pragma hdrstop
#include "IM_LeftBar.h"

#include <imgui.h>
#include <imgui_user.h>
#include <shellapi.h>

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

void IM_LeftBar::OnAdd() {
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

	IM_Storage s(false, "level.ini", "IM_LeftBar");
	m_enable_snap_list		= s.GetBool("enable_snap_list");
	m_select_snap_objs_mode = s.GetBool("select_snap_objs_mode");
	m_detach_tool_frame		= s.GetBool("detach_tool_frame");
	m_show_snaplist			= s.GetBool("show_snaplist_panel", true);
}

void IM_LeftBar::OnRemove() {
	fraObject.OnRemove();
	fraLight.OnRemove();
	fraFogVol.OnRemove();
	fraDetail.OnRemove();
	fraSector.OnRemove();
	fraPortal.OnRemove();
	fraShape.OnRemove();
	fraPS.OnRemove();
	fraWayPoint.OnRemove();
	fraSpawn.OnRemove();
	fraGroup.OnRemove();
	fraAIMap.OnRemove();

	IM_Storage s(true, "level.ini", "IM_LeftBar");

	s.PutBool("enable_snap_list", m_enable_snap_list);
	s.PutBool("select_snap_objs_mode", m_select_snap_objs_mode);
	s.PutBool("detach_tool_frame", m_detach_tool_frame);
	s.PutBool("show_snaplist_panel", m_show_snaplist);
}

void IM_LeftBar::Render() {
	if (ImGui::BeginMainMenuBar()) {
		RenderMainMenu();
		ImGui::EndMainMenuBar();
	}

	ImGui::Begin("Level Editor", NULL);

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
			ImGui::OpenPopup("Edit Mode##leftbar_mode_popup");
		}

		if (ImGui::BeginPopup("Edit Mode##leftbar_mode_popup")) {
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
		if (ImGui::BeginMenu("Commands##snap_commands_leftbar")) {
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
		if (ImGui::MenuItem("Clear##snap_clear_leftbar"))
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

	ImGui::End();
}

void IM_LeftBar::RenderToolFrame() {
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
		case OBJCLASS_SPAWNPOINT:	fraSpawn.Render();    break;
		case OBJCLASS_GROUP:		fraGroup.Render();    break;
		case OBJCLASS_AIMAP:		fraAIMap.Render();    break;
	}
}

void IM_About::Render() {
	if (!m_open)
		return;

	ImGuiIO& io = ImGui::GetIO();
	ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSizeConstraints(ImVec2(360.0f, 0.0f), ImVec2(FLT_MAX, FLT_MAX));

	ImGui::OpenPopup("About##about_popup");
	if (ImGui::BeginPopupModal("About##about_popup",
							   &m_open,
							   ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings)) {
		static const ImVec4 LINK_COLOR		   = ImVec4(0.12f, 0.56f, 1.0f, 1.0f);
		static const ImVec4 LINK_HOVER_COLOR   = ImVec4(0.12f, 0.56f, 1.0f, 1.0f);
		static const ImVec4 VISITED_COLOR	   = ImVec4(0.27f, 0.51f, 0.71f, 1.0f);

		static const float	BUTTON_WIDTH_RATIO = 0.33f;

		auto				CenterText		   = [](const char* text)
		{
			float w	 = ImGui::GetWindowSize().x;
			float tw = ImGui::CalcTextSize(text).x;
			ImGui::SetCursorPosX((w - tw) * 0.5f);
			ImGui::Text("%s", text);
		};

		auto CenterBoldText = [&](const char* text)
		{
			ImGui::PushFont(io.Fonts->Fonts[1]);
			float w	 = ImGui::GetWindowSize().x;
			float tw = ImGui::CalcTextSize(text).x;
			ImGui::SetCursorPosX((w - tw) * 0.5f);
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", text);
			ImGui::PopFont();
		};

		auto OpenURL = [](const char* url)
		{
#ifdef _WIN32
			HINSTANCE res = ShellExecuteA(NULL, "open", url, NULL, NULL, SW_SHOW);
			if ((intptr_t)res <= 32) {
				ELog.Msg(mtError, "Failed to open URL: %s (error code: %d)", url, (intptr_t)res);
			}
#else
		// привет тем психам, что решат это переписать под юникс систем
#endif
		};

		ImGui::Dummy(ImVec2(0, 8.0f));
		CenterBoldText("X-Ray B.O.R.S.C.H.T. Level Editor");
		ImGui::Dummy(ImVec2(0, 8.0f));

		ImGui::Separator();

		char buf[128];
		sprintf(buf, "Build %u, %s", build_id, build_date);
		CenterText(buf);

		CenterText("(C) 2008 GSC Game World");
		CenterText("(C) 2017-2026 B.O.R.S.C.H.T. Project");

		ImGui::Separator();

		float		window_width = ImGui::GetWindowSize().x;

		const char* src_text	 = "Source Code";
		float		src_w		 = ImGui::CalcTextSize(src_text).x;
		ImGui::SetCursorPosX((window_width - src_w) * 0.5f);

		ImGui::TextColored(visited_source ? VISITED_COLOR : LINK_COLOR, "%s", src_text);

		if (ImGui::IsItemHovered()) {
			ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
			ImVec2 min = ImGui::GetItemRectMin();
			ImVec2 max = ImGui::GetItemRectMax();
			ImGui::GetWindowDrawList()->AddLine(ImVec2(min.x, max.y + 2), ImVec2(max.x, max.y + 2), ImGui::GetColorU32(LINK_COLOR), 1.0f);
		}

		if (ImGui::IsItemClicked()) {
			visited_source = true;
			OpenURL("https://bitbucket.org/stalker/xray-borscht/src/borscht/");
		}

		const char* upd_text = "Forum Thread";
		float		upd_w	 = ImGui::CalcTextSize(upd_text).x;
		ImGui::SetCursorPosX((window_width - upd_w) * 0.5f);

		ImGui::TextColored(visited_updates ? VISITED_COLOR : LINK_COLOR, "%s", upd_text);

		if (ImGui::IsItemHovered()) {
			ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
			ImVec2 min = ImGui::GetItemRectMin();
			ImVec2 max = ImGui::GetItemRectMax();
			ImGui::GetWindowDrawList()->AddLine(ImVec2(min.x, max.y + 2), ImVec2(max.x, max.y + 2), ImGui::GetColorU32(LINK_COLOR), 1.0f);
		}

		if (ImGui::IsItemClicked()) {
			visited_updates = true;
			OpenURL("https://forum.xray-engine.org/index.php/topic,252");
		}

		ImGui::Separator();

		ImGui::Dummy(ImVec2(0, 8.0f));
		float button_w = window_width * BUTTON_WIDTH_RATIO;
		ImGui::SetCursorPosX((window_width - button_w) * 0.5f);
		if (ImGui::Button("OK", ImVec2(button_w, 0))) {
			m_open = false;
		}
		ImGui::Dummy(ImVec2(0, 8.0f));

		ImGui::EndPopup();
	}
}

void IM_LeftBar::RenderMainMenu() {
	if (ImGui::BeginMenu("File##main_file")) {
		if (ImGui::MenuItem("Clear Scene##clear_scene"))
			ExecCommand(COMMAND_CLEAR);
		if (ImGui::MenuItem("Open Scene...##open_scene"))
			ExecCommand(COMMAND_LOAD);
		if (ImGui::MenuItem("Save Scene##save_scene"))
			ExecCommand(COMMAND_SAVE);
		if (ImGui::MenuItem("Save Scene As...##save_scene_as"))
			ExecCommand(COMMAND_SAVE, 0, 1);
		ImGui::Separator();
		if (ImGui::MenuItem("Open Selection##file_open_sel"))
			ExecCommand(COMMAND_LOAD_SELECTION);
		if (ImGui::MenuItem("Save Selection As...##file_save_sel"))
			ExecCommand(COMMAND_SAVE_SELECTION);
		ImGui::Separator();
		if (ImGui::MenuItem("Export Scene as OBJ...##scene_export_all"))
			Scene->ExportObj(false);
		if (ImGui::MenuItem("Export Selection as OBJ...##scene_export_sel"))
			Scene->ExportObj(true);
#ifndef NO_VCL
		if (ImGui::MenuItem("Make Pack##file_make_pack"))
			TDB_packer::ActivatePacker();
#endif
		ImGui::Separator();
		if (!!EPrefs->scene_recent_count) {
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
			for (AStringIt it = EPrefs->scene_recent_list.begin(); it != EPrefs->scene_recent_list.end(); it++) {
				if (ImGui::MenuItem(it->c_str()))
					ExecCommand(COMMAND_LOAD, xr_string(*it));
			}
			ImGui::PopStyleColor();
		}
		ImGui::Separator();
		if (ImGui::MenuItem("Quit##file_quit"))
			ExecCommand(COMMAND_QUIT);
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Edit##main_edit")) {
		if (ImGui::MenuItem("Undo##edit_undo"))
			ExecCommand(COMMAND_UNDO);
		if (ImGui::MenuItem("Redo##edit_redo"))
			ExecCommand(COMMAND_REDO);
		ImGui::Separator();
		if (ImGui::MenuItem("Cut##edit_cut"))
			ExecCommand(COMMAND_CUT);
		if (ImGui::MenuItem("Copy##edit_copy"))
			ExecCommand(COMMAND_COPY);
		if (ImGui::MenuItem("Paste##edit_paste"))
			ExecCommand(COMMAND_PASTE);
		ImGui::Separator();
		if (ImGui::MenuItem("Select All##sel_all"))
			ExecCommand(COMMAND_SELECT_ALL);
		if (ImGui::MenuItem("Deselect All##sel_deselect"))
			ExecCommand(COMMAND_DESELECT_ALL);
		if (ImGui::MenuItem("Invert Selection##sel_invert"))
			ExecCommand(COMMAND_INVERT_SELECTION_ALL);
		ImGui::Separator();
		if (ImGui::MenuItem("Multi Rename##tools_multi_rename"))
			ExecCommand(COMMAND_MULTI_RENAME_OBJECTS);
		ImGui::Separator();
		if (ImGui::MenuItem("Preferences##main_prefs"))
			ExecCommand(COMMAND_EDITOR_PREF);
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Tools##main_tools")) {
		if (ImGui::MenuItem("Hide Selection##vis_hide_sel"))
			ExecCommand(COMMAND_HIDE_SEL, FALSE);
		if (ImGui::MenuItem("Hide Unselected##vis_hide_unsel"))
			ExecCommand(COMMAND_HIDE_UNSEL);
		if (ImGui::MenuItem("Hide All##vis_hide_all"))
			ExecCommand(COMMAND_HIDE_ALL, FALSE);
		if (ImGui::MenuItem("Unhide All##vis_unhide_all"))
			ExecCommand(COMMAND_HIDE_ALL, TRUE);
		ImGui::Separator();
		if (ImGui::MenuItem("Lock Selection##lock_sel"))
			ExecCommand(COMMAND_LOCK_SEL, TRUE);
		if (ImGui::MenuItem("Lock Unselected##lock_unsel"))
			ExecCommand(COMMAND_LOCK_UNSEL, TRUE);
		if (ImGui::MenuItem("Lock All##lock_all"))
			ExecCommand(COMMAND_LOCK_ALL, TRUE);
		if (ImGui::MenuItem("Unlock Selection##unlock_sel"))
			ExecCommand(COMMAND_LOCK_SEL, FALSE);
		if (ImGui::MenuItem("Unlock Unselected##unlock_unsel"))
			ExecCommand(COMMAND_LOCK_UNSEL, FALSE);
		if (ImGui::MenuItem("Unlock All##unlock_all"))
			ExecCommand(COMMAND_LOCK_ALL, FALSE);
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Scene##main_scene")) {
		if (ImGui::MenuItem("Properties##tools_props"))
			ExecCommand(COMMAND_SHOW_PROPERTIES);
		ImGui::Separator();
		if (ImGui::MenuItem("Options##scene_options"))
			ExecCommand(COMMAND_OPTIONS, CCommandVar("Scene"));
		ImGui::Separator();
		if (ImGui::MenuItem("Validate##scene_validate"))
			ExecCommand(COMMAND_VALIDATE_SCENE);
		ImGui::Separator();
		if (ImGui::MenuItem("Summary Info##scene_summary")) {
			ExecCommand(COMMAND_CLEAR_SCENE_SUMMARY);
			ExecCommand(COMMAND_COLLECT_SCENE_SUMMARY);
			ExecCommand(COMMAND_SHOW_SCENE_SUMMARY);
		}
		if (ImGui::MenuItem("Highlight Texture...##scene_highlight_tex"))
			ExecCommand(COMMAND_SCENE_HIGHLIGHT_TEXTURE);
		ImGui::Separator();
		if (ImGui::MenuItem("Clear Debug Draw##scene_clear_debug"))
			ExecCommand(COMMAND_CLEAR_DEBUG_DRAW);
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Compile##main_compile")) {
		if (ImGui::MenuItem("Build##compile_build"))
			ExecCommand(COMMAND_BUILD);
		ImGui::Separator();
		if (ImGui::MenuItem("Make game##compile_make_game"))
			ExecCommand(COMMAND_MAKE_GAME);
		if (ImGui::MenuItem("Make details##compile_make_details"))
			ExecCommand(COMMAND_MAKE_DETAILS);
		if (ImGui::MenuItem("Make HOM##compile_make_hom"))
			ExecCommand(COMMAND_MAKE_HOM);
		if (ImGui::MenuItem("Make Sound Occluder##compile_make_som"))
			ExecCommand(COMMAND_MAKE_SOM);
		if (ImGui::MenuItem("Make AI-map##compile_make_aimap"))
			ExecCommand(COMMAND_MAKE_AIMAP);
		ImGui::Separator();
		if (ImGui::MenuItem("Import error list##compile_import_errors"))
			ExecCommand(COMMAND_IMPORT_COMPILER_ERROR);
		if (ImGui::MenuItem("Export error list##compile_export_errors"))
			ExecCommand(COMMAND_EXPORT_COMPILER_ERROR);
		if (ImGui::MenuItem("Clear error list##compile_clear_errors"))
			ExecCommand(COMMAND_CLEAR_DEBUG_DRAW);
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Objects##main_objects")) {
		if (ImGui::MenuItem("Object Library Editor##objects_lib_editor"))
			ExecCommand(COMMAND_LIBRARY_EDITOR);
		ImGui::Separator();
		if (ImGui::MenuItem("Reload Objects##objects_reload"))
			ExecCommand(COMMAND_RELOAD_OBJECTS);
		if (ImGui::MenuItem("Clean Library##objects_clean_lib"))
			ExecCommand(COMMAND_CLEAN_LIBRARY);
		//if (ImGui::MenuItem("Clip Editor##objects_clip_editor"))
		//	ExecCommand(COMMAND_SHOW_CLIP_EDITOR);
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Images##main_images")) {
		if (ImGui::MenuItem("Image Editor##images_editor"))
			ExecCommand(COMMAND_IMAGE_EDITOR);
		ImGui::Separator();
		if (ImGui::MenuItem("Reload Textures##images_reload_tex"))
			ExecCommand(COMMAND_RELOAD_TEXTURES);
		ImGui::Separator();
		if (ImGui::MenuItem("Synchronize Textures##images_sync_tex"))
			ExecCommand(COMMAND_REFRESH_TEXTURES);
		if (ImGui::MenuItem("Check New Textures##images_check_new"))
			ExecCommand(COMMAND_CHECK_TEXTURES);
		ImGui::Separator();
		if (ImGui::MenuItem("Edit Minimap##images_edit_minimap"))
			ExecCommand(COMMAND_MINIMAP_EDITOR);
		if (ImGui::MenuItem("Sync THM##images_sync_thm"))
			;
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Sounds##main_sounds")) {
		if (ImGui::MenuItem("Sound Editor##sounds_editor"))
			ExecCommand(COMMAND_SOUND_EDITOR);
		ImGui::Separator();
		if (ImGui::MenuItem("Synchronize Sounds##sounds_sync"))
			ExecCommand(COMMAND_SYNC_SOUNDS);
		ImGui::Separator();
		if (ImGui::MenuItem("Refresh Environment Library##sounds_refresh_env_lib"))
			ExecCommand(COMMAND_REFRESH_SOUND_ENVS);
		if (ImGui::MenuItem("Refresh Environment Geometry##sounds_refresh_env_geom"))
			ExecCommand(COMMAND_REFRESH_SOUND_ENV_GEOMETRY);
		if (ImGui::MenuItem("Load Sound Occluder##sounds_load_occluder"))
			ExecCommand(COMMAND_LOAD_SOUND_OCCLUDER);
		ImGui::EndMenu();
	}

	if (ImGui::MenuItem("Light Animation Editor##main_lanim_editor"))
		ExecCommand(COMMAND_LANIM_EDITOR);
	if (ImGui::MenuItem("Object List##main_object_list"))
		ExecCommand(COMMAND_SHOW_OBJECTLIST);

	if (ImGui::BeginMenu("Help##main_help")) {
		if (ImGui::MenuItem("ImGui Demo##help_demo_window"))
			UI->show_demo_window = !UI->show_demo_window;
		ImGui::Separator();
		if (ImGui::MenuItem("About...##main_about")) {
			IM_About* about = xr_new<IM_About>();
			about->Open();
			UI->AddIMWindow(about);
		}
		ImGui::EndMenu();
	}
}

IM_LeftBar imLeftBar;
