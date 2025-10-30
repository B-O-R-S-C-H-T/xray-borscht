#include "stdafx.h"

#include "dxStatsRender.h"

#include <imgui.h>

#include "dxRenderDeviceRender.h"

void dxStatsRender::Copy(IStatsRender&_in)
{
	*this = *((dxStatsRender*)&_in);
}

void dxStatsRender::OutData1()
{
	ImGui::Text("VERT:        %d/%d",	RCache.stat.verts, RCache.stat.calls ? RCache.stat.verts / RCache.stat.calls : 0);
	ImGui::Text("POLY:        %d/%d",	RCache.stat.polys, RCache.stat.calls ? RCache.stat.polys / RCache.stat.calls : 0);
	ImGui::Text("DIP/DP:      %d",		RCache.stat.calls);
}

void dxStatsRender::OutData2()
{
#ifdef DEBUG
	ImGui::Text("SH/T/M/C:    %d/%d/%d/%d",	RCache.stat.states, RCache.stat.textures, RCache.stat.matrices, RCache.stat.constants);
	ImGui::Text("RT/PS/VS:    %d/%d/%d",	RCache.stat.target_rt, RCache.stat.ps, RCache.stat.vs);
	ImGui::Text("DCL/VB/IB:   %d/%d/%d",	RCache.stat.decl, RCache.stat.vb, RCache.stat.ib);
#endif
}

void dxStatsRender::OutData3()
{
	ImGui::Text("xforms:      %d",			RCache.stat.xforms);
}

void dxStatsRender::OutData4()
{
	ImGui::Text("static:        %3.1f/%d", RCache.stat.r.s_static.verts / 1024.f,       RCache.stat.r.s_static.dips);
	ImGui::Text("flora:         %3.1f/%d", RCache.stat.r.s_flora.verts / 1024.f,        RCache.stat.r.s_flora.dips);
	ImGui::Text("  flora_lods:  %3.1f/%d", RCache.stat.r.s_flora_lods.verts / 1024.f,   RCache.stat.r.s_flora_lods.dips);
	ImGui::Text("dynamic:       %3.1f/%d", RCache.stat.r.s_dynamic.verts / 1024.f,      RCache.stat.r.s_dynamic.dips);
	ImGui::Text("  dynamic_sw:  %3.1f/%d", RCache.stat.r.s_dynamic_sw.verts / 1024.f,   RCache.stat.r.s_dynamic_sw.dips);
	ImGui::Text("  dynamic_inst:%3.1f/%d", RCache.stat.r.s_dynamic_inst.verts / 1024.f, RCache.stat.r.s_dynamic_inst.dips);
	ImGui::Text("  dynamic_1B:  %3.1f/%d", RCache.stat.r.s_dynamic_1B.verts / 1024.f,   RCache.stat.r.s_dynamic_1B.dips);
	ImGui::Text("  dynamic_2B:  %3.1f/%d", RCache.stat.r.s_dynamic_2B.verts / 1024.f,   RCache.stat.r.s_dynamic_2B.dips);
	ImGui::Text("  dynamic_3B:  %3.1f/%d", RCache.stat.r.s_dynamic_3B.verts / 1024.f,   RCache.stat.r.s_dynamic_3B.dips);
	ImGui::Text("  dynamic_4B:  %3.1f/%d", RCache.stat.r.s_dynamic_4B.verts / 1024.f,   RCache.stat.r.s_dynamic_4B.dips);
	ImGui::Text("details:       %3.1f/%d", RCache.stat.r.s_details.verts / 1024.f,      RCache.stat.r.s_details.dips);
}

void dxStatsRender::GuardVerts()
{
	if (RCache.stat.verts > 500000)
		ImGui::Text("Verts     > 500k: %d", RCache.stat.verts);
}

void dxStatsRender::GuardDrawCalls()
{
	if (RCache.stat.calls > 1000)
		ImGui::Text("DIP/DP    > 1k:   %d", RCache.stat.calls);
}
