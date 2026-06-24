#include "stdafx.h"
#pragma hdrstop

#include "IM_Stats.h"

#include <imgui.h>

#include "../ECore/Editor/device.h"

void IM_Stats::OnAdd() {
	IM_Storage& storage = EPrefs->GetSectionStorage("im_stats");

	m_saved_x = storage.GetFloat("pos_x", 60.0f);
	m_saved_y = storage.GetFloat("pos_y", 100.0f);
	m_saved_w = storage.GetFloat("size_w", 300.0f);
	m_saved_h = storage.GetFloat("size_h", 200.0f);

	m_last_x  = m_saved_x;
	m_last_y  = m_saved_y;
	m_last_w  = m_saved_w;
	m_last_h  = m_saved_h;
}

void IM_Stats::OnRemove() {
	IM_Storage& storage = EPrefs->GetSectionStorage("im_stats");

	storage.PutFloat("pos_x", m_saved_x);
	storage.PutFloat("pos_y", m_saved_y);
	storage.PutFloat("size_w", m_saved_w);
	storage.PutFloat("size_h", m_saved_h);

	storage.Save();
}

void IM_Stats::Render() {
	if (!psDeviceFlags.is(rsStatistic))
		return;

	ImGui::SetNextWindowPos(ImVec2(m_saved_x, m_saved_y), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(m_saved_w, m_saved_h), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowBgAlpha(0.0f);

	constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground |
									   ImGuiWindowFlags_NoSavedSettings;

	if (ImGui::Begin("Statistics", nullptr, flags)) {
		ImVec2 curr_pos	 = ImGui::GetWindowPos();
		ImVec2 curr_size = ImGui::GetWindowSize();

		if (std::fabs(curr_pos.x - m_last_x) > 0.5f || std::fabs(curr_pos.y - m_last_y) > 0.5f) {
			m_saved_x = curr_pos.x;
			m_saved_y = curr_pos.y;
			m_last_x  = curr_pos.x;
			m_last_y  = curr_pos.y;
		}
		if (std::fabs(curr_size.x - m_last_w) > 0.5f || std::fabs(curr_size.y - m_last_h) > 0.5f) {
			m_saved_w = curr_size.x;
			m_saved_h = curr_size.y;
			m_last_w  = curr_size.x;
			m_last_h  = curr_size.y;
		}

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

		CBackend::_stats& DPS = RCache.stat;

		ImGui::Text("FPS/RFPS:     %3.1f/%3.1f", Device.Statistic->fFPS, Device.Statistic->fRFPS);
//		ImGui::Text("TPS:          %2.2f M", Device.Statistic->fTPS);
		ImGui::Text("VERTS:         %d", DPS.verts);
		ImGui::Text("TRIS:         %d", DPS.polys);
		ImGui::Text("DIP/DP:       %d", DPS.calls);
//		ImGui::Text("SH/T/M/C:     %d/%d/%d/%d",
//					Device.Statistic->dwShader_Codes,
//					Device.Statistic->dwShader_Textures,
//					Device.Statistic->dwShader_Matrices,
//					Device.Statistic->dwShader_Constants);
		ImGui::Text("LIGHT S/T:    %d/%d", Device.Statistic->dwLightInScene, Device.Statistic->dwTotalLight);
//		ImGui::Text("Skeletons:    %2.2fms, %d", Device.Statistic->Animation.result, Device.Statistic->Animation.count);
//		ImGui::Text("Skinning:     %2.2fms", Device.Statistic->RenderDUMP_SKIN.result);
/*
		ImGui::Separator();

		ImGui::Text("Input:        %2.2fms", Device.Statistic->Input.result);
		ImGui::Text("clRAY:        %2.2fms, %d", Device.Statistic->clRAY.result, Device.Statistic->clRAY.count);
		ImGui::Text("clBOX:        %2.2fms, %d", Device.Statistic->clBOX.result, Device.Statistic->clBOX.count);
		ImGui::Text("clFRUSTUM:    %2.2fms, %d", Device.Statistic->clFRUSTUM.result, Device.Statistic->clFRUSTUM.count);

		ImGui::Separator();

		ImGui::Text("RT:           %2.2fms, %d", Device.Statistic->RenderDUMP_RT.result, Device.Statistic->RenderDUMP_RT.count);
		ImGui::Text("DT_Vis:       %2.2fms", Device.Statistic->RenderDUMP_DT_VIS.result);
		ImGui::Text("DT_Render:    %2.2fms", Device.Statistic->RenderDUMP_DT_Render.result);
		ImGui::Text("DT_Cache:     %2.2fms", Device.Statistic->RenderDUMP_DT_Cache.result);

		ImGui::Separator();

		ImGui::Text("TEST 0:       %2.2fms, %d", Device.Statistic->TEST0.result, Device.Statistic->TEST0.count);
		ImGui::Text("TEST 1:       %2.2fms, %d", Device.Statistic->TEST1.result, Device.Statistic->TEST1.count);
		ImGui::Text("TEST 2:       %2.2fms, %d", Device.Statistic->TEST2.result, Device.Statistic->TEST2.count);
		ImGui::Text("TEST 3:       %2.2fms, %d", Device.Statistic->TEST3.result, Device.Statistic->TEST3.count);
*/
		ImGui::PopStyleColor();
	}
	ImGui::End();

	{
		Device.Statistic->Animation.FrameStart();
		Device.Statistic->RenderTOTAL.FrameStart();

		Device.Statistic->Input.FrameStart();
		Device.Statistic->clRAY.FrameStart();
		Device.Statistic->clBOX.FrameStart();
		Device.Statistic->clFRUSTUM.FrameStart();

		Device.Statistic->RenderDUMP_SKIN.FrameStart();
		Device.Statistic->RenderDUMP_RT.FrameStart();

		Device.Statistic->RenderDUMP_DT_VIS.FrameStart();
		Device.Statistic->RenderDUMP_DT_Render.FrameStart();
		Device.Statistic->RenderDUMP_DT_Cache.FrameStart();

		Device.Statistic->TEST0.FrameStart();
		Device.Statistic->TEST1.FrameStart();
		Device.Statistic->TEST2.FrameStart();
		Device.Statistic->TEST3.FrameStart();
	}

	CBackend::_stats& DPS				  = RCache.stat;

	Device.Statistic->dwShader_Codes	  = 0;
	Device.Statistic->dwShader_Textures	  = 0;
	Device.Statistic->dwShader_Matrices	  = 0;
	Device.Statistic->dwShader_Constants  = 0;

	Device.Statistic->dwSND_Played		  = 0;
	Device.Statistic->dwSND_Allocated	  = 0;

	Device.Statistic->dwTotalLight		  = 0;
	Device.Statistic->dwLightInScene	  = 0;

	Device.Statistic->RenderDUMP_DT_Count = 0;

	DPS.polys = DPS.verts = DPS.calls = DPS.vs = 0;

	Device.Statistic->dwLevelSelFaceCount	   = 0;
	Device.Statistic->dwLevelSelVertexCount	   = 0;
}

IM_Stats g_imStats;
