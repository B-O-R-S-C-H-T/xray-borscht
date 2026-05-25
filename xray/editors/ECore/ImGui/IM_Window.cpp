#include "stdafx.h"
#pragma hdrstop

#include "IM_Window.h"

/*
 *   [IM_Example]
 *        |
 *        |  OnRemove() / OnAdd()
 *        v
 *   IM_Storage storage(true/false, "level.ini", "IM_Example");
 *        |
 *        |  PutBool/PutFloat/Get*...
 *        v
 *   shared CInifile <-- refcount++
 *        |
 *        v
 *   ~IM_Storage	 --> refcount--
 *        |
 *        +-- if refcount == 0
 *              |
 *              v
 *        CInifile::Destroy() --> save_as("level.ini")
 *              |
 *              v
 *          level.ini
 */

xr_map<xr_string, CInifile*> IM_Storage::s_file_inis;
xr_map<xr_string, int>		 IM_Storage::s_ref_counts;

IM_Storage::IM_Storage(bool for_write, const xr_string& fn, const xr_string& sn) : m_file_name(fn), m_sect_name(sn) {
	// CInifile automatically converts section names to lowercase while writing
	// but while reading it uses case sensitive comarison :-\
    // so convert section name to lowercase anyway
	for (size_t i = 0; i < m_sect_name.size(); ++i)
		m_sect_name[i] = tolower(m_sect_name[i]);

	auto it = s_file_inis.find(m_file_name);
	if (it != s_file_inis.end()) {
		ini = it->second;
		++s_ref_counts[m_file_name];
	} else {
		ini						  = CInifile::Create(m_file_name.c_str(), FALSE);
		s_file_inis[m_file_name]  = ini;
		s_ref_counts[m_file_name] = 1;
	}

	if (for_write)
		ini->set_override_names(TRUE);
}

IM_Storage::IM_Storage(bool for_write, const xr_string& sn) : IM_Storage(for_write, xr_string("imgui.ini"), sn) {}

IM_Storage::~IM_Storage() {
	auto it = s_ref_counts.find(m_file_name);
	if (it != s_ref_counts.end()) {
		--it->second;
		if (it->second == 0) {
			CInifile::Destroy(ini);
			s_file_inis.erase(m_file_name);
			s_ref_counts.erase(it);
		}
	} else {
		CInifile::Destroy(ini);
	}
}

int IM_Storage::GetInt(LPCSTR name, int def) {
	if (ini->line_exist(m_sect_name.c_str(), name))
		return ini->r_s32(m_sect_name.c_str(), name);
	else
		return def;
}

void IM_Storage::PutInt(LPCSTR name, int value) {
	ini->remove_line(m_sect_name.c_str(), name);
	ini->w_s32(m_sect_name.c_str(), name, value);
}

float IM_Storage::GetFloat(LPCSTR name, float def) {
	if (ini->line_exist(m_sect_name.c_str(), name))
		return ini->r_float(m_sect_name.c_str(), name);
	else
		return def;
}

void IM_Storage::PutFloat(LPCSTR name, float value) {
	ini->remove_line(m_sect_name.c_str(), name);
	ini->w_float(m_sect_name.c_str(), name, value);
}

bool IM_Storage::GetBool(LPCSTR name, bool def) {
	if (ini->line_exist(m_sect_name.c_str(), name))
		return ini->r_bool(m_sect_name.c_str(), name);
	else
		return def;
}

void IM_Storage::PutBool(LPCSTR name, bool value) {
	ini->remove_line(m_sect_name.c_str(), name);
	ini->w_bool(m_sect_name.c_str(), name, value);
}

xr_string IM_Storage::GetString(LPCSTR name, const xr_string& def) {
	if (ini->line_exist(m_sect_name.c_str(), name))
		return ini->r_string_wb(m_sect_name.c_str(), name).c_str();
	else
		return def;
}

void IM_Storage::PutString(LPCSTR name, const xr_string& value) {
	xr_string s = xr_string("\"") + value + "\"";
	ini->remove_line(m_sect_name.c_str(), name);
	ini->w_string(m_sect_name.c_str(), name, s.c_str());
}
