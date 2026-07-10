#include "stdafx.h"
#pragma hdrstop

#include "IM_Toolbar.h"

#include "../ECore/Editor/device.h"
#include "../Edit/UI_LevelMain.h"
#include "../Edit/UI_LevelTools.h"
#include "editors/ECore/ImGui/IM_SVGCanvas.h"

bool IM_Toolbar::SVGButton(const char* svg_name, const char* tooltip, bool activated) {
	if (!svg_name)
		return false;

	const int icon_size = m_icon_size;

	char	  fullpath[512];
	sprintf(fullpath, "icons\\%s", svg_name);

	IM_SVGCanvas* icon = IM_SVGCanvas::GetIcon(fullpath, icon_size);
	if (!icon) {
		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "X");
		if (!m_vertical)
			ImGui::SameLine();
		return false;
	}

	char id[256];
	sprintf(id, "##svg_%s", svg_name);

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.4f, 0.4f, 0.3f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.5f, 0.5f, 0.4f));

	ImVec4 tint = activated ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : ImVec4(0.60f, 0.60f, 0.60f, 1.0f);

	bool   clicked =
		ImGui::ImageButton(id, (void*)icon->GetTexture(), ImVec2((float)icon_size, (float)icon_size), ImVec2(0, 0), ImVec2(1, 1), ImVec4(0, 0, 0, 0), tint);

	ImGui::PopStyleColor(3);

	if (tooltip && ImGui::IsItemHovered())
		ImGui::SetTooltip("%s", tooltip);

	if (!m_vertical)
		ImGui::SameLine();

	return clicked;
}

void IM_Toolbar::Separator() {
	if (m_vertical) {
		ImGui::Separator();
	} else {
		ImGui::SameLine(0, 5);
	}
}

void IM_Toolbar::UpdateIconCache() {
	float current_dpi = ImGui::GetIO().FontGlobalScale;
	float base_size	  = ImGui::GetFrameHeight();
	int	  icon_size	  = static_cast<int>(base_size * current_dpi * m_icon_scale + 0.5f);

	if (icon_size != m_icon_size || current_dpi != m_font_global_scale) {
		m_icon_size			= icon_size;
		m_font_global_scale = current_dpi;
		IM_SVGCanvas::RequestResize(icon_size);
	}

	IM_SVGCanvas::ProcessPendingRebuild();
}

ImVec2 IM_Toolbar::ComputeDesiredSize(int icon_size) const {
	if (m_vertical) {
		float fixed_w = static_cast<float>(icon_size) + ImGui::GetStyle().WindowPadding.x * 2.0f;
		return ImVec2(fixed_w, 400.0f);
	} else {
		float fixed_h = static_cast<float>(icon_size) + ImGui::GetStyle().WindowPadding.y * 2.0f;
		return ImVec2(900.0f, fixed_h);
	}
}

void IM_Toolbar::OnAdd() {
	IM_Storage& storage = EPrefs->GetSectionStorage("im_toolbar");

	m_vertical			= storage.GetBool("vertical", true);
	m_icon_scale		= storage.GetFloat("icon_scale", 1.3f);

	m_saved_width		= storage.GetFloat("width", 900.0f);
	m_saved_height		= storage.GetFloat("height", 400.0f);

	m_last_width		= m_saved_width;
	m_last_height		= m_saved_height;

	m_icon_size			= 0;
	m_font_global_scale = 0.0f;
}

void IM_Toolbar::OnRemove() {
	IM_Storage& storage = EPrefs->GetSectionStorage("im_toolbar");

	storage.PutBool("vertical", m_vertical);
	storage.PutFloat("icon_scale", m_icon_scale);
	storage.PutFloat("width", m_saved_width);
	storage.PutFloat("height", m_saved_height);

	storage.Save();
}

void IM_Toolbar::Render() {
	if (EPrefs) {
		m_vertical	 = EPrefs->toolbar_vertical;
		m_icon_scale = EPrefs->toolbar_icon_scale;
	}

	UpdateIconCache();
	const int		  icon_size = m_icon_size;
	const ImGuiStyle& style		= ImGui::GetStyle();

	ImVec2			  min_sz, max_sz;
	if (m_vertical) {
		const float fixed_w = static_cast<float>(icon_size) + style.WindowPadding.x * 2.0f;
		min_sz				= ImVec2(fixed_w, static_cast<float>(icon_size) * 3.0f + style.WindowPadding.y * 2.0f);
		max_sz				= ImVec2(fixed_w, FLT_MAX);
	} else {
		const float fixed_h = static_cast<float>(icon_size) + style.WindowPadding.y * 2.0f;
		const float min_w	= static_cast<float>(icon_size) * 8.0f + style.WindowPadding.x * 2.0f + style.ItemSpacing.x * 12.0f;
		min_sz				= ImVec2(min_w, fixed_h);
		max_sz				= ImVec2(FLT_MAX, fixed_h);
	}

	ImVec2 desired_size = (m_vertical ? ImVec2(min_sz.x, m_saved_height) : ImVec2(m_saved_width, min_sz.y));
	if (desired_size.x < min_sz.x)
		desired_size.x = min_sz.x;
	if (desired_size.y < min_sz.y)
		desired_size.y = min_sz.y;

	// imgui не умеет отключать часть рёбер панели, поэтому просто прячем ненужные левое и верхнее
	// рёбра за пределы экрана и под строку меню соответственно.
	const float edge_hide = 4.0f;
	ImVec2		pos(-edge_hide, ImGui::GetFrameHeight() - edge_hide);

	ImGui::SetNextWindowPos(pos, ImGuiCond_Always);

	// приходится применять размер каждый кадр, чтобы не минимизировалась панель при наложении иного окна поверх
	ImGui::SetNextWindowSize(desired_size, ImGuiCond_Always);
	ImGui::SetNextWindowSizeConstraints(min_sz, max_sz);

	constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse |
									   ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
									   ImGuiWindowFlags_NoFocusOnAppearing;

	if (ImGui::Begin("Top bar", nullptr, flags)) {
		ImVec2 curr_size = ImGui::GetWindowSize();

		if (m_vertical) {
			if (std::fabs(curr_size.y - m_last_height) > 0.5f) {
				m_saved_height = curr_size.y;
				m_last_height  = curr_size.y;
			}
		} else {
			if (std::fabs(curr_size.x - m_last_width) > 0.5f) {
				m_saved_width = curr_size.x;
				m_last_width  = curr_size.x;
			}
		}

		ImVec2 snap_size  = curr_size;
		bool   needs_snap = false;
		if (m_vertical) {
			if (curr_size.x < min_sz.x) {
				snap_size.x = min_sz.x;
				needs_snap	= true;
			}
		} else {
			if (curr_size.y < min_sz.y) {
				snap_size.y = min_sz.y;
				needs_snap	= true;
			}
		}
		if (needs_snap) {
			ImGui::SetWindowSize(snap_size, ImGuiCond_Always);
			curr_size = snap_size;
		}
		/*
		bool		currently_dragging_edge = false;
		ImVec2		mouse_pos				= ImGui::GetMousePos();
		ImVec2		win_pos					= ImGui::GetWindowPos();
		const float edge_threshold			= 8.0f;

		if (!m_vertical) // горизонтальный - только нижнее ребро
		{
			if (ImGui::IsMouseDragging(0) && std::fabs(mouse_pos.y - (win_pos.y + curr_size.y)) < edge_threshold)
				currently_dragging_edge = true;
		} else // вертикальный - только правое ребро
		{
			if (ImGui::IsMouseDragging(0) && std::fabs(mouse_pos.x - (win_pos.x + curr_size.x)) < edge_threshold)
				currently_dragging_edge = true;
		}

		if (currently_dragging_edge && !m_is_dragging_edge) {
			m_vertical = !m_vertical;
		}
		m_is_dragging_edge = currently_dragging_edge;
		*/
		// undo/redo кнопки отключены ранее, следует пользоваться горячими клавишами
		//if (SVGButton("undo.svg", "Undo"))	ExecCommand(COMMAND_UNDO);
		//if (SVGButton("redo.svg", "Redo"))	ExecCommand(COMMAND_REDO);

		if (SVGButton("select.svg", "Select", Tools->GetAction() == etaSelect))		Tools->SetAction(etaSelect);
		if (SVGButton("add.svg", "Add", Tools->GetAction() == etaAdd))				Tools->SetAction(etaAdd);
		if (SVGButton("move.svg", "Move", Tools->GetAction() == etaMove))			Tools->SetAction(etaMove);
		if (SVGButton("rotate.svg", "Rotate", Tools->GetAction() == etaRotate))		Tools->SetAction(etaRotate);
		if (SVGButton("scale.svg", "Scale", Tools->GetAction() == etaScale))		Tools->SetAction(etaScale);

		Separator();

		// axis buttons отключены, т.к. в новом редакторе используется трёхмерный манипулятор imguizmo
		//if (SVGButton("axis_x.svg", "Restrict to X", Tools->GetAxis() == etAxisX))		Tools->SetAxis(etAxisX);
		//if (SVGButton("axis_y.svg", "Restrict to Y", Tools->GetAxis() == etAxisY))		Tools->SetAxis(etAxisY);
		//if (SVGButton("axis_z.svg", "Restrict to Z", Tools->GetAxis() == etAxisZ))		Tools->SetAxis(etAxisZ);
		//if (SVGButton("axis_zx.svg", "Restrict to ZX", Tools->GetAxis() == etAxisZX))	Tools->SetAxis(etAxisZX);

		if (SVGButton("rotate_local.svg", "Parent CS toggle", Tools->GetSettings(etfCSParent)))			ExecCommand(COMMAND_SET_SETTINGS, etfCSParent, !Tools->GetSettings(etfCSParent));
		if (SVGButton("scale_non_uniform.svg", "Non-uniform scale", Tools->GetSettings(etfNUScale)))	ExecCommand(COMMAND_SET_SETTINGS, etfNUScale, !Tools->GetSettings(etfNUScale));

		Separator();

		// snap buttons
		if (SVGButton("snap_to_grid.svg", "Snap to grid", Tools->GetSettings(etfGSnap)))			ExecCommand(COMMAND_SET_SETTINGS, etfGSnap, !Tools->GetSettings(etfGSnap));
		if (SVGButton("snap_to_object.svg", "Snap to objects", Tools->GetSettings(etfOSnap)))		ExecCommand(COMMAND_SET_SETTINGS, etfOSnap, !Tools->GetSettings(etfOSnap));
		if (SVGButton("snap_moving.svg", "Moving snap", Tools->GetSettings(etfMTSnap)))				ExecCommand(COMMAND_SET_SETTINGS, etfMTSnap, !Tools->GetSettings(etfMTSnap));
		if (SVGButton("normal_align.svg", "Normal alignment", Tools->GetSettings(etfNormalAlign)))	ExecCommand(COMMAND_SET_SETTINGS, etfNormalAlign, !Tools->GetSettings(etfNormalAlign));
		if (SVGButton("snap_to_vertex.svg", "Snap to vertex", Tools->GetSettings(etfVSnap)))		ExecCommand(COMMAND_SET_SETTINGS, etfVSnap, !Tools->GetSettings(etfVSnap));
		if (SVGButton("snap_rotate.svg", "Angle snap", Tools->GetSettings(etfASnap)))				ExecCommand(COMMAND_SET_SETTINGS, etfASnap, !Tools->GetSettings(etfASnap));
		if (SVGButton("snap_on_move.svg", "Moving snap", Tools->GetSettings(etfMSnap)))				ExecCommand(COMMAND_SET_SETTINGS, etfMSnap, !Tools->GetSettings(etfMSnap));

		Separator();

		// focus buttons
		if (SVGButton("focus_scene.svg", "Focus on whole scene"))	ExecCommand(COMMAND_ZOOM_EXTENTS, FALSE);
		if (SVGButton("focus_selected.svg", "Focus on selected"))	ExecCommand(COMMAND_ZOOM_EXTENTS, TRUE);

		Separator();

		// view buttons
		if (SVGButton("view_front.svg", "Front view"))		Device.m_Camera.ViewFront();
		if (SVGButton("view_back.svg", "Back view"))		Device.m_Camera.ViewBack();
		if (SVGButton("view_left.svg", "Left view"))		Device.m_Camera.ViewLeft();
		if (SVGButton("view_right.svg", "Right view"))		Device.m_Camera.ViewRight();
		if (SVGButton("view_top.svg", "Top view"))			Device.m_Camera.ViewTop();
		if (SVGButton("view_bottom.svg", "Bottom view"))	Device.m_Camera.ViewBottom();
		if (SVGButton("view_reset.svg", "Reset view"))		Device.m_Camera.ViewReset();

		Separator();

		// camera mode buttons
		if (SVGButton("camera_plane.svg", "PlaneMove", Device.m_Camera.GetStyle() == csPlaneMove))		Device.m_Camera.SetStyle(csPlaneMove);
		if (SVGButton("camera_arcball.svg", "3DArcBall", Device.m_Camera.GetStyle() == cs3DArcBall))	Device.m_Camera.SetStyle(cs3DArcBall);
		if (SVGButton("camera_freefly.svg", "FreeFly", Device.m_Camera.GetStyle() == csFreeFly))		Device.m_Camera.SetStyle(csFreeFly);

	}
	ImGui::End();
}

IM_Toolbar g_imToolbar;
