#include "stdafx.h"
#pragma hdrstop

#include "IM_FrameShape.h"

#include <imgui.h>

#include "../ECore/Editor/ui_main.h"
#include "../Edit/ESceneShapeTools.h"
#include "../Edit/EShape.h"
#include "../Edit/UI_LevelTools.h"
#include "../Edit/scene.h"

#pragma package(smart_init)

void IM_FrameShape::OnAdd()
{
	m_parent_tool = dynamic_cast<ESceneShapeTool*>(Scene->GetTool(OBJCLASS_SHAPE));
	VERIFY(m_parent_tool);

    IM_Storage storage(false, "level.ini", "IM_FrameShape");

    if(storage.GetString("append_mode", "box") == "box")
    {
    	m_append_box = true;
        m_append_sphere = false;
    }
    else
    {
    	m_append_box = false;
        m_append_sphere = true;
    }
}

void IM_FrameShape::OnRemove()
{
	IM_Storage storage(true, "level.ini", "IM_FrameShape");
    storage.PutString("append_mode", m_append_box ? "box" : "sphere");
}

void IM_FrameShape::Render()
{
    if (ImGui::CollapsingHeader("Append", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::Selectable("Box##shape_append_box", &m_append_box))
            m_append_sphere = !m_append_box;
        if (ImGui::Selectable("Sphere##shape_append_sphere", &m_append_sphere))
            m_append_box = !m_append_sphere;
    }

    if (ImGui::CollapsingHeader("Edit", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::Selectable("Attach Shape...##shape_attach", &m_attach_shape))
        {
        	if(m_attach_shape)
            	ExecCommand(COMMAND_CHANGE_ACTION, etaAdd);
        }
        if (ImGui::MenuItem("Detach All##shape_detach_all"))
        {
            ObjectList lst;
    		if(Scene->GetQueryObjects(lst,OBJCLASS_SHAPE,1,1,0))
            {
				Scene->SelectObjects(false,OBJCLASS_SHAPE);
    			for(ObjectIt it=lst.begin(); it!=lst.end(); it++)
        			((CEditShape*)*it)->Detach();
    		}
    	}
    }

    if (ImGui::CollapsingHeader("Level bounds", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::Selectable("Edit level bounds##shape_edit_bounds", &m_edit_level_bound))
            m_parent_tool->OnEditLevelBounds(false);

        if (ImGui::MenuItem("Recalc##shape_recalc_bounds", NULL, false, m_edit_level_bound))
            m_parent_tool->OnEditLevelBounds(true);
    }
}
