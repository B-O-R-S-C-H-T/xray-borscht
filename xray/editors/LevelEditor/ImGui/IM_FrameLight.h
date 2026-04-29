#ifndef IM_FrameLightH
#define IM_FrameLightH

#include "..\ECore\ImGui\IM_Window.h"
#include "..\ECore\ImGui\IM_PropertyTree.h"

class ESceneLightTool;

class IM_FrameLight : public IM_Window {
public:
	bool m_show_useind3d	   = true;
	bool m_show_scene_lighting = true;

private:
	IM_PropertyTree m_sceneProps;
	PropItemVec		m_sceneToolItems;

	void			UseInD3D(bool bAll, bool bFlag);
	void			UpdateSceneLightingProps();

public:
	IM_FrameLight();

	virtual void OnAdd();
	virtual void OnRemove();
	virtual void Render();
};

#endif
