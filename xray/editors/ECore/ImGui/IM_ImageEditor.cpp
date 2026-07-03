#include "stdafx.h"
#pragma hdrstop

#include "IM_ImageEditor.h"

#include <Layers/xrRender/ETextureParams.h>
#include <imgui.h>

#include "../Editor/ImageManager.h"
#include "../Editor/ui_main.h"

#pragma package(smart_init)

ECORE_API IM_ImageEditor imImageEditor;

IM_ImageEditor::IM_ImageEditor()
    : m_open(false)
    , m_title("Image Editor")
    , m_importmode(false)
    , m_needupdate(false)
    , m_showimage(true)
    , m_showcube(true)
    , m_showbump(true)
    , m_shownormal(true)
    , m_showterrain(true)
    , m_thm(THUMB_WIDTH, THUMB_HEIGHT)
{
	m_items.OnItemsFocused.bind(this, &IM_ImageEditor::OnItemsFocused);
}

void IM_ImageEditor::Open(bool import) {
	if (!m_open) {
		m_importmode = import;

		if (m_importmode) {
			m_title		= "Import Textures";
			int new_cnt = ImageLib.GetLocalNewTextures(texture_map);
			if (new_cnt < 1) {
				ELog.DlgMsg(mtInformation, "Can't find new textures");
				return;
			}
		} else {
			m_title = "Image Editor";
			ImageLib.GetTexturesRaw(texture_map);
		}

		ListItemsVec items;
		SPBItem*	 pb = UI->ProgressStart(texture_map.size(), "Fill list...");
		// fill
		for (auto it = texture_map.begin(); it != texture_map.end(); ++it) {
			pb->Inc();
			ListItem* I = LHelper().CreateItem(items, it->name.c_str(), 0);
			I->m_Object = (void*)(FindUsedTHM(it->name.c_str()));
			R_ASSERT2(I->m_Object, it->name.c_str());
		}
		UI->ProgressEnd(pb);

		m_items.AssignItems(items, false);

		UI->AddIMWindow(this);
		m_open = true;
	}
}

void IM_ImageEditor::Close() {
	texture_map.clear();
	modif_map.clear();
	DestroyUsedTHM();

	UI->RemoveIMWindow(this);
	m_thm.DestroyTexture();
	m_open = false;
}

void IM_ImageEditor::FinalClose() {
	if (imImageEditor.m_open) {
		UI->RemoveIMWindow(&imImageEditor);
		imImageEditor.m_open = false;
		imImageEditor.Close();
	}
}

void IM_ImageEditor::OnShowTypeClick() {
	ListItemsVec items;
	m_items.GetItems(items);

	for (ListItemsIt it = items.begin(), end = items.end(); it != end; it++) {
		ListItem*		   item = *it;
		ETextureThumbnail* thm	= (ETextureThumbnail*)item->m_Object;

		switch (thm->_Format().type) {
			case STextureParams::ttImage:
				item->Visible(m_showimage);
				break;
			case STextureParams::ttCubeMap:
				item->Visible(m_showcube);
				break;
			case STextureParams::ttBumpMap:
				item->Visible(m_showbump);
				break;
			case STextureParams::ttNormalMap:
				item->Visible(m_shownormal);
				break;
			case STextureParams::ttTerrain:
				item->Visible(m_showterrain);
				break;
		}
	}
}

void IM_ImageEditor::Render() {
	static float btn_width = ImGui::CalcTextSize("OK").x + ImGui::CalcTextSize("Cancel").x + ImGui::GetStyle().ItemSpacing.x +
							 ImGui::GetStyle().FramePadding.x * 5;

	if (!m_open) {
		Close();
		return;
	}

	ImGui::SetNextWindowSize(ImVec2(745, 555), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin(m_title.c_str(), &m_open)) {
		ImGui::End();
		return;
	}

	if (m_needupdate) {
		ListItemsVec selected;
		m_items.GetSelected(selected);
		OnItemsFocused(selected);

		m_needupdate = false;
	}

	float bottom_reserve = 42.0f;

	ImGui::Columns(2, "image_editor_cols", true);
	BeginPersistedColumns("image_editor_cols", "im_image_editor", 280.0f);

	ImGui::BeginChild("ItemList", ImVec2(0, -bottom_reserve), false);

	ImGui::SetNextItemWidth(-1);
	if (ImGui::InputTextWithHint("##search", "Search...", m_ItemsFilter, sizeof(m_ItemsFilter))) {
		m_items.SetFilter(m_ItemsFilter);
	}

	float buttons_reserved_height = ImGui::GetFrameHeightWithSpacing();

	ImGui::BeginChild("##TreeScrollArea", ImVec2(0, -buttons_reserved_height), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
	m_items.Render();

	if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && !ImGui::IsAnyItemHovered()) {
		ImGui::OpenPopup("##TypeFilterContext");
	}

	if (ImGui::BeginPopup("##TypeFilterContext")) {
		ImGui::TextDisabled("Show Types");
		ImGui::Separator();

		if (ImGui::MenuItem("Diffuse", nullptr, &m_showimage))		OnShowTypeClick();
		if (ImGui::MenuItem("Bump", nullptr, &m_showbump))			OnShowTypeClick();
		if (ImGui::MenuItem("Normal", nullptr, &m_shownormal))		OnShowTypeClick();
		if (ImGui::MenuItem("Cube", nullptr, &m_showcube))			OnShowTypeClick();
		if (ImGui::MenuItem("Terrain", nullptr, &m_showterrain))	OnShowTypeClick();

		ImGui::EndPopup();
	}

	ImGui::EndChild();

	ImGui::BeginGroup();
	{
		if (ImGui::Button("Collapse Others"))
			m_items.CollapseAllExceptSelection();
		ImGui::SameLine();
		if (ImGui::Button("Expand Descendants"))
			m_items.ExpandAllDescendantsOfSelection();
	}
	ImGui::EndGroup();

	ImGui::EndChild();

	ImGui::NextColumn();

	ImGui::BeginChild("RightPanel", ImVec2(0, -bottom_reserve), false);

	ImGui::BeginChild("THM", ImVec2(m_thm.Width() + 16, m_thm.Height() + 16), false);
	m_thm.Render();
	ImGui::EndChild();

	ImGui::BeginChild("ItemProps", ImVec2(0, 0), true);
	RenderPersistedPropertyTree(m_props, "im_image_editor");
	ImGui::EndChild();

	ImGui::EndChild();

	EndPersistedColumns("image_editor_cols", "im_image_editor");
	ImGui::Columns(1);

	ImGui::Indent(ImGui::GetContentRegionAvail().x - 172);

	if (ImGui::Button("OK##image_ok", ImVec2(80, 0))) {
		UpdateLib();
		m_open = false;
	}
	ImGui::SameLine();
	if (ImGui::Button("Cancel##image_cancel", ImVec2(80, 0))) {
		m_open = false;
	}

	ImGui::End();
}

ETextureThumbnail* IM_ImageEditor::FindUsedTHM(shared_str name) {
	THMMapIt it = m_THM_Used.find(name);
	if (it != m_THM_Used.end())
		return it->second;

	ETextureThumbnail* thm = xr_new<ETextureThumbnail>(name.c_str(), false);
	m_THM_Used[name]	   = thm;

	if (m_importmode) {
		xr_string fn = name.c_str();
		ImageLib.UpdateFileName(fn);

		if (!thm->Load(name.c_str(), _import_)) {
			bool bLoad = thm->Load(fn.c_str(), _game_textures_);
			ImageLib.CreateTextureThumbnail(thm, name.c_str(), _import_, !bLoad);
		}
	} else {
		thm->Load();
	}
	return thm;
}

void IM_ImageEditor::SaveUsedTHM() {
	for (THMMapIt t_it = m_THM_Used.begin(); t_it != m_THM_Used.end(); ++t_it) {
		if (modif_map.find(FS_File(t_it->second->SrcName())) != modif_map.end())
			t_it->second->Save();
	}
}

void IM_ImageEditor::DestroyUsedTHM() {
	for (THMMapIt it = m_THM_Used.begin(); it != m_THM_Used.end(); ++it)
		xr_delete(it->second);
	m_THM_Used.clear();
}

void IM_ImageEditor::OnItemsFocused(ListItemsVec& items) {
	PropItemVec props;

	RegisterModifiedTHM();
	m_THM_Current.clear();

	if (!items.empty()) {
		for (ListItemsIt it = items.begin(); it != items.end(); it++) {
			ListItem* prop = *it;
			if (prop) {
				ETextureThumbnail* thm = 0;

				thm					   = FindUsedTHM(prop->Key());
				m_THM_Current.push_back(thm);
				// prop->tag								= thm->_Format().type;

				// fill prop
				thm->FillProp(props, PropValue::TOnChange(this, &IM_ImageEditor::OnTypeChange));

				if (thm->_Format().type == STextureParams::ttCubeMap) {
					ButtonValue* B = PHelper().CreateButton(props, "CubeMap\\Edit", "Make Small", 0);
					// B->OnBtnClickEvent.bind(this,&TfrmImageLib::OnCubeMapBtnClick);
				}
			}
		}
	}

	if (m_THM_Current.size() == 1) {
		HDC hdc = m_thm.BeginPaint();
		m_THM_Current.back()->Draw(hdc, Irect().set(0, 0, m_thm.Width(), m_thm.Height()));
		m_thm.EndPaint();
	} else
		m_thm.Clear();

	m_props.AssignItems(props, true, false);
}

void IM_ImageEditor::OnTypeChange(PropValue*) { m_needupdate = true; }

void IM_ImageEditor::UpdateLib() {
	RegisterModifiedTHM();
	SaveUsedTHM();
	if (m_importmode && !texture_map.empty()) {
		AStringVec modif;
		ImageLib.SafeCopyLocalToServer(texture_map);
		// rename with folder
		FS_FileSet files = texture_map;
		texture_map.clear();
		xr_string	 fn;
		FS_FileSetIt it = files.begin();
		FS_FileSetIt _E = files.end();

		for (; it != _E; it++) {
			fn = EFS.ChangeFileExt(it->name.c_str(), "");
			ImageLib.UpdateFileName(fn);
			FS_File F(*it);
			F.name = fn;
			texture_map.insert(F);
		}
		// sync
		ImageLib.SynchronizeTextures(true, &texture_map, &modif);
		ImageLib.RefreshTextures(&modif);
	} else {
		// save game textures
		if (modif_map.size()) {
			AStringVec modif;
			ImageLib.SynchronizeTextures(true, &modif_map, &modif);
			ImageLib.RefreshTextures(&modif);
		}
	}
}

void IM_ImageEditor::RegisterModifiedTHM() {
	if (m_props.IsModified() || m_importmode) {
		for (THMIt t_it = m_THM_Current.begin(); t_it != m_THM_Current.end(); t_it++) {
			FS_FileSetIt it = texture_map.find(FS_File((*t_it)->SrcName()));
			R_ASSERT(it != texture_map.end());
			modif_map.insert(*it);
		}
	}
}
