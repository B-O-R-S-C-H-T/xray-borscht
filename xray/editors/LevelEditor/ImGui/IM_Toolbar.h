#ifndef IM_ToolbarH
#define IM_ToolbarH

#include <imgui.h>

#include "../ECore/ImGui/IM_Window.h"

class IM_Toolbar : public IM_Window {
public:
	bool  m_vertical   = true;
	float m_icon_scale = 1.3f;

private:
	int			m_icon_size			= 0;
	float		m_font_global_scale = 0.0f;

	float		m_saved_width		= 900.0f;
	float		m_saved_height		= 400.0f;
	float		m_last_width		= 0.0f;
	float		m_last_height		= 0.0f;

	void		UpdateIconCache();
	ImVec2		ComputeDesiredSize(int icon_size) const;

public:
	bool		 SVGButton(const char* svg_name, const char* tooltip = nullptr, bool activated = true);
	void		 Separator();

	IC		void SetVertical	(bool v)		{ if (m_vertical != v) m_vertical = v; }
	IC		void SetIconScale	(float scale)	{ if (std::fabs(m_icon_scale - scale) > 0.001f) m_icon_scale = scale; }

	virtual void OnAdd() override;
	virtual void OnRemove() override;

	virtual void Render() override;
};

extern IM_Toolbar g_imToolbar;

#endif