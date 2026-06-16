#include "stdafx.h"

#include "IM_DetailShuffle.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "../ECore/Editor/Library.h"
#include "../ECore/Editor/ui_main.h"
#include "editors/ECore/Editor/FSColorWrapper.h"

IM_DetailShuffle* IM_DetailShuffle::s_instance = nullptr;

IM_DetailShuffle::IM_DetailShuffle(EDetailManager* dm) : m_dm(dm), m_thm(nullptr), m_object_props('\\', false, true), m_open(true) {
	VERIFY(m_dm);
	s_instance = this;
	FillData();
	m_object_props.OnModifiedEvent.bind(this, &IM_DetailShuffle::OnPropsModified);
}

IM_DetailShuffle::~IM_DetailShuffle() {
	ClearIndexForms();
	xr_delete(m_thm);
	s_instance = nullptr;
}

void IM_DetailShuffle::OnAddObjects(IM_ChooseForm* form) {
	if (!form)
		return;

	SStringVec lst;
	_SequenceToList(lst, form->GetSelected().c_str());

	u32	new_count = 0;
	for (SStringVecIt s_it = lst.begin(); s_it != lst.end(); ++s_it) {
		auto it = std::find_if(m_main_items.begin(), m_main_items.end(), [&](EDetail* d) { return 0 == strcmp(d->GetName(), s_it->c_str()); });
		if (it == m_main_items.end())
			new_count++;
	}

	if (m_dm->objects.size() + new_count > dm_max_objects) {
		ELog.DlgMsg(mtInformation,
					"Attempt to add too many detail objects!\n"
					"The X-Ray engine supports only 64 detail objects in the scene.\n"
					"Adding has been aborted.");
		return;
	}

	for (SStringVecIt s_it = lst.begin(); s_it != lst.end(); ++s_it) {
		auto it = std::find_if(m_main_items.begin(), m_main_items.end(), [&](EDetail* d) { return 0 == strcmp(d->GetName(), s_it->c_str()); });
		if (it != m_main_items.end())
			continue;

		EDetail* D = m_dm->AppendDO(s_it->c_str());
		if (D)
			m_bObjectModif = true;
	}

	FillData();

	if (!m_main_items.empty() && !m_selected_detail)
		OnItemFocused(m_main_items.back());
}

void IM_DetailShuffle::RemoveFromMainListOnly(EDetail* dd) {
	if (!dd)
		return;

	for (auto& pair : m_dm->m_ColorIndices) {
		auto& vec = pair.second;
		auto  it  = std::find(vec.begin(), vec.end(), dd);
		if (it != vec.end()) {
			vec.erase(it);
		}
	}

	m_dm->RemoveDO(dd->GetName());

	m_bObjectModif	 = true;
	m_bColorIndModif = true;

	for (auto& grp : m_color_groups) {
		auto it = std::find(grp.items.begin(), grp.items.end(), dd);
		if (it != grp.items.end()) {
			grp.items.erase(it);
		}
	}

	FillData();

	if (m_selected_detail == dd) {
		m_selected_detail = nullptr;
		xr_delete(m_thm);
		m_thm = nullptr;
	}
}

void IM_DetailShuffle::FillData() {
	m_main_items.clear();
	m_color_groups.clear();

	for (CDetailManager::DetailIt d_it = m_dm->objects.begin(); d_it != m_dm->objects.end(); ++d_it)
		m_main_items.push_back((EDetail*)(*d_it));

	for (const auto& pair : m_dm->m_ColorIndices) {
		ColorGroup grp;
		grp.color = pair.first;
		for (const auto& do_ptr : pair.second)
			grp.items.push_back(do_ptr);
		m_color_groups.push_back(grp);
	}
}

bool IM_DetailShuffle::ApplyChanges() {
	m_dm->RemoveColorIndices();

	for (const auto& grp : m_color_groups) {
		u32 clr = grp.color;

		if (grp.items.empty()) {
			m_dm->m_ColorIndices[clr];
		} else {
			for (EDetail* dd : grp.items)
				m_dm->AppendIndexObject(clr, dd->GetName(), false);
		}
	}

	if (m_bColorIndModif || m_bObjectModif) {
		ELog.DlgMsg(mtInformation, "Object or object list changed. Reinitialize needed!");
		m_dm->InvalidateSlots();
		return true;
	}
	return false;
}

void IM_DetailShuffle::ClearIndexForms() {
	m_color_groups.clear();
	xr_delete(m_thm);
	m_thm			  = nullptr;
	m_selected_detail = nullptr;
}

void IM_DetailShuffle::OnItemFocused(EDetail* dd) {
	if (m_selected_detail == dd)
		return;

	m_selected_detail = dd;
	xr_delete(m_thm);

	if (dd) {
		m_thm = CreateThumbnail(dd->GetName(), ECustomThumbnail::ETObject, true);

		PropItemVec items;
		PHelper().CreateCaption(items, "Reference Name",	m_selected_detail->GetName());
		PHelper().CreateFloat  (items, "Density Factor",	&m_selected_detail->m_fDensityFactor, 0.1f, 1.0f);
		PHelper().CreateFloat  (items, "Min Scale",			&m_selected_detail->m_fMinScale,	  0.1f, 100.0f);
		PHelper().CreateFloat  (items, "Max Scale",			&m_selected_detail->m_fMaxScale,	  0.1f, 100.f);
		PHelper().CreateFlag32 (items, "No Waving",			&m_selected_detail->m_Flags,		  DO_NO_WAVING);
		PHelper().CreateFlag32 (items, "Align to Normal",	&m_selected_detail->m_Flags,		  DO_NORMAL_ALIGN);

		m_object_props.AssignItems(items, true, false);
	} else
		m_object_props.Clear();
}

void IM_DetailShuffle::OnPropsModified() {
	m_bObjectModif = true;
}

u32 IM_DetailShuffle::GenerateUniqueColor() {
	static u32 hue = 0;
	hue			   = (hue + 137) % 360;

	Fcolor col;
	col.set_hsv(hue / 360.0f, 0.90f, 0.95f);

	return col.get();
}

void IM_DetailShuffle::RenderObjectPropertiesPanel() {
	if (!m_selected_detail)
		return;

	ImGui::BeginChild("##props", ImVec2(0, 0), true);
	m_object_props.Render();
	ImGui::EndChild();
}

void IM_DetailShuffle::RenderObjectsListPanel() {
	ImGui::BeginChild("##objects_list", ImVec2(0, m_objects_list_height), true);
	ImVec2 child_start = ImGui::GetCursorScreenPos();

	for (size_t i = 0; i < m_main_items.size(); ++i) {
		EDetail* dd = m_main_items[i];
		if (!m_filter_text.empty()) {
			xr_string name_lower   = dd->GetName();
			xr_string filter_lower = m_filter_text;
			std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(), ::tolower);
			std::transform(filter_lower.begin(), filter_lower.end(), filter_lower.begin(), ::tolower);
			if (name_lower.find(filter_lower) == xr_string::npos)
				continue;
		}

		ImGui::PushID(static_cast<int>(i));
		if (ImGui::Button("X", ImVec2(24, 0)))
			RemoveFromMainListOnly(dd);
		ImGui::SameLine(0, 4);

		bool is_selected = (m_selected_detail == dd);
		if (ImGui::Selectable(dd->GetName(), is_selected, ImGuiSelectableFlags_SpanAvailWidth))
			OnItemFocused(dd);

		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
			m_drag_item		  = dd;
			m_drag_from_group = -1;
			ImGui::SetDragDropPayload("DETAIL_ITEM", &m_drag_item, sizeof(EDetail*));
			ImGui::Text("%s", dd->GetName());
			ImGui::EndDragDropSource();
		}
		ImGui::PopID();
	}

	ImGui::SetCursorScreenPos(child_start);
	ImVec2 full_size = ImGui::GetContentRegionAvail();
	ImGui::InvisibleButton("##full_drop_zone_main", full_size);
	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DETAIL_ITEM")) {
			if (m_drag_item && m_drag_from_group >= 0) {
				auto& src_grp = m_color_groups[m_drag_from_group];
				auto  it	  = std::find(src_grp.items.begin(), src_grp.items.end(), m_drag_item);
				if (it != src_grp.items.end()) {
					src_grp.items.erase(it);
					m_bColorIndModif = true;
				}
			}
			m_drag_item		  = nullptr;
			m_drag_from_group = -1;
		}
		ImGui::EndDragDropTarget();
	}
	ImGui::EndChild();

	ImGui::InvisibleButton("##splitter", ImVec2(-1, 8));
	if (ImGui::IsItemActive() || ImGui::IsItemHovered())
		ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
	if (ImGui::IsItemActive()) {
		m_objects_list_height += ImGui::GetIO().MouseDelta.y;
		if (m_objects_list_height < 120.0f)
			m_objects_list_height = 120.0f;
		if (m_objects_list_height > 600.0f)
			m_objects_list_height = 600.0f;
	}

	float thumb_size = std::min(ImGui::GetContentRegionAvail().x, 512.0f);

	if (m_thm) {
		HDC	  hdc = m_thumbnail_canvas.BeginPaint();
		Irect r;
		r.set(0, 0, 512, 512);
		m_thm->Draw(hdc, r);
		m_thumbnail_canvas.EndPaint();

		ImGui::Image((ImTextureID)m_thumbnail_canvas.GetTexture(),
					 ImVec2(thumb_size, thumb_size),
					 ImVec2(0, 0),
					 ImVec2(1, 1),
					 ImVec4(1, 1, 1, 1),
					 ImVec4(0, 0, 0, 0));
	}

	RenderObjectPropertiesPanel();
}

void IM_DetailShuffle::RenderColorGroups() {
	ImGui::BeginChild("##color_groups", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), true);
	const float color_width = 60.0f;
	const float list_height = 180.0f;

	for (int g = 0; g < (int)m_color_groups.size(); ++g) {
		ColorGroup& grp = m_color_groups[g];
		ImGui::PushID(g);

		Fcolor fclr;
		fclr.set(grp.color);
		ImVec4 col4(fclr.r, fclr.g, fclr.b, 1.0f);
/*
		if (ImGui::ColorButton("##swatch", col4, ImGuiColorEditFlags_NoAlpha, ImVec2(color_width, list_height))) {
			ImGui::OpenPopup("ColorPickerPopup");
		}

		if (ImGui::BeginPopup("ColorPickerPopup")) {
			if (ImGui::ColorPicker4("##picker",
									(float*)&fclr,
									ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_PickerHueBar)) {
				grp.color		 = fclr.get();
				m_bColorIndModif = true;
			}
			ImGui::EndPopup();
		}
*/
		if (ImGui::ColorButton("##swatch", col4, ImGuiColorEditFlags_NoAlpha, ImVec2(color_width, list_height))) {
			u32 clr = grp.color;
			if (FSColorPicker(&clr)) {
				grp.color		 = clr;
				m_bColorIndModif = true;
			}
		}

		ImGui::SameLine(0.0f, 8.0f);
		ImGui::BeginChild("##group_list", ImVec2(0.0f, list_height), true);

		ImVec2 child_start = ImGui::GetCursorScreenPos();
		if (grp.items.empty())
			ImGui::TextDisabled("Drag objects from left list here...");

		for (size_t i = 0; i < grp.items.size(); ++i) {
			EDetail* dd = grp.items[i];
			ImGui::PushID(static_cast<int>(i));
			bool is_selected = (m_selected_detail == dd);

			if (ImGui::Selectable(dd->GetName(), is_selected))
				OnItemFocused(dd);

			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
				m_drag_item		  = dd;
				m_drag_from_group = g;
				ImGui::SetDragDropPayload("DETAIL_ITEM", &m_drag_item, sizeof(EDetail*));
				ImGui::Text("%s", dd->GetName());
				ImGui::EndDragDropSource();
			}
			ImGui::PopID();
		}

		ImGui::SetCursorScreenPos(child_start);
		ImVec2 full_size = ImGui::GetContentRegionAvail();
		ImGui::InvisibleButton("##full_drop_zone_group", full_size);

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DETAIL_ITEM")) {
				if (m_drag_item) {
					bool already_exists = std::find(grp.items.begin(), grp.items.end(), m_drag_item) != grp.items.end();
					if (!already_exists) {
						if (m_drag_from_group >= 0) {
							auto& src = m_color_groups[m_drag_from_group];
							auto  it  = std::find(src.items.begin(), src.items.end(), m_drag_item);
							if (it != src.items.end())
								src.items.erase(it);
						}
						grp.items.push_back(m_drag_item);
						m_bColorIndModif = true;
					}
				}
				m_drag_item		  = nullptr;
				m_drag_from_group = -1;
			}
			ImGui::EndDragDropTarget();
		}
		ImGui::EndChild();

		if (ImGui::Button("Remove Group")) {
			m_color_groups.erase(m_color_groups.begin() + g);
			m_bColorIndModif = true;
			--g;
		}

		ImGui::PopID();
		ImGui::Separator();
	}
	ImGui::EndChild();
}

void IM_DetailShuffle::Render() {
	if (!m_open) {
		ApplyChanges();
		UI->RemoveIMWindow(this);
		auto _this = this;
		xr_delete(_this);
		return;
	}

	ImGui::SetNextWindowSize(ImVec2(1100, 650), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("Detail Objects###DetailShuffle", &m_open)) {
		ImGui::End();
		return;
	}

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
	if (ImGui::Button("Add Object...", ImVec2(140, 0))) {
		IM_ChooseForm* cf = xr_new<IM_ChooseForm>(smObject, -1);
		cf->OnOK.bind(this, &IM_DetailShuffle::OnAddObjects);
		UI->AddIMWindow(cf);
	}
	ImGui::SameLine();
	if (ImGui::Button("Refresh List", ImVec2(120, 0))) {
		FillData();
	}
	ImGui::SameLine();
	if (ImGui::Button("Add Color Group", ImVec2(160, 0))) {
		ColorGroup grp;
		grp.color = GenerateUniqueColor();
		m_color_groups.push_back(grp);
		m_bColorIndModif = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Clear All", ImVec2(100, 0))) {
		if (ELog.DlgMsg(mtConfirmation, "Clear ALL detail objects and groups?\n\nThis action cannot be undone.") == mrYes) {
			m_dm->ClearColorIndices();
			m_dm->InvalidateSlots();
			ClearIndexForms();
			m_bColorIndModif = true;
			m_bObjectModif	 = true;
			FillData();
		}
	}
	ImGui::SameLine();
	float remaining_width = ImGui::GetContentRegionAvail().x;
	ImGui::Dummy(ImVec2(remaining_width - 216.0f, 0.0f));
	ImGui::SameLine();

	if (ImGui::Button("Save List", ImVec2(100, 0))) {
		xr_string fileName;
		if (EFS.GetSaveName(_detail_objects_, fileName)) {
			ApplyChanges();
			m_dm->ExportColorIndices(fileName.c_str());
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Load List", ImVec2(100, 0))) {
		xr_string fileName;
		if (EFS.GetOpenName(_detail_objects_, fileName)) {
			if (m_dm->ImportColorIndices(fileName.c_str())) {
				m_bColorIndModif = true;
				m_dm->InvalidateSlots();
				ClearIndexForms();
				FillData();
			}
		}
	}

	ImGui::PopStyleVar();

	char filter_buf[256] = {};
	if (!m_filter_text.empty())
		strncpy(filter_buf, m_filter_text.c_str(), sizeof(filter_buf) - 1);
	if (ImGui::InputTextWithHint("##filter", "Filter objects...", filter_buf, sizeof(filter_buf))) {
		m_filter_text = filter_buf;
		_Trim(m_filter_text);
	}

	ImGui::BeginChild("##left_panel", ImVec2(420, 0), true);
	RenderObjectsListPanel();
	ImGui::EndChild();

	ImGui::SameLine();
	ImGui::BeginChild("##right_panel", ImVec2(0, 0), true);
	RenderColorGroups();
	ImGui::EndChild();

	ImGui::End();
}

bool IM_DetailShuffle::Run(EDetailManager* dm) {
	if (!dm)
		return false;

	if (s_instance)
		return true;

	auto wnd = xr_new<IM_DetailShuffle>(dm);
	UI->AddIMWindow(wnd);
	return true;
}
