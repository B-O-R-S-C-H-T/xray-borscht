#include "stdafx.h"

#include "IM_ObjectListWnd.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "../ECore/Editor/ui_main.h"
#include "../Edit/Scene.h"
#include "../Edit/UI_LevelTools.h"
#include "../Edit/customobject.h"

IM_ObjectListWnd* IM_ObjectListWnd::instance;

void IM_ObjectListWnd::Render() {
	// нужно проверять перед перестроением списка, иначе упадёт на загрузке следующего уровня
	if (Scene && Scene->ObjCount() != m_lastSceneObjCount) {
		m_needRefresh		 = true;
		m_lastSceneObjCount = Scene->ObjCount();
	}

	 if (m_needRefresh)
		 refresh();
	 if (m_needRecalcVis)
		 recalcVis();

	 bool open			 = true;
	 int  rangeSel_group = -1;
	 int  rangeSel_top	 = -1;
	 int  rangeSel_end	 = -1;

	 ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
	 if (ImGui::Begin("Object List", &open, ImGuiWindowFlags_MenuBar)) {
		 ImGui::PushItemWidth(-1);
		 if (ImGui::InputTextWithHint("##filter", "Search...", m_filter, sizeof(m_filter) - 1))
			 m_needRefresh = true;
		 ImGui::PopItemWidth();

		 if (ImGui::BeginMenuBar()) {
			 if (ImGui::BeginMenu("Tools")) {
				 if (ImGui::MenuItem("All", nullptr, m_filterTools == 0)) {
					 m_filterTools = 0;
					 m_needRefresh = true;
				 }
				 if (ImGui::MenuItem("Current", nullptr, m_filterTools == 1)) {
					 m_filterTools = 1;
					 m_needRefresh = true;
				 }
				 ImGui::EndMenu();
			 }
			 if (ImGui::BeginMenu("Show")) {
				 if (ImGui::MenuItem("All", nullptr, m_showOptions == 0)) {
					 m_showOptions = 0;
					 m_needRefresh = true;
				 }
				 if (ImGui::MenuItem("Visible Only", nullptr, m_showOptions == 1)) {
					 m_showOptions = 1;
					 m_needRefresh = true;
				 }
				 if (ImGui::MenuItem("Invisible Only", nullptr, m_showOptions == 2)) {
					 m_showOptions = 2;
					 m_needRefresh = true;
				 }
				 ImGui::EndMenu();
			 }
			 if (ImGui::BeginMenu("Edit")) {
				 if (ImGui::MenuItem("Refresh List")) {
					 m_needRefresh = true;
				 }
				 ImGui::EndMenu();
			 }
			 ImGui::EndMenuBar();
		 }

		 ImGui::BeginChild("##items", ImVec2(0, 0), true);

		 ImGuiListClipper clipper;
		 clipper.Begin(m_vis.size(), ImGui::GetTextLineHeightWithSpacing());

		// принудительно включить элемент в отрисовку
		if (m_ScrollToSelected && m_ScrollTargetVisIndex >= 0) {
			clipper.IncludeItemByIndex(m_ScrollTargetVisIndex);
		}

		 while (clipper.Step()) {
			 ImGui::Indent();
			 for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
				 const auto& vis = m_vis[i];

				 if (vis.second == -1) {
					 // группа
					 auto& group = m_items[vis.first];
					 ImGui::Unindent();

					 if (!group.Objects.empty()) {
						 bool opened =
							 ImGui::TreeNodeEx(group.Tool, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_NoTreePushOnOpen, "%ss", group.Tool->ClassDesc());

						 if (opened != group.Opened) {
							 m_needRecalcVis = true;
							 group.Opened	 = opened;
						 }
					 } else {
						 ImGui::BulletText("%ss", group.Tool->ClassDesc());
					 }
					 ImGui::Indent();

				 } else {
					 // объект
					 auto& object  = m_items[vis.first].Objects[vis.second];
					 bool  visible = object.Obj->Visible();

					 ImGui::PushID(object.Obj);

					 if (!visible)
						 ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);

					 if (ImGui::Selectable(object.Obj->GetName(), object.Selected)) {
						 if (ImGui::GetIO().KeyShift && (rangeSel_top = getTopSelInGroup(m_vis[i].first)) >= 0) {
							 /* range select */
							 rangeSel_group = m_vis[i].first;
							 rangeSel_end	= m_vis[i].second;
						 } else {
							 /* normal select */
							 if (ImGui::GetIO().KeyCtrl) {
								 bool sel		 = !object.Selected;
								 object.Selected = sel;
								 object.Obj->Select(sel);
							 } else {
								 clearAllSel();
								 object.Selected = true;
								 object.Obj->Select(true);
							 }
						 }
					 }

					 if (!visible)
						 ImGui::PopStyleColor();

					 if (ImGui::BeginPopupContextItem()) {
						 if (visible) {
							 if (ImGui::MenuItem("Hide Selected"))
								 showSelected(false);
						 } else {
							 if (ImGui::MenuItem("Show Selected"))
								 showSelected(true);
						 }

						 ImGui::Separator();

						 if (ImGui::MenuItem("Properties..."))
							 ExecCommand(COMMAND_SHOW_PROPERTIES);

						 ImGui::EndPopup();
					 }

					 ImGui::PopID();

					if (m_ScrollToSelected && i == m_ScrollTargetVisIndex) {
						ImGui::ScrollToItem(ImGuiScrollFlags_AlwaysCenterY);
						m_ScrollToSelected		= false;
						m_ScrollTargetVisIndex = -1;
					}
				 }
			 }
			 ImGui::Unindent();
		 }
		 clipper.End();

		 ImGui::EndChild();
	 }
	 ImGui::End();

	 /* do range select */
	 if (rangeSel_group >= 0) {
		 auto& objects = m_items[rangeSel_group].Objects;
		 if (rangeSel_end > rangeSel_top) {
			 clearAllSel();
			 for (int it = rangeSel_top; it <= rangeSel_end; it++) {
				 objects[it].Selected = true;
				 objects[it].Obj->Select(true);
			 }
		 } else {
			 int rangeSel_bottom = getBottomSelInGroup(rangeSel_group);
			 clearAllSel();
			 for (int it = rangeSel_end; it <= rangeSel_bottom; it++) {
				 objects[it].Selected = true;
				 objects[it].Obj->Select(true);
			 }
		 }
	 }

	 if (!open) {
		 UI->RemoveIMWindow(this);
		 auto _this = this;
		 xr_delete(_this);
	 }
}

void IM_ObjectListWnd::RefreshList() {
	refresh();
}

void IM_ObjectListWnd::OnAdd() {
	IM_Storage& storage = EPrefs->GetSectionStorage("im_object_list");

	m_filterTools		= storage.GetInt("filter_tools", 0);
	m_showOptions		= storage.GetInt("show_options", 0);

	xr_string filterStr = storage.GetString("filter", "");
	strncpy(m_filter, filterStr.c_str(), sizeof(m_filter) - 1);
	m_filter[sizeof(m_filter) - 1] = '\0';

	m_needRefresh				   = true;
}

void IM_ObjectListWnd::OnRemove() {
	IM_Storage& storage = EPrefs->GetSectionStorage("im_object_list");

	storage.PutInt("filter_tools", m_filterTools);
	storage.PutInt("show_options", m_showOptions);
	storage.PutString("filter", m_filter);

	for (auto& group : m_items) {
		xr_string key = xr_string("opened_") + group.Tool->ClassDesc();
		storage.PutBool(key.c_str(), group.Opened);
	}

	storage.Save();
}

void IM_ObjectListWnd::refresh() {
	m_needRefresh = false;

	m_items.clear();

	// collect and sort tools
	std::vector<ESceneCustomOTool*> tools;
	tools.reserve(OBJCLASS_COUNT);
	if (m_filterTools == 1) {
		ObjClassID i = LTools->CurrentClassID();
		if (i != OBJCLASS_DUMMY)
			tools.push_back(Scene->GetOTool(i));
	} else {
		for (u32 i = 0; i != OBJCLASS_COUNT; i++) {
			auto tool = Scene->GetOTool(i);
			if (!tool)
				continue;
			tools.push_back(tool);
		}
		std::sort(tools.begin(), tools.end(), [](auto el1, auto el2) { return stricmp(el1->ClassDesc(), el2->ClassDesc()) < 0; });
	}

	IM_Storage& storage = EPrefs->GetSectionStorage("im_object_list");

	for (auto tool : tools) {
		m_items.push_back(Group{tool, true});
		auto& group = m_items.back();

		xr_string openedKey = xr_string("opened_") + tool->ClassDesc();
		group.Opened		= storage.GetBool(openedKey.c_str(), true);

		for (auto obj : tool->GetObjects()) {
			if (!strstr(obj->GetName(), m_filter))
				continue;
			if (m_showOptions == 1 && !obj->Visible())
				continue;
			if (m_showOptions == 2 && obj->Visible())
				continue;
			group.Objects.push_back(Object{obj, (bool)obj->Selected()});
		}
		std::sort(group.Objects.begin(),
				  group.Objects.end(),
				  [](const auto& el1, const auto& el2) { return stricmp(el1.Obj->GetName(), el2.Obj->GetName()) < 0; });
	}

	m_lastSceneObjCount = Scene ? Scene->ObjCount() : 0;

	// авторазворачивание групп с выделенными объектами
	bool expanded_any	= false;
	for (auto& g : m_items) {
		if (!g.Opened) {
			for (const auto& o : g.Objects) {
				if (o.Selected) {
					g.Opened	 = true;
					expanded_any = true;
					break;
				}
			}
		}
	}

	recalcVis();
	if (expanded_any)
		recalcVis();

	// ищем первый выделенный объект для прокрутки
	m_ScrollTargetVisIndex = -1;
	m_ScrollToSelected	   = false;

	for (size_t i = 0; i < m_vis.size(); ++i) {
		const auto& v = m_vis[i];
		if (v.second != -1) { // объект
			if (m_items[v.first].Objects[v.second].Selected) {
				m_ScrollTargetVisIndex = (int)i;
				m_ScrollToSelected	   = true;
				break;
			}
		}
	}
}

void IM_ObjectListWnd::clearAllSel() {
	for (auto& group : m_items)
		for (auto& obj : group.Objects) {
			obj.Selected = false;
			obj.Obj->Select(false);
		}
}

void IM_ObjectListWnd::showSelected(bool vis) {
	for (auto& group : m_items)
		for (auto& obj : group.Objects) {
			if (!obj.Selected)
				continue;
			obj.Obj->Show(vis);
			if (vis)
				obj.Obj->Select(true);
		}
}

void IM_ObjectListWnd::recalcVis() {
	m_needRecalcVis = false;

	size_t visCount = 0;
	for (const auto& group : m_items) {
		visCount += 1 + (group.Opened ? group.Objects.size() : 0);
	}
	m_vis.resize(visCount);
	size_t vis_i = 0;
	for (size_t i = 0; i != m_items.size(); i++) {
		auto& group	   = m_items[i];
		m_vis[vis_i++] = {i, -1};
		if (group.Opened)
			for (u32 j = 0; j != group.Objects.size(); j++)
				m_vis[vis_i++] = {i, j};
	}
}

int IM_ObjectListWnd::getTopSelInGroup(int group) {
	for (size_t i = 0; i < m_items[group].Objects.size(); i++) {
		if (m_items[group].Objects[i].Selected)
			return (int)i;
	}
	return -1;
}

int IM_ObjectListWnd::getBottomSelInGroup(int group) {
	int result = -1;
	for (size_t i = 0; i < m_items[group].Objects.size(); i++) {
		if (m_items[group].Objects[i].Selected)
			result = (int)i;
	}
	return result;
}