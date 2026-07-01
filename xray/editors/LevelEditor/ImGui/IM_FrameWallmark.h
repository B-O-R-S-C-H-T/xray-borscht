#ifndef IM_FrameWallmarkH
#define IM_FrameWallmarkH

#include "../ECore/ImGui/IM_Window.h"
#include "editors/ECore/ImGui/IM_PropertyTree.h"

class ESceneWallmarkTool;

class IM_FrameWallmark : public IM_Window {
public:
	ESceneWallmarkTool* parent_tool = nullptr;

private:
	IM_PropertyTree m_wallmark_props;
	bool			m_props_initialized = false;

public:
	IM_FrameWallmark() : parent_tool(nullptr) {}

	void		 RefreshProperties() { m_props_initialized = false; }

	virtual void OnAdd() override;
	virtual void OnRemove() override;

	virtual void Render() override;
};

#endif
