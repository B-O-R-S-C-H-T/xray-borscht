#include "stdafx.h"
#pragma hdrstop

#include "IM_SoundEditor.h"

#include "imgui.h"
#include "addons/implot/implot.h"
#include "..\Editor\ui_main.h"
#include "..\..\..\xrCore\ELocatorAPI.h"
#include "..\Editor\SoundManager.h"
#include "..\..\..\xrSound\soundrender_source.h"

ECORE_API IM_SoundEditor imSoundEditor;

IM_SoundEditor::IM_SoundEditor() : m_open(false), m_title("Sound Editor"), m_items('\\', true), m_Flags{0}, bAutoPlay(TRUE) {
	m_items.OnItemsFocused.bind(this, &IM_SoundEditor::OnItemsFocused);
	m_items.OnItemRemove.bind(this, &IM_SoundEditor::RemoveSound);
	m_items.OnItemRename.bind(this, &IM_SoundEditor::RenameSound);
}

void IM_SoundEditor::Open() {
	if (!m_open) {
		UI->AddIMWindow(this);

		m_open = true;
		InitItemsList();

		modif_map.clear	();
		m_Flags.zero	();
		if (!FS.can_write_to_alias(_sounds_)){
			Log				("#!You don't have permisions to modify sounds.");
			m_Flags.set		(flReadOnly,TRUE);
			bAutoPlay		= TRUE;
		}
	}
}

void IM_SoundEditor::Close() {
	UI->RemoveIMWindow(this);
	m_open = false;
}

void IM_SoundEditor::OnRemove() {
	modif_map.clear();
	DestroyUsedTHM();
	m_items.Clear();
	m_props.Clear();
	m_Snd.destroy();
}

void IM_SoundEditor::Render() {
	if (!m_open) {
		Close();
		return;
	}

	ImGui::SetNextWindowSize(ImVec2(640, 580), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSizeConstraints(ImVec2(350, 400), ImVec2(FLT_MAX, FLT_MAX));

	if (!ImGui::Begin(m_title.c_str(), &m_open, ImGuiWindowFlags_NoCollapse)) {
		ImGui::End();
		return;
	}

	ImGui::Columns(2, "sound_editor_cols", true);

	ImGui::BeginChild("##ItemList", ImVec2(0, -40), true);
	m_items.Render();
	ImGui::EndChild();

	ImGui::NextColumn();

	ImGui::BeginChild("##ItemProps", ImVec2(0, -40), true);
	m_props.Render();
	ImGui::EndChild();

	ImGui::Columns(1);

	if (!m_Flags.is(flReadOnly)) {
		ImGui::Indent(ImGui::GetContentRegionAvail().x - 172);

		if (ImGui::Button("OK##sound_ok", ImVec2(80, 0))) {
			UpdateLib();
			m_open = false;
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel##sound_cancel", ImVec2(80, 0))) {
			m_open = false;
		}
	}

	ImGui::End();
}

void IM_SoundEditor::InitItemsList() {
	FS_FileSet sound_map;
	SndLib->GetSounds(sound_map, TRUE);

	ListItemsVec items;

	// fill items
	FS_FileSetIt it = sound_map.begin();
	FS_FileSetIt _E = sound_map.end();
	for (; it != _E; it++)
		LHelper().CreateItem(items, it->name.c_str(), 0);

	m_items.AssignItems(items, false);
}

ESoundThumbnail* IM_SoundEditor::FindUsedTHM(LPCSTR name) {
	for (THMIt it = m_THM_Used.begin(); it != m_THM_Used.end(); it++)
		if (0 == strcmp((*it)->SrcName(), name))
			return *it;
	return 0;
}

void IM_SoundEditor::SaveUsedTHM() {
	int m_age		= time(NULL);
	for (THMIt t_it = m_THM_Used.begin(); t_it != m_THM_Used.end(); t_it++)
		(*t_it)->Save(m_age, 0);
}

void IM_SoundEditor::DestroyUsedTHM() {
	for (THMIt it = m_THM_Used.begin(); it != m_THM_Used.end(); it++)
		xr_delete(*it);
	m_THM_Used.clear();
}

void IM_SoundEditor::AppendModif(LPCSTR nm) {
	FS_File			dest;
	string_path		fname;
	FS.update_path	(fname, _sounds_, ChangeFileExt(nm, ".wav").c_str());
	BOOL bFind		= ((ELocatorAPI&)FS).file_find(fname, dest); R_ASSERT(bFind);
	modif_map.insert(dest);
}

void IM_SoundEditor::RegisterModifiedTHM() {
	if (m_props.IsModified()) {
		for (THMIt t_it = m_THM_Current.begin(); t_it != m_THM_Current.end(); t_it++) {
//.            (*t_it)->Save	(0,0);
			AppendModif((*t_it)->SrcName());
		}
	}
}

void IM_SoundEditor::UpdateLib() {
	RegisterModifiedTHM();
	SaveUsedTHM();
	// save game sounds
	if (modif_map.size()) {
		//LockForm			();
		SndLib->SynchronizeSounds(true, true, true, &modif_map, 0);
//		SndLib->ChangeFileAgeTo		(&modif_map,time(NULL));
		//UnlockForm			();
		SndLib->RefreshSounds(false);
		modif_map.clear();
	}
}

void __stdcall IM_SoundEditor::OnItemsFocused(ListItemsVec& items) {
	PropItemVec props;
	RegisterModifiedTHM();
	m_Snd.destroy();
	m_THM_Current.clear();

	#ifdef DEBUG
	if (!items.empty())		ELog.Msg(mtInformation, "Selected %d sound(s)", items.size());
	#endif

	if (!items.empty()) {
		for (ListItem* prop : items) {
			if (prop) {
				ESoundThumbnail* thm = FindUsedTHM(prop->Key());
				if (!thm)
					m_THM_Used.push_back(thm = xr_new<ESoundThumbnail>(prop->Key()));

				m_THM_Current.push_back(thm);
				thm->FillProp(props);
			}
		}
	}

	if (m_THM_Current.size() == 1) {
		ESoundThumbnail* thm  = m_THM_Current.back();
		u32				 size = 0;
		float			 time = 0;
		PlaySound(thm->SrcName(), size, time);

		PHelper().CreatePlot(props, "Attenuation", fastdelegate::MakeDelegate(this, &IM_SoundEditor::OnPlotDraw), "");

		ButtonValue* B = PHelper().CreateButton(props, "Auto Attenuation", "Fit from Min,Fit from Max", ButtonValue::flFirstOnly);
		B->OnBtnClickEvent.bind(this, &IM_SoundEditor::OnAttClick);

		PHelper().CreateCaption(props, "File Size", shared_str().sprintf("%.2f Kb", float(size) / 1024.f));
		PHelper().CreateCaption(props, "Duration", shared_str().sprintf("%.2f sec", time));

		if (!m_Flags.is(flReadOnly)) {
			B = PHelper().CreateButton(props, "Playback", "Play,Stop", ButtonValue::flFirstOnly);
			B->OnBtnClickEvent.bind(this, &IM_SoundEditor::OnControlClick);
		}
	}

	if (!m_Flags.is(flReadOnly) && m_THM_Current.size()) {
		ButtonValue* B = PHelper().CreateButton(props, "Auto-Play", bAutoPlay ? "on" : "off", ButtonValue::flFirstOnly);
		B->OnBtnClickEvent.bind(this, &IM_SoundEditor::OnControl2Click);
	}

	if (!m_Flags.is(flReadOnly) && m_THM_Current.size()) {
		ButtonValue* B = PHelper().CreateButton(props, "MANAGE", "Sync to GAMEDATA", ButtonValue::flFirstOnly);
		B->OnBtnClickEvent.bind(this, &IM_SoundEditor::OnSyncCurrentClick);
	}

	m_props.Clear();
	m_props.AssignItems(props, false, false);
}

void __stdcall IM_SoundEditor::OnControlClick(ButtonValue* V, bool& bModif, bool& bSafe) {
	switch (V->btn_num) {
		case 0: m_Snd.play(0, sm_2D);	break;
		case 1: m_Snd.stop();			break;
		case 2: {
			bAutoPlay			 = !bAutoPlay;
			V->value[V->btn_num] = shared_str().sprintf("Auto (%s)", bAutoPlay ? "on" : "off");
		} break;
	}
	bModif = false;
}

void __stdcall IM_SoundEditor::OnControl2Click(ButtonValue* V, bool& bModif, bool& bSafe) {
	switch (V->btn_num) {
		case 0: {
			bAutoPlay			 = !bAutoPlay;
			V->value[V->btn_num] = bAutoPlay ? "on" : "off";
		} break;
	}
	bModif = false;
}

void __stdcall IM_SoundEditor::OnSyncCurrentClick(ButtonValue* V, bool& bModif, bool& bSafe) {
	if (m_Snd._handle()) {
		m_Snd.stop();
		m_Snd.destroy();
	}

	THMIt it   = m_THM_Current.begin();
	THMIt it_e = m_THM_Current.end();

	for (; it != it_e; ++it) {
		ESoundThumbnail* pTHM = *it;

		string_path		src_name, game_name;
		FS.update_path	(src_name, _sounds_, pTHM->SrcName());
		strconcat		(sizeof(src_name), src_name, src_name, ".wav");

		FS.update_path	(game_name, _game_sounds_, pTHM->SrcName());
		strconcat		(sizeof(game_name), game_name, game_name, ".ogg");

		ELog.Msg		(mtInformation, "Synchronizing [%s]", game_name);

		SndLib->MakeGameSound(pTHM, src_name, game_name);
	}

	ELog.Msg(mtInformation, "All sounds have been successfully synchronized.");
}

void __stdcall IM_SoundEditor::OnAttClick(ButtonValue* V, bool& bModif, bool& bSafe) {
	bModif				 = true;
	ESoundThumbnail* thm = m_THM_Current.back();
	switch (V->btn_num) {
		case 0: {
			float dist = thm->MinDist() / (0.01f * psSoundRolloff);
			thm->SetMaxDist(dist + 0.1f * dist);
		} break;
		case 1: {
			float dist = psSoundRolloff * (thm->MaxDist() - (0.1f / 1.1f) * thm->MaxDist()) * 0.01f;
			thm->SetMinDist(dist);
		} break;
	}
}

void __stdcall IM_SoundEditor::OnPlotDraw(PlotValue* sender, void*) {
	if (m_THM_Current.empty())
		return;

	ESoundThumbnail* thm	= m_THM_Current.back();

	float maxD	 = thm->MaxDist();
	float minD	 = thm->MinDist();
	float maxAID = thm->MaxAIDist();

	if (maxD <= 0.0f)
		maxD = 1.0f;

	if (ImPlot::BeginPlot("##AttenuationPlot", ImVec2(-1, 220), ImPlotFlags_CanvasOnly | ImPlotFlags_NoInputs | ImPlotFlags_NoFrame | ImPlotFlags_NoMenus)) {
		ImPlot::SetupAxis(ImAxis_X1, "distance", ImPlotAxisFlags_NoLabel);
		ImPlot::SetupAxis(ImAxis_Y1, "volume", ImPlotAxisFlags_NoLabel);

		ImPlot::SetupAxisLimits(ImAxis_X1, 0.0, maxD * 1.1f, ImPlotCond_Always);
		ImPlot::SetupAxisLimits(ImAxis_Y1, 0.0, 1.1f, ImPlotCond_Always);

		const int points = 300;
		std::vector<double> x(points), y(points);

		for (int i = 0; i < points; ++i) {
			float dist = maxD * (i / float(points - 1));
			float vol  = (dist > 0.0f) ? minD / (psSoundRolloff * dist) : 1.0f;
			vol		   = std::clamp(vol, 0.0f, 1.0f);

			x[i] = dist;
			y[i] = vol;
		}

		ImPlot::PlotLine("Attenuation", x.data(), y.data(), points, ImPlotSpec());

		if (maxAID > 0.0f && maxAID <= maxD * 1.1f) {
			ImPlot::PlotInfLines("Max AI Distance", &maxAID, 1, ImPlotSpec());
		}

		ImPlot::EndPlot();
	}
}

void __stdcall IM_SoundEditor::RemoveSound(LPCSTR fname, EItemType type, bool& res) {
	// delete it from modif map
	FS_FileSetIt it = modif_map.find(FS_File(fname));
	if (it != modif_map.end()) modif_map.erase(it);
	// remove sound source
	res = SndLib->RemoveSound(fname, type);
}

void __stdcall IM_SoundEditor::RenameSound(LPCSTR p0, LPCSTR p1, EItemType type) {
	// rename sound source
	SndLib->RenameSound(p0, p1, type);
	// delete old from map
	FS_FileSetIt old_it = modif_map.find(FS_File(p0));
	if (old_it != modif_map.end()) {
		modif_map.erase	(old_it);
		AppendModif		(p1);
	}
}

void IM_SoundEditor::PlaySound(LPCSTR name, u32& size, float& time) {
	string_path fname;
	FS.update_path(fname, _game_sounds_, ChangeFileExt(name, ".ogg").c_str());
	FS_File F;
	if (((ELocatorAPI&)FS).file_find(fname, F)) {
		m_Snd.create		(name, st_Effect, sg_Undefined);
		m_Snd.play			(0, sm_2D);
		CSoundRender_Source* src = (CSoundRender_Source*)m_Snd._handle(); VERIFY(src);
		size				= F.size;
		time				= src->length_sec();
		if (!bAutoPlay)		m_Snd.stop();
	} else {
		ELog.Msg(mtError, "The sound '%s' is currently not synchronized to GAMEDATA!", name);
	}
}
