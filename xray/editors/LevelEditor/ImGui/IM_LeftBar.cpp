#include "stdafx.h"
#pragma hdrstop

#include "IM_LeftBar.h"

#include <imgui.h>
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

void IM_LeftBar::OnAdd()
{
	m_title = u8"Редактор уровней X-Ray 1.8";

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

    m_enable_snap_list = s.GetBool("enable_snap_list");
    m_select_snap_objs_mode = s.GetBool("select_snap_objs_mode");
    m_detach_tool_frame = s.GetBool("detach_tool_frame");
    m_detach_main_menu = s.GetBool("detach_main_menu");

    m_show_scene = s.GetBool("show_scene_panel", true);
    m_show_tools = s.GetBool("show_tools_panel", true);
    m_show_editmode = s.GetBool("show_editmode_panel", true);
    m_show_snaplist = s.GetBool("show_snaplist_panel", true);
}

void IM_LeftBar::OnRemove()
{
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
    s.PutBool("detach_main_menu", m_detach_main_menu);

    s.PutBool("show_scene_panel", m_show_scene);
    s.PutBool("show_tools_panel", m_show_tools);
    s.PutBool("show_editmode_panel", m_show_editmode);
    s.PutBool("show_snaplist_panel", m_show_snaplist);
}

void IM_LeftBar::Render()
{
	ImGui::Begin("Level Editor", NULL);

	ImGui::TextUnformatted(&*m_title.begin(), &*m_title.end());
	ImGui::SameLine();
    if (ImGui::SmallButton("ImGui##leftbar_demo"))
    	UI->show_demo_window = true;

    if(!m_detach_main_menu)
    {
        bool need_detach = ImGui::Button("##detach_menu", ImVec2(-1, 5));

        if(ImGui::CollapsingPanel("Scene", &m_show_scene))
            RenderMainMenu();
        if(ImGui::CollapsingPanel("Tools", &m_show_tools))
            RenderToolsMenu();

        if(need_detach)
            m_detach_main_menu = true;
    }
    else
    {
        if(ImGui::BeginMainMenuBar())
        {
            bool need_attach = ImGui::MenuItem("|##attach_menu");

            RenderMainMenu();
            ImGui::EndMainMenuBar();

            if(need_attach)
                m_detach_main_menu = false;
        }
    }

    if(ImGui::CollapsingPanel("Edit mode", &m_show_editmode))
    {
    	ImGui::Columns(2);

		for(ObjClassID i = OBJCLASS_FIRST_CLASS; i < OBJCLASS_COUNT; i++)
        {
        	ESceneToolBase *tool = Scene->GetTool(i);

            ImGui::PushID(1005 + i);
            if (ImGui::Button(tool->IsVisible() ? "*##editmode_visible" : " ##editmode_invisible"))
            	ExecCommand(COMMAND_SHOW_TARGET, i, !tool->IsVisible());
            ImGui::PopID();

            ImGui::SameLine();

			ImGui::RadioButton(tool->ClassDesc(), LTools->CurrentClassID() == i);
            if(ImGui::IsItemClicked())
            	ExecCommand(COMMAND_CHANGE_TARGET, i);

            if(i == OBJCLASS_COUNT/2-1)
            	ImGui::NextColumn();
        }

        ImGui::Columns(1);
    }

    if(ImGui::CollapsingPanel("Snap list", &m_show_snaplist))
    {
        if (ImGui::BeginMenu("Commands##snap_commands_leftbar"))
        {
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
            if (ImGui::MenuItem("Clear list##snap_clear_list"))
            {
            	if (ELog.DlgMsg(mtConfirmation,TMsgDlgButtons() << mbYes << mbNo,"Are you sure to clear snap objects?")==mrYes)
					ExecCommand(COMMAND_CLEAR_SNAP_OBJECTS);
            }

        	ImGui::EndMenu();
        }

        ImGui::Selectable("Enable/Show snap list##snap_enable", &m_enable_snap_list);

        ImGui::Columns(3, NULL, false);

        if (ImGui::Selectable("+/- Mode##snap_mode", &m_select_snap_objs_mode))
        	if (m_select_snap_objs_mode&&(etaSelect!=Tools->GetAction())){
    			ELog.Msg(mtError,"Before modify snap list activate select mode!");
        		m_select_snap_objs_mode = false;
    		}

        ImGui::NextColumn();

        if (ImGui::MenuItem("Clear##snap_clear_leftbar"))
        	ExecCommand(COMMAND_CLEAR_SNAP_OBJECTS);

        ImGui::NextColumn();

        if (ImGui::MenuItem("Edit...##snap_edit"))
        	ExecCommand(COMMAND_EDIT_SNAP_OBJECTS);

        ImGui::Columns(1);

    	ObjectList *ol = Scene->GetSnapList(true);
        int i = 0;

        ImGui::BeginChild("snap list", ImVec2(0, 150), true);

        for (ObjectIt it = ol->begin(), end = ol->end(); it != end; it++)
        {
            ImGui::PushID(i);
            ImGui::Text("%u: %s", i, (*it)->FName.c_str());
            ImGui::PopID();
            i++;
        }

        ImGui::EndChild();
    }

    if(m_detach_tool_frame)
    {
    	ImGui::Begin("Tool Options", &m_detach_tool_frame);
        RenderToolFrame();
        ImGui::End();
    }
    else
    {
    	bool detach = ImGui::Button("##detach_tool_opts", ImVec2(-1, 5));
		RenderToolFrame();
        m_detach_tool_frame = detach;
    }

    ImGui::End();
}

void IM_LeftBar::RenderToolFrame()
{
    switch (LTools->CurrentClassID())
    {
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

void IM_LeftBar::RenderMainMenu()
{
	if (ImGui::BeginMenu("File##main_file"))
    {
        if (ImGui::MenuItem("Clear##file_clear"))
        	ExecCommand(COMMAND_CLEAR);
        if (ImGui::MenuItem("Open##file_open"))
        	ExecCommand(COMMAND_LOAD);
        if (ImGui::MenuItem("Save##file_save"))
        	ExecCommand(COMMAND_SAVE);
        if (ImGui::MenuItem("Save as##file_saveas"))
        	ExecCommand(COMMAND_SAVE, 0, 1);
        ImGui::Separator();
        if (ImGui::MenuItem("Open selection##file_open_sel"))
        	ExecCommand(COMMAND_LOAD_SELECTION);
        if (ImGui::MenuItem("Save selection as##file_save_sel"))
        	ExecCommand(COMMAND_SAVE_SELECTION);
        ImGui::Separator();
#ifndef NO_VCL
        if (ImGui::MenuItem("Make pack##file_make_pack"))
        	TDB_packer::ActivatePacker();
#endif
        ImGui::Separator();
        if (m_detach_main_menu || ImGui::BeginMenu("Open recent##file_recent", !!EPrefs->scene_recent_count))
        {
        	for(AStringIt it = EPrefs->scene_recent_list.begin(); it != EPrefs->scene_recent_list.end(); it++)
            {
            	if(ImGui::MenuItem(it->c_str()))
                	ExecCommand(COMMAND_LOAD, xr_string(it->c_str()));
            }

            if(!m_detach_main_menu)
        	    ImGui::EndMenu();
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Quit##file_quit"))
        	ExecCommand(COMMAND_QUIT);

        ImGui::EndMenu();
    }

    if(m_detach_main_menu)
    {
        if (ImGui::BeginMenu("Tools##main_tools"))
        {
            RenderToolsMenu();
            ImGui::EndMenu();
        }
    }

    if (ImGui::BeginMenu("Scene##main_scene"))
    {
        if (ImGui::MenuItem("Options##scene_options"))
        	ExecCommand(COMMAND_OPTIONS, CCommandVar("Scene"));
        ImGui::Separator();
        if (ImGui::MenuItem("Validate##scene_validate"))
        	ExecCommand(COMMAND_VALIDATE_SCENE);
        ImGui::Separator();
        if (ImGui::MenuItem("Summary Info##scene_summary"))
        {
        	ExecCommand(COMMAND_CLEAR_SCENE_SUMMARY);
            ExecCommand(COMMAND_COLLECT_SCENE_SUMMARY);
            ExecCommand(COMMAND_SHOW_SCENE_SUMMARY);
        }
        if (ImGui::MenuItem("Highlight Texture...##scene_highlight_tex"))
        	ExecCommand(COMMAND_SCENE_HIGHLIGHT_TEXTURE);
        ImGui::Separator();
        if (ImGui::MenuItem("Clear Debug Draw##scene_clear_debug"))
        	ExecCommand(COMMAND_CLEAR_DEBUG_DRAW);
        ImGui::Separator();
        if (ImGui::MenuItem("Export entire Scene as OBJ##scene_export_all"))
        	Scene->ExportObj(false);
        if (ImGui::MenuItem("Export selection as OBJ##scene_export_sel"))
        	Scene->ExportObj(true);

    	ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Compile##main_compile"))
    {
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

    if (ImGui::BeginMenu("Objects##main_objects"))
	{
        if (ImGui::MenuItem("Library editor##objects_lib_editor"))
        	ExecCommand(COMMAND_LIBRARY_EDITOR);
        ImGui::Separator();
        if (ImGui::MenuItem("Reload objects##objects_reload"))
        	ExecCommand(COMMAND_RELOAD_OBJECTS);
        if (ImGui::MenuItem("Clean library##objects_clean_lib"))
        	ExecCommand(COMMAND_CLEAN_LIBRARY);
        if (ImGui::MenuItem("Clip editor##objects_clip_editor"))
        	ExecCommand(COMMAND_SHOW_CLIP_EDITOR);

    	ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Images##main_images"))
    {
        if (ImGui::MenuItem("Image editor##images_editor"))
        	ExecCommand(COMMAND_IMAGE_EDITOR);
        ImGui::Separator();
        if (ImGui::MenuItem("Reload textures##images_reload_tex"))
        	ExecCommand(COMMAND_RELOAD_TEXTURES);
        ImGui::Separator();
        if (ImGui::MenuItem("Synchronize textures##images_sync_tex"))
        	ExecCommand(COMMAND_REFRESH_TEXTURES);
        if (ImGui::MenuItem("Check new textures##images_check_new"))
        	ExecCommand(COMMAND_CHECK_TEXTURES);
        ImGui::Separator();
        if (ImGui::MenuItem("Edit minimap##images_edit_minimap"))
        	ExecCommand(COMMAND_MINIMAP_EDITOR);
        if (ImGui::MenuItem("SyncTHM##images_sync_thm"))
			; // TODO

    	ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Sounds##main_sounds"))
    {
        if (ImGui::MenuItem("Sound editor##sounds_editor"))
        	ExecCommand(COMMAND_SOUND_EDITOR);
        ImGui::Separator();
        if (ImGui::MenuItem("Synchronize sounds##sounds_sync"))
        	ExecCommand(COMMAND_SYNC_SOUNDS);
        ImGui::Separator();
        if (ImGui::MenuItem("Refresh environment library##sounds_refresh_env_lib"))
        	ExecCommand(COMMAND_REFRESH_SOUND_ENVS);
        if (ImGui::MenuItem("Refresh environment geometry##sounds_refresh_env_geom"))
        	ExecCommand(COMMAND_REFRESH_SOUND_ENV_GEOMETRY);
        if (ImGui::MenuItem("Load sound occluder##sounds_load_occluder"))
        	ExecCommand(COMMAND_LOAD_SOUND_OCCLUDER);
    	ImGui::EndMenu();
    }

    if (ImGui::MenuItem("Light anim editor##main_lanim_editor"))
    	ExecCommand(COMMAND_LANIM_EDITOR);

    if (ImGui::MenuItem("Object list##main_object_list"))
    	ExecCommand(COMMAND_SHOW_OBJECTLIST);

    if (ImGui::MenuItem("Preferences##main_prefs"))
    	ExecCommand(COMMAND_EDITOR_PREF);

#ifndef NO_VCL
    if (ImGui::MenuItem("About##main_about"))
    	AboutForm->ShowModal();
#endif
}

void IM_LeftBar::RenderToolsMenu()
{
    if(!m_detach_main_menu)
        ImGui::Columns(2);

    if (ImGui::BeginMenu("Edit##tools_edit"))
    {
        if (ImGui::MenuItem("Cut##edit_cut"))
        	ExecCommand(COMMAND_CUT);
        if (ImGui::MenuItem("Copy##edit_copy"))
        	ExecCommand(COMMAND_COPY);
        if (ImGui::MenuItem("Paste##edit_paste"))
        	ExecCommand(COMMAND_PASTE);
        ImGui::Separator();
        if (ImGui::MenuItem("Undo##edit_undo"))
        	ExecCommand(COMMAND_UNDO);
        if (ImGui::MenuItem("Redo##edit_redo"))
        	ExecCommand(COMMAND_REDO);
    	ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Selection##tools_selection"))
    {
        if (ImGui::MenuItem("Invert##sel_invert"))
        	ExecCommand(COMMAND_INVERT_SELECTION_ALL);
        if (ImGui::MenuItem("Select all##sel_all"))
        	ExecCommand(COMMAND_SELECT_ALL);
        if (ImGui::MenuItem("Unselect all##sel_unselect"))
        	ExecCommand(COMMAND_DESELECT_ALL);
    	ImGui::EndMenu();
    }

    if(!m_detach_main_menu)
    {
        if (ImGui::MenuItem("Properties##tools_props"))
    	    ExecCommand(COMMAND_SHOW_PROPERTIES);
    }

    if(!m_detach_main_menu)
        ImGui::NextColumn();

    if (ImGui::BeginMenu("Visibility##tools_visibility"))
    {
        if (ImGui::MenuItem("Hide selection##vis_hide_sel"))
        	ExecCommand(COMMAND_HIDE_SEL, FALSE);
        if (ImGui::MenuItem("Hide unselected##vis_hide_unsel"))
        	ExecCommand(COMMAND_HIDE_UNSEL);
        if (ImGui::MenuItem("Hide all##vis_hide_all"))
        	ExecCommand(COMMAND_HIDE_ALL, FALSE);
        ImGui::Separator();
        if (ImGui::MenuItem("Unhide all##vis_unhide_all"))
        	ExecCommand(COMMAND_HIDE_ALL, TRUE);
    	ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Locking##tools_locking"))
    {
        if (ImGui::MenuItem("Lock selection##lock_sel"))
        	ExecCommand(COMMAND_LOCK_SEL, TRUE);
        if (ImGui::MenuItem("Lock unselected##lock_unsel"))
        	ExecCommand(COMMAND_LOCK_UNSEL, TRUE);
        if (ImGui::MenuItem("Lock all##lock_all"))
        	ExecCommand(COMMAND_LOCK_ALL, TRUE);
        ImGui::Separator();
        if (ImGui::MenuItem("Unlock selection##unlock_sel"))
        	ExecCommand(COMMAND_LOCK_SEL, FALSE);
        if (ImGui::MenuItem("Unlock unselected##unlock_unsel"))
        	ExecCommand(COMMAND_LOCK_UNSEL, FALSE);
        if (ImGui::MenuItem("Unlock all##unlock_all"))
        	ExecCommand(COMMAND_LOCK_ALL, FALSE);
    	ImGui::EndMenu();
    }

    if (ImGui::MenuItem("Multi rename##tools_multi_rename"))
    	ExecCommand(COMMAND_MULTI_RENAME_OBJECTS);

    if(m_detach_main_menu)
    {
        if (ImGui::MenuItem("Properties##tools_props_detach"))
    	    ExecCommand(COMMAND_SHOW_PROPERTIES);
    }

    if(!m_detach_main_menu)
        ImGui::Columns(1);
}

IM_LeftBar imLeftBar;
