#ifndef IM_ContextMenuH
#define IM_ContextMenuH

#include "../ECore/ImGui/IM_Window.h"
#include <imgui.h>

class IM_ContextMenu : public IM_Window {
public:
	bool m_open;
	bool m_detach;

	static constexpr float	TEAR_OFF_HEIGHT		= 10.0f;
	static constexpr float	LINE_THICKNESS		= 4.0f;
	static constexpr float	LINE_PADDING		= 6.0f;
	static constexpr ImVec2 INITIAL_WINDOW_SIZE = ImVec2(320.0f, 400.0f);

	IM_ContextMenu() : m_open(false), m_detach(false) {}

public:
	virtual void Render();

	void		 Open();
	void		 ShowMenu() const;
};

extern IM_ContextMenu g_imContextMenu;

#endif
