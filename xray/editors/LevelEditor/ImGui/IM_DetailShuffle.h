#pragma once

#include <imgui.h>

#include "../ECore/Editor/EThumbnail.h"
#include "../ECore/ImGui/IM_Canvas.h"
#include "../ECore/ImGui/IM_ChooseForm.h"
#include "../ECore/ImGui/IM_Window.h"
#include "../Edit/ESceneDOTools.h"
#include "editors/ECore/ImGui/IM_PropertyTree.h"

struct ColorGroup {
	u32					color;
	xr_vector<EDetail*> items;
	ColorGroup() : color(0) {}
};

class IM_DetailShuffle : public IM_Window {
private:
	EDetailManager*			 m_dm = nullptr;
	xr_vector<ColorGroup>	 m_color_groups;
	xr_vector<EDetail*>		 m_main_items;
	bool					 m_bColorIndModif  = false;
	bool					 m_bObjectModif	   = false;
	EDetail*				 m_selected_detail = nullptr;
	EImageThumbnail*		 m_thm			   = nullptr;
	EDetail*				 m_drag_item	   = nullptr;
	int						 m_drag_from_group = -1;
	IM_PropertyTree			 m_object_props;
	xr_string				 m_filter_text		   = "";
	float					 m_objects_list_height = 320.0f;
	IM_Canvas				 m_thumbnail_canvas{512, 512};
	bool					 m_open = true;

	static IM_DetailShuffle* s_instance;

	void					 RenderObjectPropertiesPanel();
	void					 RenderObjectsListPanel();
	void					 OnAddObjects(IM_ChooseForm* form);
	void					 RemoveFromMainListOnly(EDetail* dd);
	void					 FillData();
	bool					 ApplyChanges();
	void					 ClearIndexForms();
	void					 OnItemFocused(EDetail* dd);
	void					 OnPropsModified();
	void					 RenderColorGroups();
	u32						 GenerateUniqueColor();

public:
	explicit IM_DetailShuffle(EDetailManager* dm);
	~IM_DetailShuffle();

	virtual void Render() override;

	static bool	 Run(EDetailManager* dm);
};
