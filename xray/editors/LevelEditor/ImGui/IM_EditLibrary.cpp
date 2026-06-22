#include "stdafx.h"

#include "IM_EditLibrary.h"

#include <imgui.h>

#include "../../xrCore/xr_trims.h"
#include "../ECore/Editor/EThumbnail.h"
#include "../ECore/Editor/EditObject.h"
#include "../ECore/Editor/ImageManager.h"
#include "../ECore/Editor/Library.h"
#include "../ECore/Editor/ui_main.h"
#include "../ECore/Engine/Image.h"
#include "../ECore/imgui/im_tree.h"
#include "Edit/Builder.h"
#include "Edit/ESceneDOTools.h"
#include "Edit/Scene.h"
#include "Edit/SceneObject.h"
#include "Edit/UI_LevelMain.h"
#include "Edit/UI_LevelTools.h"

IM_EditLibrary* g_imEditLibrary				= nullptr;

bool			IM_EditLibrary::bFinalExit	= false;
bool			IM_EditLibrary::bExitResult = true;

IM_EditLibrary::IM_EditLibrary() : m_Image(128, 128) {
	m_Items.OnNodeSelected		   = IM_TOnNodeSelected(this, &IM_EditLibrary::OnNodeSelected);
	m_Items.OnItemsFocused		   = TOnILItemsFocused(this, &IM_EditLibrary::OnItemsFocused);
	m_Items.OnItemRename		   = TOnItemRename(this, &IM_EditLibrary::OnObjectRename);
	m_Items.OnItemRemove		   = TOnItemRemove(this, &IM_EditLibrary::OnObjectRemove);

	m_Items.multiselect			   = true;

	m_Items.SetCacheInvalidator(
		[this](const shared_str& path)
		{
			if (!path.empty())
				m_Items.InvalidateCache(path);
		});

	m_BasicProps.OnModifiedEvent   = TOnModifiedEvent(this, &IM_EditLibrary::OnModified);
	m_SurfaceProps.OnModifiedEvent = TOnModifiedEvent(this, &IM_EditLibrary::OnModified);
	bReadOnly					   = false;
	cbPreview					   = true;
	m_Thm						   = nullptr;

	if (!g_imEditLibrary)
		g_imEditLibrary = this;
}

IM_EditLibrary::~IM_EditLibrary() {
	if (g_imEditLibrary == this)
		g_imEditLibrary = nullptr;
}

void IM_EditLibrary::ShowEditor() {
	if (!g_imEditLibrary)
		g_imEditLibrary = xr_new<IM_EditLibrary>();

	UI->AddIMWindow(g_imEditLibrary);
	g_imEditLibrary->FormShow();
}

void IM_EditLibrary::FormShow() {
	UI->BeginEState(esEditLibrary);
	InitObjects();

	Flight L;
	ZeroMemory(&L, sizeof(Flight));
	L.type = D3DLIGHT_DIRECTIONAL;
	L.diffuse.set(1, 1, 1, 1);
	L.direction.set(1, -1, 1);
	L.direction.normalize();
	Device.SetLight(0, L);
	Device.LightEnable(0, true);

	L.diffuse.set(0.5f, 0.5f, 0.5f, 1);
	L.direction.set(1, -1, -1);
	L.direction.normalize();
	Device.SetLight(1, L);
	Device.LightEnable(1, true);
}

void IM_EditLibrary::FormClose() {
	if (!bFinalExit && !modif_map.empty()) {
		UI->SetStatus("Objects reloading...");
		for (const auto& f : modif_map)
			Lib.ReloadObject(f.name.c_str());
		UI->ResetStatus();
	}

	if (Scene->locked())
		Scene->unlock();
	UI->EndEState(esEditLibrary);

	Device.LightEnable(0, false);
	Device.LightEnable(1, false);

	ClearEditObjects();
	xr_delete(m_Thm);

	UI->RemoveIMWindow(this);
}

void IM_EditLibrary::ClearEditObjects() {
	for (auto& SO : m_pEditObjects)
		xr_delete(SO);
	m_pEditObjects.clear();
}

void IM_EditLibrary::InitObjects() {
	ListItemsVec items;
	FS_FileSet	 lst;
	Lib.GetObjects(lst);

	m_Items.Clear();
	for (const auto& f : lst)
		m_Items.Add(f.name.c_str(), (ListItem*)nullptr);
}

void IM_EditLibrary::OnModified() {
	if (!m_open)
		return;

	for (auto& SO : m_pEditObjects) {
		CEditableObject* E = SO->GetReference();
		if (E) {
			modif_map.insert(FS_File(E->GetName()));
			E->Modified();
			SO->UpdateTransform();
		}
	}
	UI->RedrawScene();

	UpdateObjectProperties();
}

void IM_EditLibrary::OnItemsFocused(ListItemsVec& items) {
	xr_delete(m_Thm);

	xr_vector<shared_str> selected_paths;
	m_Items.GetSelectedPaths(selected_paths);

	bool b_one = (selected_paths.size() == 1);

	if (b_one && UI->ContainEState(esEditLibrary)) {
		string_path thm_fn;
		FS.update_path(thm_fn, _objects_, ChangeFileExt(selected_paths[0].c_str(), ".thm").c_str());
		if (FS.exist(thm_fn))
			m_Thm = xr_new<EObjectThumbnail>(selected_paths[0].c_str());
	}

	if (cbPreview || (m_IMProps && m_IMProps->IsOpen())) {
		RStringVec sel_strings;
		for (const auto& path : selected_paths)
			sel_strings.push_back(path.c_str());

		ChangeReference(sel_strings);
	}

	if (m_Thm && m_Thm->Valid()) {
		HDC	  hdc = m_Image.BeginPaint();
		Irect r;
		r.set(0, 0, m_Image.Width(), m_Image.Height());
		m_Thm->Draw(hdc, r);
		m_Image.EndPaint();

	} else {
		m_Image.Clear();
	}

	UpdateObjectProperties();
	UI->RedrawScene();
}

void IM_EditLibrary::UpdateObjectProperties() {
	if (m_pEditObjects.empty()) {
		m_BasicProps.Clear();
		m_SurfaceProps.Clear();
		return;
	}

	PropItemVec basicItems;
	PropItemVec surfaceItems;

	for (const auto& SO : m_pEditObjects) {
		CEditableObject* O = SO->GetReference();
		if (!O)
			continue;

		O->FillBasicProps("Main Options", basicItems);

		for (SurfaceIt s_it = O->FirstSurface(); s_it != O->LastSurface(); ++s_it) {
			AnsiString pref = AnsiString("Surfaces\\") + (*s_it)->_Name();
			O->FillSurfaceProps(*s_it, pref.c_str(), surfaceItems);
		}
	}

	m_BasicProps.AssignItems(basicItems, true, false);
	m_SurfaceProps.AssignItems(surfaceItems, true, false);
}

bool IM_EditLibrary::SelectionToReference(ListItemsVec* props) {
	RStringVec	 sel_strings;
	ListItemsVec sel_items;

	if (props == nullptr)
		m_Items.GetSelected(sel_items);
	else
		sel_items = *props;

	for (auto& item : sel_items)
		sel_strings.push_back(item->Key());

	if (!sel_strings.empty()) {
		ChangeReference(sel_strings);
		return true;
	}
	return false;
}

void IM_EditLibrary::ChangeReference(const RStringVec& items) {
	ClearEditObjects();

	for (const auto& name : items) {
		CSceneObject* SO = xr_new<CSceneObject>(nullptr, nullptr);
		m_pEditObjects.push_back(SO);

		SO->SetReference(name.c_str());

		CEditableObject* NE = SO->GetReference();
		if (NE) {
			SO->FPosition = NE->t_vPosition;
			SO->FRotation = NE->t_vRotate;
			SO->FScale	  = NE->t_vScale;

			SO->UpdateTransform();
		}
	}

	ExecCommand(COMMAND_EVICT_OBJECTS);
	ExecCommand(COMMAND_EVICT_TEXTURES);

//	ELog.Msg(mtInformation, "ChangeReference: %d objects loaded into preview", m_pEditObjects.size());
}

void IM_EditLibrary::RefreshSelected() {
	if (m_open && cbPreview) {
		SelectionToReference(nullptr);
		UI->RedrawScene();
	}
}

void IM_EditLibrary::Render() {
	if (!m_open) {
		FormClose();
		auto _this = this;
		xr_delete(_this);
		return;
	}

	ImGui::SetNextWindowSize(ImVec2(820, 620), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Object Library", &m_open, ImGuiWindowFlags_NoCollapse)) {
		ImGui::Columns(2, "lib_cols", true);

		BeginPersistedColumns("lib_cols", "im_objectlibrary", 280.0f);

		ImGui::BeginChild("ItemsPanel", ImVec2(0, 0), true);
		m_Items.Render();
		ImGui::EndChild();

		ImGui::NextColumn();

		ImGui::BeginChild("RightPanel", ImVec2(0, 0), false);

		bool canMakeThumbnail = m_Items.GetSelectedCount() > 0;
		ImGui::BeginDisabled(!canMakeThumbnail);
		if (ImGui::Button("Make Thumbnail", ImVec2(-1, 0))) {
			MakeThumbnail();
		}
		ImGui::EndDisabled();

		bool canMakeLOD = cbPreview && !bReadOnly && (m_Items.GetSelectedCount() > 0);
		ImGui::BeginDisabled(!canMakeLOD);
		if (ImGui::Button("Make LOD", ImVec2(-1, 0))) {
			MakeLOD(true);
		}
		ImGui::EndDisabled();

		ImGui::Dummy(ImVec2(0, 8.0f));

		ImGui::BeginGroup();
		{
			ImGui::BeginChild("Thumbnail", ImVec2(140, 140), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
			m_Image.Render();
			ImGui::EndChild();

			ImGui::SameLine();
			ImGui::BeginGroup();
			{
				ImGui::Dummy(ImVec2(0, 12));
				if (ImGui::Checkbox("Preview", &cbPreview))
					RefreshSelected();

				float used		= ImGui::GetCursorPosY();
				float remaining = 140.0f - used - 8.0f;
				if (remaining > 0)
					ImGui::Dummy(ImVec2(0, remaining));

				if (m_Thm && m_Thm->Valid()) {
					ImGui::Text("Faces: %d", m_Thm->_FaceCount());
					ImGui::Text("Vertices: %d", m_Thm->_VertexCount());
				}
			}
			ImGui::EndGroup();
		}
		ImGui::EndGroup();

		ImGui::BeginDisabled(modif_map.empty());
		if (ImGui::Button("Save", ImVec2(-1, 0)))
			Save();
		ImGui::EndDisabled();

		ImGui::Separator();

		if (ImGui::BeginTabBar("ObjectPropsTabs")) {
			if (ImGui::BeginTabItem("Main Options")) {
				RenderPersistedPropertyTree(m_BasicProps, "im_object_library", "basic_props_splitter_width");
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Surfaces")) {
				RenderPersistedPropertyTree(m_SurfaceProps, "im_object_library", "surface_props_splitter_width");
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}

		ImGui::EndChild();
		EndPersistedColumns("lib_cols", "im_object_library");
		ImGui::Columns(1);
	}

	ImGui::End();
}

void IM_EditLibrary::OnNodeSelected(shared_str path, shared_str value, bool select) {
	ListItemsVec dummy;
	OnItemsFocused(dummy);
}

void IM_EditLibrary::OnObjectRename(LPCSTR p0, LPCSTR p1, EItemType type) {
	Lib.RenameObject(p0, p1, type);
}

void IM_EditLibrary::OnObjectRemove(LPCSTR name, EItemType type, bool& removed) {
	auto it = m_pEditObjects.begin();
	while (it != m_pEditObjects.end()) {
		CSceneObject*	 SO = *it;
		CEditableObject* EO = SO ? SO->GetReference() : nullptr;
		if (EO && strcmp(EO->GetName(), name) == 0) {
			xr_delete(SO);
			it = m_pEditObjects.erase(it);
		} else {
			++it;
		}
	}
	Lib.RemoveObject(name, type, removed);
}

CSceneObject* IM_EditLibrary::RayPick(const Fvector& start, const Fvector& direction, SRayPickInfo* pinf) {
	if (!g_imEditLibrary)
		return nullptr;
	if (g_imEditLibrary->m_pEditObjects.empty())
		return nullptr;

	float		 dist = UI->ZFar(); // или Device.m_Camera.GetFar()
	SRayPickInfo localPinf;

	for (auto& SO : g_imEditLibrary->m_pEditObjects) {
		if (SO->RayPick(dist, start, direction, &localPinf)) {
			if (pinf)
				*pinf = localPinf;

			pinf->s_obj = SO;
			return SO;
		}
	}
	return nullptr;
}

void __fastcall IM_EditLibrary::OnRender() {
	if (!g_imEditLibrary)
		return;
	if (!g_imEditLibrary->m_open)
		return;
	if (!g_imEditLibrary->cbPreview)
		return;

	xr_vector<CSceneObject*>::iterator it	= g_imEditLibrary->m_pEditObjects.begin();
	xr_vector<CSceneObject*>::iterator it_e = g_imEditLibrary->m_pEditObjects.end();

	for (; it != it_e; ++it) {
		CSceneObject*	 SO = *it;
		CEditableObject* O	= SO->GetReference();
		if (O) {
			if (!SO->FPosition.similar(O->t_vPosition))
				SO->FPosition = O->t_vPosition;
			if (!SO->FRotation.similar(O->t_vRotate))
				SO->FRotation = O->t_vRotate;
			if (!SO->FScale.similar(O->t_vScale))
				SO->FScale = O->t_vScale;

			SO->UpdateTransform();
			SO->OnFrame();
			SO->RenderSingle();
		}
	}
}

void __fastcall IM_EditLibrary::ZoomObject() {
	if (!g_imEditLibrary)
		return;
	if (!g_imEditLibrary->m_open)
		return;
	if (!g_imEditLibrary->cbPreview)
		return;

	Fbox bb_max;
	bb_max.invalidate();

	xr_vector<CSceneObject*>::iterator it	= g_imEditLibrary->m_pEditObjects.begin();
	xr_vector<CSceneObject*>::iterator it_e = g_imEditLibrary->m_pEditObjects.end();
	for (; it != it_e; ++it) {
		CSceneObject* SO = *it;
		Fbox		  bb;
		if (SO->GetBox(bb))
			bb_max.merge(bb);
	}

	if (bb_max.is_valid())
		Device.m_Camera.ZoomExtents(bb_max);
}

void IM_EditLibrary::Save() {
	if (modif_map.empty())
		return;

	Lib.Save(&modif_map);

	shared_str current_selection = m_Items.GetSelectedPath();

	modif_map.clear();

	Lib.CleanLibrary();
	InitObjects();

	if (current_selection.size() > 0) {
		m_Items.Select(*current_selection, true);

		RStringVec names;
		names.push_back(current_selection);
		ChangeReference(names);
	} else {
		RStringVec empty;
		ChangeReference(empty);
	}

	RefreshSelected();
	UI->RedrawScene();
}

void IM_EditLibrary::MakeThumbnail() {
	if (!cbPreview) {
		ELog.DlgMsg(mtError, "Cannot create thumbnail! Set preview mode.");
		return;
	}

	xr_vector<shared_str> selected_paths;
	m_Items.GetSelectedPaths(selected_paths);

	SPBItem* pb = UI->ProgressStart(selected_paths.size(), "Creating thumbnails...");

	for (const shared_str& path : selected_paths) {
		pb->Inc(path.c_str());

		RStringVec single_sel;
		single_sel.push_back(path.c_str());
		ChangeReference(single_sel);

		UI->RedrawScene();

		CEditableObject* obj = Lib.CreateEditObject(path.c_str());
		if (obj) {
			string_path fn;
			FS.update_path(fn, _objects_, ChangeFileExt(path.c_str(), ".thm").c_str());

			if (ImageLib.CreateOBJThumbnail(fn, obj, obj->Version())) {
				ELog.Msg(mtInformation, "Thumbnail successfully created for '%s.object'", path.c_str());

				m_Items.InvalidateCache(path);

				if (selected_paths.size() == 1 || m_Items.GetSelectedPath() == path) {
					xr_delete(m_Thm);

					string_path thm_fn;
					FS.update_path(thm_fn, _objects_, ChangeFileExt(path.c_str(), ".thm").c_str());

					if (FS.exist(thm_fn)) {
						m_Thm = xr_new<EObjectThumbnail>(path.c_str());
					}

					if (m_Thm && m_Thm->Valid()) {
						HDC	  hdc = m_Image.BeginPaint();
						Irect r{0, 0, m_Image.Width(), m_Image.Height()};
						m_Thm->Draw(hdc, r);
						m_Image.EndPaint();
					} else {
						m_Image.Clear();
					}
				}
			} else {
				ELog.Msg(mtError, "Failed to create thumbnail for '%s.object'!", path.c_str());
			}

			Lib.RemoveEditObject(obj);
		} else {
			ELog.Msg(mtError, "Cannot load '%s.object'!", path.c_str());
		}

		if (UI->NeedAbort()) {
			break;
		}
	}

	UI->ProgressEnd(pb);
	UI->RedrawScene();
}

void IM_EditLibrary::MakeLOD(bool bHighQuality) {
	if (!modif_map.empty()) {
		ELog.Msg(mtInformation, "Save library changes before generating LOD!");
		return;
	}

	xr_vector<shared_str> selected_paths;
	m_Items.GetSelectedPaths(selected_paths);

	ListItemsVec props;
	for (const shared_str& path : selected_paths) {
		ListItem* item = LHelper().CreateItem(props, path.c_str(), 0, 0, nullptr);
	}

	bool any_lod_generated = GenerateLOD(props, bHighQuality);

	if (any_lod_generated) {
		ELog.Msg(mtInformation, "LOD generation completed.");
	} else {
		ELog.Msg(mtInformation, "LOD generation failed!");
	}
}

bool IM_EditLibrary::GenerateLOD(ListItemsVec& props, bool bHighQuality) {
	if (props.empty())
		return true;

	SPBItem* pb			   = UI->ProgressStart(props.size(), "Making LOD...");
	int		 success_count = 0;

	int		 current_age   = time(NULL);

	for (ListItemsIt it = props.begin(); it != props.end(); ++it) {
		ListItem* item = *it;

		RStringVec sel_strings;
		sel_strings.push_back(item->Key());
		ChangeReference(sel_strings);

		R_ASSERT(m_pEditObjects.size() == 1);
		CSceneObject*	 SO = m_pEditObjects[0];
		CEditableObject* O	= SO ? SO->GetReference() : nullptr;

		if (O && O->IsMUStatic()) {
			pb->Inc(O->GetName());

			BOOL bLod = O->m_objectFlags.is(CEditableObject::eoUsingLOD);
			O->m_objectFlags.set(CEditableObject::eoUsingLOD, FALSE);

			xr_string	tex_name = EFS.ChangeFileExt(O->GetName(), "");
			string_path tmp;
			strcpy(tmp, tex_name.c_str());
			_ChangeSymbol(tmp, '\\', '_');
			tex_name = xr_string("lod_") + tmp;
			tex_name = ImageLib.UpdateFileName(tex_name);

			ImageLib.CreateLODTexture(O, tex_name.c_str(), LOD_IMAGE_SIZE, LOD_IMAGE_SIZE, LOD_SAMPLE_COUNT, current_age, bHighQuality ? 4 : 1);

			O->OnDeviceDestroy();
			O->m_objectFlags.set(CEditableObject::eoUsingLOD, bLod);

			ELog.Msg(mtInformation, "LOD for '%s.object' successfully created.", O->GetName());
			++success_count;
		} else {
			ELog.Msg(mtError, "Can't create billboard: object %s must be of 'Multiple Usage' type!", item->Key());
		}

		if (UI->NeedAbort())
			break;
	}

	UI->ProgressEnd(pb);
	return (success_count > 0);
}
