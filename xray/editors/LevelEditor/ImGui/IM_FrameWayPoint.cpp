#include "stdafx.h"
#pragma hdrstop

#include "IM_FrameWayPoint.h"

#include <imgui.h>

#include "../ECore/Editor/ui_main.h"
#include "../Edit/ESceneWayControls.h"
#include "../Edit/UI_LevelTools.h"
#include "../Edit/WayPoint.h"
#include "../Edit/scene.h"

#pragma package(smart_init)

void IM_FrameWayPoint::Add1Links()
{
	if (!m_point_mode){
    	ELog.DlgMsg(mtInformation,"Before editing enter Point Mode.");
    	return;
    }
    bool bRes = false;
	ObjectList lst;
    Scene->GetQueryObjects(lst, OBJCLASS_WAY, 1, 1, 0);
	// remove links
    for (ObjectIt it=lst.begin(); it!=lst.end(); it++){
    	((CWayObject*)(*it))->RemoveLink();
    	bRes|=((CWayObject*)(*it))->Add1Link();
    }
	if (bRes) Scene->UndoSave();
    ExecCommand(COMMAND_UPDATE_PROPERTIES);
}

void IM_FrameWayPoint::Add2Links()
{
	if (!m_point_mode){
    	ELog.DlgMsg(mtInformation,"Before editing enter Point Mode.");
    	return;
    }
    bool bRes = false;
	ObjectList lst;
    Scene->GetQueryObjects(lst, OBJCLASS_WAY, 1, 1, 0);
    for (ObjectIt it=lst.begin(); it!=lst.end(); it++)
    	bRes|=((CWayObject*)(*it))->Add2Link();
	if (bRes) Scene->UndoSave();
    ExecCommand(COMMAND_UPDATE_PROPERTIES);
}

void IM_FrameWayPoint::InvertLinks()
{
	if (!m_point_mode){
    	ELog.DlgMsg(mtInformation,"Before editing enter Point Mode.");
    	return;
    }
	ObjectList lst;
    int cnt = Scene->GetQueryObjects(lst, OBJCLASS_WAY, 1, 1, 0);
    for (ObjectIt it=lst.begin(); it!=lst.end(); it++)
    	((CWayObject*)(*it))->InvertLink();
	if (cnt) Scene->UndoSave();
    ExecCommand(COMMAND_UPDATE_PROPERTIES);
}

void IM_FrameWayPoint::ConvertTo1Links()
{
	ObjectList lst;
    int cnt = Scene->GetQueryObjects(lst, OBJCLASS_WAY, 1, 1, 0);
    for (ObjectIt it=lst.begin(); it!=lst.end(); it++)
    	((CWayObject*)(*it))->Convert1Link();
	if (cnt) Scene->UndoSave();
    ExecCommand(COMMAND_UPDATE_PROPERTIES);
}

void IM_FrameWayPoint::ConvertTo2Links()
{
	ObjectList lst;
    int cnt = Scene->GetQueryObjects(lst, OBJCLASS_WAY, 1, 1, 0);
    for (ObjectIt it=lst.begin(); it!=lst.end(); it++)
    	((CWayObject*)(*it))->Convert2Link();
	if (cnt) Scene->UndoSave();
    ExecCommand(COMMAND_UPDATE_PROPERTIES);
}

void IM_FrameWayPoint::RemoveLinks()
{
	if (!m_point_mode){
    	ELog.DlgMsg(mtInformation,"Before editing enter Point Mode.");
    	return;
    }
	ObjectList lst;
    int cnt = Scene->GetQueryObjects(lst, OBJCLASS_WAY, 1, 1, 0);
    for (ObjectIt it=lst.begin(); it!=lst.end(); it++)
    	((CWayObject*)(*it))->RemoveLink();
	if (cnt) Scene->UndoSave();
    ExecCommand(COMMAND_UPDATE_PROPERTIES);
}

void IM_FrameWayPoint::OnAdd()
{
	IM_Storage storage(false, "level.ini", "IM_FrameWayPoint");
    m_auto_link = storage.GetBool("auto_link", true);
}

void IM_FrameWayPoint::OnRemove()
{
	IM_Storage storage(true, "level.ini", "IM_FrameWayPoint");
    storage.PutBool("auto_link", m_auto_link);
}

void IM_FrameWayPoint::Render()
{
    if (ImGui::CollapsingHeader("Way Mode", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::MenuItem("Way Mode##way_mode_way", NULL, m_way_mode))
        {
        	m_way_mode = true;
            m_point_mode = false;
            LTools->SetTarget(OBJCLASS_WAY, estWayModeWay);
        }
        if (ImGui::MenuItem("Point Mode##way_mode_point", NULL, m_point_mode))
        {
        	m_point_mode = true;
            m_way_mode = false;
            LTools->SetTarget(OBJCLASS_WAY, estWayModePoint);
        }
    }

    if (ImGui::CollapsingHeader("Link Command", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::MenuItem("Auto Link##way_auto_link", NULL, m_auto_link))
            m_auto_link = !m_auto_link;

        ImGui::Separator();

        ImGui::Columns(2, "way_link_commands_cols", false);

        if (ImGui::MenuItem("Create 1-Link##way_create_1link"))
            Add1Links();
        if (ImGui::MenuItem("Create 2-Link##way_create_2link"))
            Add2Links();
        if (ImGui::MenuItem("Invert Link##way_invert_link"))
            InvertLinks();

        ImGui::NextColumn();

        if (ImGui::MenuItem("Convert to 1-Link##way_convert_1link"))
            ConvertTo1Links();
        if (ImGui::MenuItem("Convert to 2-Link##way_convert_2link"))
            ConvertTo2Links();
        if (ImGui::MenuItem("Remove Link##way_remove_link"))
            RemoveLinks();

        ImGui::Columns(1);
    }
}
