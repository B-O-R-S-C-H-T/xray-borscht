#include "stdafx.h"
#pragma hdrstop

#include "IM_ChooseForm.h"

#include <Layers/xrRender/ETextureParams.h>
#include <editors/xrEProps/ChoseForm.h> // for GetEvents
#include <imgui.h>

#include "../Editor/ui_main.h"

#pragma package(smart_init)

IM_ChooseForm::IM_ChooseForm(EChooseMode mode, int max_select, ChooseItemVec* items, TOnChooseFillItems fill_items, void* fill_param)
	: OnOK(NULL), OnCancel(NULL), OnClose(NULL), m_show_thumbnail(true), m_thm(THUMB_WIDTH, THUMB_HEIGHT), m_current_item(0), m_open(true) {
	m_events = TfrmChoseItem::GetEvents(mode);

	if (max_select > 1) {
		m_items_tree				  = IM_Tree('\\', true, true);
		m_items_tree.MaxSelectedCount = max_select;
		m_items_tree.CheckmarkMode	  = true;
	}

	m_items_tree.OnNodeSelected = IM_TOnNodeSelected(this, &IM_ChooseForm::OnNodeSelected);

	// fill items
	if (items) {
		VERIFY2(fill_items.empty(), "double source");
		ChooseItemVecIt it = items->begin();
		for (; it != items->end(); it++)
			m_items_tree.Add((*it).name.c_str(), (*it).hint.c_str());
	} else {
		ChooseItemVec items;
		if (!fill_items.empty()) {
			fill_items(items, fill_param);
		} else {
			VERIFY2(m_events && !m_events->on_fill.empty(), "no source");
			m_events->on_fill(items, fill_param);
		}

		ChooseItemVecIt it = items.begin();
		for (; it != items.end(); it++)
			m_items_tree.Add((*it).name.c_str(), (*it).hint.c_str());
	}
}

void IM_ChooseForm::OnNodeSelected(shared_str path, shared_str value, bool select) {
	m_props.Clear();
	m_thm.Clear();

	xr_vector<shared_str>::iterator it;
	for (it = m_selected.begin(); it != m_selected.end(); it++) {
		if (select && *it == path)
			return;
		else if (!select && *it == path) {
			m_selected.erase(it);
			return;
		}
	}

	m_selected.push_back(path);
}

void IM_ChooseForm::Close() {
	if (m_events && !m_events->on_close.empty())
		m_events->on_close();

	if (!OnClose.empty())
		OnClose(this);

	UI->RemoveIMWindow(this);
	IM_ChooseForm* _this = this;
	xr_delete(_this);
}

void IM_ChooseForm::SetSelected(shared_str selected) {
	m_items_tree.SelectAll(false);
	m_current_item = 0;

	int cnt		   = _GetItemCount(*selected, ',');
	for (int i = 0; i < cnt; i++) {
		xr_string item;
		_GetItem(*selected, i, item);

		m_items_tree.Select(item.c_str(), true);
	}
}

shared_str IM_ChooseForm::GetSelected() {
	xr_string selected = "";
	for (size_t i = 0; i < m_selected.size(); i++) {
		if (i > 0)
			selected += ",";
		selected += *m_selected[i];
	}

	return shared_str(selected.c_str());
}

void IM_ChooseForm::UpdateSelectionUI(float height) {
	if (m_items_tree.MaxSelectedCount <= 1)
		return;
	ImGui::TextUnformatted("Selection:");
	ImGui::BeginChild("##selection", ImVec2(-1, height), true);
	size_t size = m_selected.size();
	for (size_t i = 0; i < size; i++) {
		if (ImGui::Selectable(m_selected[i].c_str(), i == m_current_item))
			m_current_item = i;
	}
	ImGui::EndChild();
	if (ImGui::Button(u8"↑##choose_up"))
		if (m_current_item > 0 && m_current_item < m_selected.size()) {
			std::swap(m_selected[m_current_item], m_selected[m_current_item - 1]);
			m_current_item--;
		}
	ImGui::SameLine();
	if (ImGui::Button(u8"↓##choose_down"))
		if (m_selected.size() && m_current_item < m_selected.size() - 1) {
			std::swap(m_selected[m_current_item], m_selected[m_current_item + 1]);
			m_current_item++;
		}
	ImGui::SameLine();
	if (ImGui::Button("Remove##choose_remove"))
		if (m_selected.size() && m_current_item < m_selected.size())
			m_items_tree.Select(*m_selected[m_current_item], false);
	ImGui::SameLine();
	if (ImGui::Button("Clear All##choose_clear")) {
		m_items_tree.SelectAll(false);
		m_current_item = 0;
	}
}

void IM_ChooseForm::Render() {
	if (!m_open) {
		Close();
		return;
	}

	const char* caption = m_events ? m_events->caption.c_str() : "Select Item";
	const float phi		= 1.618f;
	ImGui::SetNextWindowSize(ImVec2(550.0f, 550.0f * phi), ImGuiCond_FirstUseEver);

	ImGui::OpenPopup(caption);
	if (ImGui::BeginPopupModal(caption, &m_open)) {
		int h = ImGui::GetFrameHeight() + 5;

		ImGui::Columns(2, "##ccc", false);

		ImGui::BeginChild("##items", ImVec2(0, -h), true);
		m_items_tree.Render();
		ImGui::EndChild();

		ImGui::NextColumn();

		ImGui::Checkbox("Show thumbnail", &m_show_thumbnail);
		if (m_show_thumbnail) {
			ImGui::BeginChild("##thm", ImVec2(m_thm.Width() + 16, m_thm.Height() + 16), true);
			m_thm.Render();
			ImGui::EndChild();
		}

		ImGui::BeginChild("##lower_right", ImVec2(0, -h), true);
		{
			float avail = ImGui::GetContentRegionAvail().y;

			if (m_items_tree.MaxSelectedCount > 1) {
				float selection_h = avail * m_selection_ratio;
				float splitter_h  = 8.0f;
				float props_h	  = avail - selection_h - splitter_h;

				UpdateSelectionUI(selection_h);

				ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.6f, 0.6f, 0.6f, 0.6f));
				ImGui::Separator();
				ImGui::PopStyleColor();

				ImGui::InvisibleButton("##vsplitter", ImVec2(-1.0f, splitter_h));
				if (ImGui::IsItemHovered() || ImGui::IsItemActive()) {
					ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
				}
				if (ImGui::IsItemActive()) {
					m_selection_ratio += ImGui::GetIO().MouseDelta.y / avail;
					if (m_selection_ratio < 0.1f)
						m_selection_ratio = 0.1f;
					if (m_selection_ratio > 0.9f)
						m_selection_ratio = 0.9f;
				}

				ImGui::TextUnformatted("Properties:");
				ImGui::BeginChild("##props", ImVec2(0, props_h), true);
				m_props.Render();
				ImGui::EndChild();
			} else {
				ImGui::TextUnformatted("Properties:");
				ImGui::BeginChild("##props", ImVec2(0, -1.0f), true);
				m_props.Render();
				ImGui::EndChild();
			}
		}
		ImGui::EndChild(); // ##lower_right

		ImGui::Columns(1);

		if (ImGui::Button("OK##choose_ok", ImVec2(80, 0))) {
			if (!OnOK.empty())
				OnOK(this);

			m_open = false;
		}

		ImGui::SameLine();
		if (ImGui::Button("Cancel##choose_cancel", ImVec2(80, 0))) {
			if (!OnCancel.empty())
				OnCancel(this);

			m_open = false;
		}

		ImGui::EndPopup();
	}
}
