#pragma once

#include "../ECore/Editor/EThumbnail.h"
#include "../ECore/Editor/EditObject.h"
#include "../ECore/Editor/Library.h"
#include "../ECore/ImGui/IM_Canvas.h"
#include "../ECore/ImGui/IM_PropertyTree.h"
#include "../ECore/ImGui/IM_Tree.h"
#include "../ECore/ImGui/IM_Window.h"
#include "Edit/Scene.h"
#include "Edit/SceneObject.h"
#include "Edit/UI_LevelMain.h"

class IM_EditLibrary : public IM_Window {
private:
	IM_Tree					 m_Items;
	IM_PropertyTree			 m_Props;
	IM_Canvas				 m_Image;
	EObjectThumbnail*		 m_Thm	   = nullptr;
	IM_PropertiesWnd*		 m_IMProps = nullptr;

	IM_PropertyTree			 m_BasicProps;
	IM_PropertyTree			 m_SurfaceProps;

	xr_vector<CSceneObject*> m_pEditObjects;
	FS_FileSet				 modif_map;

	bool					 m_open	   = true;
	bool					 cbPreview = true;
	bool					 bReadOnly = false;

	static bool				 bFinalExit;
	static bool				 bExitResult;

	void					 InitObjects();
	void					 OnModified();
	void					 OnItemsFocused(ListItemsVec& items);
	void					 UpdateObjectProperties();
	void					 ChangeReference(const RStringVec& items);
	bool					 SelectionToReference(ListItemsVec* props = nullptr);
	void					 RefreshSelected();

	void					 ClearEditObjects();

	void __stdcall			 OnObjectRename(LPCSTR p0, LPCSTR p1, EItemType type);
	void __stdcall			 OnObjectRemove(LPCSTR name, EItemType type, bool& removed);

	void					 MakeThumbnail();
	void					 MakeLOD(bool bHighQuality);
	bool					 GenerateLOD(ListItemsVec& props, bool bHighQuality);
	void					 Save();

public:
	IM_EditLibrary();
	~IM_EditLibrary();

	static void						ShowEditor();

	virtual void					Render() override;

	static void __fastcall			OnRender();
	static void __fastcall			ZoomObject();
	static CSceneObject* __fastcall RayPick(const Fvector& start, const Fvector& direction, SRayPickInfo* pinf);

	void							FormShow();
	void							FormClose();

private:
	void OnNodeSelected(shared_str path, shared_str value, bool select);
};

extern ECORE_API IM_EditLibrary* g_imEditLibrary;
