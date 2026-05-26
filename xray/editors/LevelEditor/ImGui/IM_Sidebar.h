#pragma once

#include "..\ECore\ImGui\IM_Window.h"
#include "IM_FrameAIMap.h"
#include "IM_FrameDetail.h"
#include "IM_FrameFogVol.h"
#include "IM_FrameGroup.h"
#include "IM_FrameLight.h"
#include "IM_FrameObject.h"
#include "IM_FramePS.h"
#include "IM_FramePortal.h"
#include "IM_FrameSector.h"
#include "IM_FrameShape.h"
#include "IM_FrameSpawn.h"
#include "IM_FrameWayPoint.h"
#include "IM_FrameWallmark.h"

class IM_Sidebar : public IM_Window {
public:
	IM_FrameObject	 fraObject;
	IM_FrameLight	 fraLight;
	IM_FrameFogVol	 fraFogVol;
	IM_FrameDetail	 fraDetail;
	IM_FrameSector	 fraSector;
	IM_FramePortal	 fraPortal;
	IM_FrameShape	 fraShape;
	IM_FramePS		 fraPS;
	IM_FrameWayPoint fraWayPoint;
	IM_FrameWallmark fraWallmark;
	IM_FrameSpawn	 fraSpawn;
	IM_FrameGroup	 fraGroup;
	IM_FrameAIMap	 fraAIMap;

	bool			 m_enable_snap_list;
	bool			 m_select_snap_objs_mode;
	bool			 m_show_snaplist;
	bool			 m_detach_tool_frame;
	float			 m_sidebar_width;

public:
	IM_Sidebar() : m_enable_snap_list(false), m_select_snap_objs_mode(false), m_show_snaplist(true), m_detach_tool_frame(false), m_sidebar_width(350.0f) {}

	virtual void OnAdd();
	virtual void OnRemove();
	virtual void Render();
	void		 RenderToolFrame();
};

extern IM_Sidebar imSidebar;
