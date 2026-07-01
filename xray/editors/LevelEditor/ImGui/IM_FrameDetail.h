#ifndef IM_FrameDetailH
#define IM_FrameDetailH

#include "../ECore/ImGui/IM_Window.h"
#include "editors/ECore/ImGui/IM_PropertyTree.h"

class EDetailManager;

class IM_FrameDetail : public IM_Window {
public:
	EDetailManager* parent_tool = nullptr;

private:
	IM_PropertyTree m_detail_props;
	bool			m_props_initialized = false;

public:
	IM_FrameDetail() : parent_tool(nullptr) {}

	void		 RefreshProperties() { m_props_initialized = false; }

	virtual void OnAdd() override;
	virtual void OnRemove() override;

	virtual void Render() override;
};

#endif
