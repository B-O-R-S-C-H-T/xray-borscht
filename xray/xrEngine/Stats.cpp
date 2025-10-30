#include "stdafx.h"

#include <imgui.h>

#include "xr_object.h"

#include "../Include/xrRender/DrawUtils.h"
#include "IGame_Persistent.h"
#include "ISpatial.h"
#include "render.h"

int		g_ErrorLineCount	= 15;
Flags32 g_stats_flags		= {0};

// stats
void __fastcall rp_Stats(void *p)
{
	((pureStats *)p)->OnStats();
}

class	optimizer	{
	float	average_	;
	BOOL	enabled_	;
public:
	optimizer	()		{
		average_	= 30.f;
//		enabled_	= TRUE;
//		disable		();
		// because Engine is not exist
		enabled_	= FALSE;
	}

	BOOL	enabled	()	{ return enabled_;	}
	void	enable	()	{ if (!enabled_)	{ Engine.External.tune_resume	();	enabled_=TRUE;	}}
	void	disable	()	{ if (enabled_)		{ Engine.External.tune_pause	();	enabled_=FALSE; }}
	void	update	(float value)	{
		if (value < average_*0.7f)	{
			// 25% deviation
			enable	();
		} else {
			disable	();
		};
		average_	= 0.99f*average_ + 0.01f*value;
	};
};
static	optimizer	vtune;

ENGINE_API CStats Statistic;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
BOOL			g_bDisableRedText	= FALSE;
CStats::CStats() : m_pRender(nullptr, [](IStatsRender* obj) { RenderFactory->DestroyStatsRender(obj); })
{
	fFPS				= 30.f;
	fRFPS				= 30.f;
	fTPS				= 0;
	fMem_calls			= 0;
	RenderDUMP_DT_Count = 0;
	Device.seqRender.Add		(this,REG_PRIORITY_LOW-1000);
}

CStats::~CStats()
{
	Device.seqRender.Remove		(this);
}

void _draw_cam_pos()
{
	ImGui::SetNextWindowPos(ImVec2(10, 600), ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(0.0f);
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
							 ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBackground;
	if (ImGui::Begin("Camera Position", nullptr, flags)) {
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		ImGui::Text("CAMERA POSITION: [%3.2f,%3.2f,%3.2f]", Device.vCameraPosition.x, Device.vCameraPosition.y, Device.vCameraPosition.z);
		ImGui::PopStyleColor();
	}
	ImGui::End();
}

void CStats::Show()
{
	{
		EngineTOTAL.FrameEnd		();
		Sheduler.FrameEnd			();
		UpdateClient.FrameEnd		();
		Physics.FrameEnd			();
		ph_collision.FrameEnd		();
		ph_core.FrameEnd			();
		Animation.FrameEnd			();
		AI_Think.FrameEnd			();
		AI_Range.FrameEnd			();
		AI_Path.FrameEnd			();
		AI_Node.FrameEnd			();
		AI_Vis.FrameEnd				();
		AI_Vis_Query.FrameEnd		();
		AI_Vis_RayTests.FrameEnd	();

		RenderTOTAL.FrameEnd		();
		RenderCALC.FrameEnd			();
		RenderCALC_HOM.FrameEnd		();
		RenderDUMP.FrameEnd			();
		RenderDUMP_RT.FrameEnd		();
		RenderDUMP_SKIN.FrameEnd	();
		RenderDUMP_Wait.FrameEnd	();
		RenderDUMP_Wait_S.FrameEnd	();
		RenderDUMP_HUD.FrameEnd		();
		RenderDUMP_Glows.FrameEnd	();
		RenderDUMP_Lights.FrameEnd	();
		RenderDUMP_WM.FrameEnd		();
		RenderDUMP_DT_VIS.FrameEnd	();
		RenderDUMP_DT_Render.FrameEnd();
		RenderDUMP_DT_Cache.FrameEnd();
		RenderDUMP_Pcalc.FrameEnd	();
		RenderDUMP_Scalc.FrameEnd	();
		RenderDUMP_Srender.FrameEnd	();

		Sound.FrameEnd				();
		Input.FrameEnd				();
		clRAY.FrameEnd				();
		clBOX.FrameEnd				();
		clFRUSTUM.FrameEnd			();

		netClient1.FrameEnd			();
		netClient2.FrameEnd			();
		netServer.FrameEnd			();

		TEST0.FrameEnd				();
		TEST1.FrameEnd				();
		TEST2.FrameEnd				();
		TEST3.FrameEnd				();

		g_SpatialSpace->stat_insert.FrameEnd		();
		g_SpatialSpace->stat_remove.FrameEnd		();
		g_SpatialSpacePhysic->stat_insert.FrameEnd	();
		g_SpatialSpacePhysic->stat_remove.FrameEnd	();
	}

	// calc FPS & TPS
	if (Device.fTimeDelta > EPS_S) {
		float fps  = 1.f / Device.fTimeDelta;

		float fOne = 0.3f;
		float fInv = 1.f - fOne;
		fFPS       = fInv * fFPS + fOne * fps;

		if (RenderTOTAL.result > EPS_S) {
			u32 rendered_polies = Device.m_pRender->GetCacheStatPolys();
			fTPS                = fInv * fTPS + fOne * float(rendered_polies) / (RenderTOTAL.result * 1000.f);
			fRFPS               = fInv * fRFPS + fOne * 1000.f / RenderTOTAL.result;
		}
	}
	{
		float mem_count = float(Memory.stat_calls);
		if (mem_count > fMem_calls)
			fMem_calls = mem_count;
		else
			fMem_calls = .9f * fMem_calls + .1f * mem_count;
		Memory.stat_calls = 0;
	}

	////////////////////////////////////////////////
	if (g_dedicated_server) return;
	////////////////////////////////////////////////
	ImGuiIO& io       = ImGui::GetIO();
	ImFont*  boldFont = io.Fonts->Fonts[1];

	if (vtune.enabled()) {
		ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		ImGui::SetNextWindowBgAlpha(0.5f);
		ImGuiWindowFlags vtune_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
		                               ImGuiWindowFlags_NoInputs;
		if (ImGui::Begin("VTune", nullptr, vtune_flags)) {
			ImGui::PushFont(boldFont);
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "--= tune =--");
			ImGui::PopFont();
		}
		ImGui::End();
	}

	if (psDeviceFlags.test(rsStatistic)) {
		bool  tmp_fake_value = true;
		float menu_height    = tmp_fake_value ? ImGui::GetFrameHeight() : 0.0f; // Здесь нужно проверять, открыт ли внутриигровой редактор...
		ImGui::SetNextWindowPos(ImVec2(0, menu_height), ImGuiCond_Once);        // ...и тогда статистика будет выводиться ниже строки меню.
		ImGui::SetNextWindowSize(ImVec2(400, io.DisplaySize.y - menu_height), ImGuiCond_Once);
		ImGui::SetNextWindowBgAlpha(0.0f);
		ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
		                         ImGuiWindowFlags_NoBackground;
		if (ImGui::Begin("Statistics", nullptr, flags)) {
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f)); // начало белого текста

			static float r_ps = 0;
			static float b_ps = 0;
			r_ps              = .99f * r_ps + .01f * (clRAY.count / clRAY.result);
			b_ps              = .99f * b_ps + .01f * (clBOX.count / clBOX.result);
			CSound_stats snd_stat;
			::Sound->statistic(&snd_stat, 0);

			ImGui::Text("FPS/RFPS:    %3.1f/%3.1f", fFPS, fRFPS);
			ImGui::Text("TPS:         %2.2f M", fTPS);
			m_pRender->OutData1();

#ifdef DEBUG
			ImGui::Separator();
			ImGui::Text("mapped:      %d", g_file_mapped_memory);
			ImGui::Separator();
			m_pRender->OutData2();
#endif
			m_pRender->OutData3();

#define PPP(a) (100.f * float(a) / float(EngineTOTAL.result))
			ImGui::Separator();
			ImGui::PushFont(boldFont);
			ImGui::Text("*** ENGINE:  %2.2fms", EngineTOTAL.result);
			ImGui::PopFont();
			ImGui::Text("Memory:      %2.2fa", fMem_calls);
			ImGui::Text("uClients:    %2.2fms, %2.1f%%, crow(%d)/active(%d)/total(%d)",
			            UpdateClient.result,
			            PPP(UpdateClient.result),
			            UpdateClient_crows,
			            UpdateClient_active,
			            UpdateClient_total);
			ImGui::Text("uSheduler:   %2.2fms, %2.1f%%", Sheduler.result, PPP(Sheduler.result));
			ImGui::Text("uSheduler_L: %2.2fms", fShedulerLoad);
			ImGui::Text("uParticles:  Qstart[%d] Qactive[%d] Qdestroy[%d]", Particles_starting, Particles_active, Particles_destroy);
			ImGui::Text("spInsert:    o[%.2fms, %2.1f%%], p[%.2fms, %2.1f%%]",
			            g_SpatialSpace->stat_insert.result,
			            PPP(g_SpatialSpace->stat_insert.result),
			            g_SpatialSpacePhysic->stat_insert.result,
			            PPP(g_SpatialSpacePhysic->stat_insert.result));
			ImGui::Text("spRemove:    o[%.2fms, %2.1f%%], p[%.2fms, %2.1f%%]",
			            g_SpatialSpace->stat_remove.result,
			            PPP(g_SpatialSpace->stat_remove.result),
			            g_SpatialSpacePhysic->stat_remove.result,
			            PPP(g_SpatialSpacePhysic->stat_remove.result));
			ImGui::Text("Physics:     %2.2fms, %2.1f%%", Physics.result, PPP(Physics.result));
			ImGui::Text("  collider:  %2.2fms", ph_collision.result);
			ImGui::Text("  solver:    %2.2fms, %d", ph_core.result, ph_core.count);
			ImGui::Text("aiThink:     %2.2fms, %d", AI_Think.result, AI_Think.count);
			ImGui::Text("  aiRange:   %2.2fms, %d", AI_Range.result, AI_Range.count);
			ImGui::Text("  aiPath:    %2.2fms, %d", AI_Path.result, AI_Path.count);
			ImGui::Text("  aiNode:    %2.2fms, %d", AI_Node.result, AI_Node.count);
			ImGui::Text("aiVision:    %2.2fms, %d", AI_Vis.result, AI_Vis.count);
			ImGui::Text("  Query:     %2.2fms", AI_Vis_Query.result);
			ImGui::Text("  RayCast:   %2.2fms", AI_Vis_RayTests.result);

#undef PPP
#define PPP(a) (100.f * float(a) / float(RenderTOTAL.result))
			ImGui::Separator();
			ImGui::PushFont(boldFont);
			ImGui::Text("*** RENDER:  %2.2fms", RenderTOTAL.result);
			ImGui::PopFont();
			ImGui::Text("R_CALC:      %2.2fms, %2.1f%%", RenderCALC.result, PPP(RenderCALC.result));
			ImGui::Text("  HOM:       %2.2fms, %d", RenderCALC_HOM.result, RenderCALC_HOM.count);
			ImGui::Text("  Skeletons: %2.2fms, %d", Animation.result, Animation.count);
			ImGui::Text("R_DUMP:      %2.2fms, %2.1f%%", RenderDUMP.result, PPP(RenderDUMP.result));
			ImGui::Text("  Wait-L:    %2.2fms", RenderDUMP_Wait.result);
			ImGui::Text("  Wait-S:    %2.2fms", RenderDUMP_Wait_S.result);
			ImGui::Text("  Skinning:  %2.2fms", RenderDUMP_SKIN.result);
			ImGui::Text("  DT_Vis/Cnt:%2.2fms", RenderDUMP_DT_VIS.result, RenderDUMP_DT_Count);
			ImGui::Text("  DT_Render: %2.2fms", RenderDUMP_DT_Render.result);
			ImGui::Text("  DT_Cache:  %2.2fms", RenderDUMP_DT_Cache.result);
			ImGui::Text("  Wallmarks: %2.2fms, %d/%d - %d", RenderDUMP_WM.result, RenderDUMP_WMS_Count, RenderDUMP_WMD_Count, RenderDUMP_WMT_Count);
			ImGui::Text("  Glows:     %2.2fms", RenderDUMP_Glows.result);
			ImGui::Text("  Lights:    %2.2fms, %d", RenderDUMP_Lights.result, RenderDUMP_Lights.count);
			ImGui::Text("  RT:        %2.2fms, %d", RenderDUMP_RT.result, RenderDUMP_RT.count);
			ImGui::Text("  HUD:       %2.2fms", RenderDUMP_HUD.result);
			ImGui::Text("  P_calc:    %2.2fms", RenderDUMP_Pcalc.result);
			ImGui::Text("  S_calc:    %2.2fms", RenderDUMP_Scalc.result);
			ImGui::Text("  S_render:  %2.2fms, %d", RenderDUMP_Srender.result, RenderDUMP_Srender.count);

			ImGui::Separator();
			ImGui::PushFont(boldFont);
			ImGui::Text("*** SOUND:   %2.2fms", Sound.result);
			ImGui::PopFont();
			ImGui::Text("  TGT/SIM/E: %d/%d/%d", snd_stat._rendered, snd_stat._simulated, snd_stat._events);
			ImGui::Text("  HIT/MISS:  %d/%d", snd_stat._cache_hits, snd_stat._cache_misses);
			ImGui::Separator();
			ImGui::Text("Input:       %2.2fms", Input.result);
			ImGui::Text("clRAY:       %2.2fms, %d, %2.0fK", clRAY.result, clRAY.count, r_ps);
			ImGui::Text("clBOX:       %2.2fms, %d, %2.0fK", clBOX.result, clBOX.count, b_ps);
			ImGui::Text("clFRUSTUM:   %2.2fms, %d", clFRUSTUM.result, clFRUSTUM.count);
			ImGui::Separator();
			ImGui::Text("netClientRecv:   %2.2fms, %d", netClient1.result, netClient1.count);
			ImGui::Text("netClientSend:   %2.2fms, %d", netClient2.result, netClient2.count);
			ImGui::Text("netServer:   %2.2fms, %d", netServer.result, netServer.count);
			ImGui::Separator();

			ImGui::Text("TEST 0:      %2.2fms, %d", TEST0.result, TEST0.count);
			ImGui::Text("TEST 1:      %2.2fms, %d", TEST1.result, TEST1.count);
			ImGui::Text("TEST 2:      %2.2fms, %d", TEST2.result, TEST2.count);
			ImGui::Text("TEST 3:      %2.2fms, %d", TEST3.result, TEST3.count);
#ifdef DEBUG_MEMORY_MANAGER
			ImGui::Separator();
			ImGui::Text("str: cmp[%3d], dock[%3d], qpc[%3d]", Memory.stat_strcmp, Memory.stat_strdock, CPU::qpc_counter);
			Memory.stat_strcmp  = 0;
			Memory.stat_strdock = 0;
			CPU::qpc_counter    = 0;
#else
			ImGui::Separator();
			ImGui::Text("qpc[%3d]", CPU::qpc_counter);
			CPU::qpc_counter = 0;
#endif

			ImGui::Separator();
			m_pRender->OutData4();

			ImGui::Separator();
			Render->Statistics();

			ImGui::Separator();
			g_pGamePersistent->Statistics();

			seqStats.Process(rp_Stats);

			ImGui::PopStyleColor(); // конец белого текста
		}
		ImGui::End();
	}

	if (psDeviceFlags.test(rsCameraPos)) {
		_draw_cam_pos();
	}

#ifdef DEBUG
	if (!g_bDisableRedText) {
		ImGui::PushFont(boldFont);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0625f, 0.0625f, 1.0f)); // красный
		if (fFPS < 30)
			ImGui::Text("FPS       < 30:   %3.1f", fFPS);

		m_pRender->GuardVerts();

		if (psDeviceFlags.test(rsStatistic)) {
			m_pRender->GuardDrawCalls();

			if (RenderDUMP_DT_Count > 1000)
				ImGui::Text("DT_count  > 1000: %u", RenderDUMP_DT_Count);
			ImGui::Separator();

			if (Sheduler.result > 3.f)
				ImGui::Text("Update     > 3ms:	%3.1f", Sheduler.result);
			if (UpdateClient.result > 3.f)
				ImGui::Text("UpdateCL   > 3ms: %3.1f", UpdateClient.result);
			if (Physics.result > 5.f)
				ImGui::Text("Physics    > 5ms: %3.1f", Physics.result);
		}
		ImGui::PopStyleColor();
		ImGui::PopFont();
	}

	if (!g_bDisableRedText && errors.size()) {
		ImGui::SetNextWindowPos(ImVec2(200, 0), ImGuiCond_Once);
		ImGui::SetNextWindowBgAlpha(0.0f);
		ImGuiWindowFlags err_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground;
		ImGui::Begin("Errors", nullptr, err_flags);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0625f, 0.0625f, 0.75f)); // красный с альфой
			for (u32 it = (u32)_max(int(0), (int)errors.size() - g_ErrorLineCount); it < errors.size(); it++)
				ImGui::Text("%s", errors[it].c_str());
			ImGui::PopStyleColor();
		ImGui::End();
	}
#endif

	{
		EngineTOTAL.FrameStart		();
		Sheduler.FrameStart			();
		UpdateClient.FrameStart		();
		Physics.FrameStart			();
		ph_collision.FrameStart		();
		ph_core.FrameStart			();
		Animation.FrameStart		();
		AI_Think.FrameStart			();
		AI_Range.FrameStart			();
		AI_Path.FrameStart			();
		AI_Node.FrameStart			();
		AI_Vis.FrameStart			();
		AI_Vis_Query.FrameStart		();
		AI_Vis_RayTests.FrameStart	();

		RenderTOTAL.FrameStart		();
		RenderCALC.FrameStart		();
		RenderCALC_HOM.FrameStart	();
		RenderDUMP.FrameStart		();
		RenderDUMP_RT.FrameStart	();
		RenderDUMP_SKIN.FrameStart	();
		RenderDUMP_Wait.FrameStart	();
		RenderDUMP_Wait_S.FrameStart();
		RenderDUMP_HUD.FrameStart	();
		RenderDUMP_Glows.FrameStart	();
		RenderDUMP_Lights.FrameStart();
		RenderDUMP_WM.FrameStart	();
		RenderDUMP_DT_VIS.FrameStart();
		RenderDUMP_DT_Render.FrameStart();
		RenderDUMP_DT_Cache.FrameStart();
		RenderDUMP_Pcalc.FrameStart	();
		RenderDUMP_Scalc.FrameStart	();
		RenderDUMP_Srender.FrameStart();

		Sound.FrameStart		();
		Input.FrameStart		();
		clRAY.FrameStart		();
		clBOX.FrameStart		();
		clFRUSTUM.FrameStart	();

		netClient1.FrameStart	();
		netClient2.FrameStart	();
		netServer.FrameStart	();

		TEST0.FrameStart();
		TEST1.FrameStart();
		TEST2.FrameStart();
		TEST3.FrameStart();

		g_SpatialSpace->stat_insert.FrameStart();
		g_SpatialSpace->stat_remove.FrameStart();

		g_SpatialSpacePhysic->stat_insert.FrameStart();
		g_SpatialSpacePhysic->stat_remove.FrameStart();
	}
	dwSND_Played = dwSND_Allocated = 0;
	Particles_starting = Particles_active = Particles_destroy = 0;
}

void _LogCallback(LPCSTR string)
{
	if (string && '!'==string[0] && ' '==string[1])
		Statistic.errors.push_back	(shared_str(string));
}

void CStats::OnDeviceCreate()
{
	g_bDisableRedText				= strstr(Core.Params,"-xclsx")?TRUE:FALSE;
#ifdef DEBUG
	if (!g_bDisableRedText)			SetLogCB	(_LogCallback);
#endif
	m_pRender.reset(RenderFactory->CreateStatsRender());
}

void CStats::OnDeviceDestroy()
{
	m_pRender.reset();
	SetLogCB(0);
}

void CStats::OnRender()
{
#ifdef DEBUG
	if (g_stats_flags.is(st_sound)) {
		CSound_stats_ext				snd_stat_ext;
		::Sound->statistic				(0,&snd_stat_ext);
		CSound_stats_ext::item_vec_it	_I = snd_stat_ext.items.begin();
		CSound_stats_ext::item_vec_it	_E = snd_stat_ext.items.end();
		for (; _I != _E; _I++) {
			const CSound_stats_ext::SItem& item = *_I;
			if (item._3D) {
				DU->DrawCross			(item.params.position, 0.5f, 0xFF0000FF, true );
				if (g_stats_flags.is(st_sound_min_dist))
					DU->DrawSphere		(Fidentity, item.params.position, item.params.min_distance, 0x400000FF,	0xFF0000FF, true, true);
				if (g_stats_flags.is(st_sound_max_dist))
					DU->DrawSphere		(Fidentity, item.params.position, item.params.max_distance, 0x4000FF00,	0xFF008000, true, true);
				xr_string out_txt		= (g_stats_flags.is(st_sound_info_name))?item.name.c_str():"";
				if (item.game_object){
					if (g_stats_flags.is(st_sound_ai_dist))
						DU->DrawSphere	(Fidentity, item.params.position, item.params.max_ai_distance, 0x80FF0000,0xFF800000,true,true);
					if (g_stats_flags.is(st_sound_info_object)){
						out_txt			+= "  (";
						out_txt			+= item.game_object->cNameSect().c_str();
						out_txt			+= ")";
					}
				}
				if (g_stats_flags.is_any(st_sound_info_name | st_sound_info_object))
					DU->OutText			(item.params.position, out_txt.c_str(),0xFFFFFFFF,0xFF000000);
			}
		}
	}
#endif
}
