#ifndef IM_SoundEditorH
#define IM_SoundEditorH

#include "IM_Window.h"
#include "IM_Tree.h"
#include "IM_PropertyTree.h"
#include "../Editor/EThumbnail.h"

class ECORE_API IM_SoundEditor : public IM_Window {
public:
	bool			m_open;
	xr_string		m_title;

	IM_Tree			m_items;
	IM_PropertyTree m_props;

	DEFINE_VECTOR(ESoundThumbnail*, THMVec, THMIt);
	THMVec m_THM_Used;
	THMVec m_THM_Current;

	enum {
		flUpdateProperties = (1 << 0),
		flReadOnly		   = (1 << 1),
	};
	Flags32	   m_Flags;
	ref_sound  m_Snd;
	FS_FileSet modif_map;

	BOOL	   bAutoPlay = TRUE;

private:
	char m_ItemsFilter[256] = {};

public:
	string_path			  pending_rename_old = {};
	string_path			  pending_rename_new = {};
	bool				  has_pending_rename = false;

	xr_vector<shared_str> pending_removes;
	bool				  has_dangerous_remove = false;

public:
	IM_SoundEditor();
	~IM_SoundEditor();

	void			 Open();
	void			 Close();
	static void		 FinalClose();

	virtual void	 OnRemove();
	virtual void	 Render();

	void			 InitItemsList();

	ESoundThumbnail* FindUsedTHM(LPCSTR name);
	void			 SaveUsedTHM();
	void			 DestroyUsedTHM();

	void			 AppendModif(LPCSTR nm);
	void			 RegisterModifiedTHM();
	void			 UpdateLib();

	void			 OnItemsFocused(ListItemsVec& items);
	void			 OnControlClick(ButtonValue* sender, bool& bModif, bool& bSafe);
	void			 OnControl2Click(ButtonValue* sender, bool& bModif, bool& bSafe);
	void			 OnSyncCurrentClick(ButtonValue* sender, bool& bModif, bool& bSafe);
	void			 OnAttClick(ButtonValue* sender, bool& bModif, bool& bSafe);
	void			 OnPlotDraw(PlotValue* sender, void*);

	bool			 IsDangerousSound(LPCSTR fname) const;
	void			 RemoveSound(LPCSTR fname, EItemType type, bool& res);
	void			 RenameSound(LPCSTR p0, LPCSTR p1, EItemType type);

	void			 PlaySound(LPCSTR name, u32& size, float& time);
};

extern ECORE_API IM_SoundEditor imSoundEditor;

#endif
