#ifndef IM_WINDOW_H
#define IM_WINDOW_H

class IM_PropertyTree;

class ECORE_API IM_Storage {
	xr_string m_file_name;
	xr_string m_sect_name;

	CInifile* ini;

	static xr_map<xr_string, CInifile*> s_file_inis;
	static xr_map<xr_string, int>		s_ref_counts;

public:
	IM_Storage(bool for_write, const xr_string& fn, const xr_string& sn);
	IM_Storage(bool for_write, const xr_string& sn);

	~IM_Storage();

public:
	int		  GetInt	(LPCSTR name, int def = 0);
	float	  GetFloat	(LPCSTR name, float def = 0.0f);
	bool	  GetBool	(LPCSTR name, bool def = false);
	xr_string GetString	(LPCSTR name, const xr_string& def = xr_string(""));

	void	  PutInt	(LPCSTR name, int value);
	void	  PutFloat	(LPCSTR name, float value);
	void	  PutBool	(LPCSTR name, bool value);
	void	  PutString	(LPCSTR name, const xr_string& value);

	CInifile*		GetIni()		{ return ini; }
	const CInifile* GetIni() const	{ return ini; }

	void	  Save();
};

class ECORE_API IM_Window {
public:
	virtual void OnAdd() {}
	virtual void OnRemove() {}

	virtual void Render() = 0;

protected:
	void BeginPersistedColumns		(const char* columns_id, const char* ui_section, float default_first_width = 280.0f);
	void EndPersistedColumns		(const char* columns_id, const char* ui_section);
	void RenderPersistedPropertyTree(IM_PropertyTree& tree, const char* ui_section, const char* key = "props_splitter_width", float default_width = 220.0f);
};

#endif
