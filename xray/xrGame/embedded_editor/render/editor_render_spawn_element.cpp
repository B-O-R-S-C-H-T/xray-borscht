#include "stdafx.h"

#include "editor_render_spawn_element.h"

#include "../../ai_space.h"
#include "../../alife_object_registry.h"
#include "../../alife_simulator.h"
#include "../../game_graph.h"
#include "../../level_graph.h"
#include "../Include/xrRender/UIRender.h"
#include "../Include/xrRender/UIShader.h"
#include "../xrEngine/xr_collide_form.h"
#include "ui_defs.h"
#include "xrServer_Objects_ALife.h"
#include "xrServer_Objects_ALife_Monsters.h"
#include "../embedded_editor_main.h"

bool isRenderSpawnElement = true;

namespace SpawnFilter {
	bool showLevelChangers		= false;
	bool showAnomalousZones		= false;
	bool showSmartZones			= false;
	bool showTeamBaseZones		= false;
	bool showSmartCovers		= false;
	bool showClimableObjects	= false;
	bool showGenericRestrictors = false;
} // namespace SpawnFilter

void renderSpaceObjectShape(CSE_Abstract* obj, const xr_vector<CShapeData::shape_def>& shapes_vec, u32 transpColor) {
	const u32 edgeColor = color_rgba(32, 32, 32, 255);

	Fvector zero = {0.f, 0.f, 0.f};

	Fmatrix transform{};
	if (smart_cast<CSE_ALifeObjectClimable*>(obj)) {
		transform.setXYZ(obj->angle());
	} else {
		Fquaternion Q{};
		Q.rotationYawPitchRoll(obj->angle());
		transform.mk_xform(Q, obj->position());
	}
	transform.c = obj->position();

	UIRender->CacheSetCullMode(IUIRender::cmNONE);

	for (const auto& shape : shapes_vec) {
		switch (shape.type) {
			case CShapeData::cfSphere:
			{
				const Fsphere& S = shape.data.sphere;
				Fmatrix B{};
				B.scale(S.R, S.R, S.R);
				B.translate_over(S.P);
				B.mulA_43(transform);
				UIRender->CacheSetXformWorld(B);
				DU->DrawCross(zero, 1.f, edgeColor, false);
				DU->DrawIdentSphere(true, true, transpColor, edgeColor);
			} break;
			case CShapeData::cfBox:
			{
				Fmatrix B = shape.data.box;
				B.mulA_43(transform);
				UIRender->CacheSetXformWorld(B);
				DU->DrawIdentBox(true, true, transpColor, edgeColor);
			} break;
		}
	}
	UIRender->CacheSetCullMode(IUIRender::cmCCW);
}

void renderSpawnElements() {
	auto alife = ai().get_alife();
	if (!alife)
		return;

	auto& registry = alife->objects();
	for (auto& el : registry.objects()) {
		if (ai().game_graph().vertex(el.second->m_tGraphID)->level_id() != ai().level_graph().level_id()) continue;

		CSE_Abstract* obj = el.second;

		auto shape = smart_cast<CSE_Shape*>(obj);
		if (!shape || shape->shapes.empty()) continue;

		if (smart_cast<CSE_ALifeLevelChanger*>(obj)		&& !SpawnFilter::showLevelChangers)		continue;
		if (smart_cast<CSE_ALifeAnomalousZone*>(obj)	&& !SpawnFilter::showAnomalousZones)	continue;
		if (smart_cast<CSE_ALifeSmartZone*>(obj)		&& !SpawnFilter::showSmartZones)		continue;
		if (smart_cast<CSE_ALifeTeamBaseZone*>(obj)		&& !SpawnFilter::showTeamBaseZones)		continue;
		if (smart_cast<CSE_SmartCover*>(obj)			&& !SpawnFilter::showSmartCovers)		continue;
		if (smart_cast<CSE_ALifeObjectClimable*>(obj)	&& !SpawnFilter::showClimableObjects)	continue;

		bool isGeneric = !smart_cast<CSE_ALifeLevelChanger*>(obj) &&
						 !smart_cast<CSE_ALifeAnomalousZone*>(obj) &&
						 !smart_cast<CSE_ALifeSmartZone*>(obj) &&
						 !smart_cast<CSE_ALifeTeamBaseZone*>(obj) &&
						 !smart_cast<CSE_SmartCover*>(obj) &&
						 !smart_cast<CSE_ALifeObjectClimable*>(obj);

		if (isGeneric && !SpawnFilter::showGenericRestrictors) continue;

		u32 color = color_rgba(80, 255, 80, 120);

		if		(smart_cast<CSE_ALifeLevelChanger*>(obj))	color = color_rgba(200, 150, 110, 100);
		else if (smart_cast<CSE_ALifeAnomalousZone*>(obj))	color = color_rgba(255, 80, 80, 120);
		else if (smart_cast<CSE_ALifeSmartZone*>(obj))		color = color_rgba(180, 80, 255, 120);
		else if (smart_cast<CSE_ALifeTeamBaseZone*>(obj))	color = color_rgba(30, 30, 220, 140);
		else if (smart_cast<CSE_SmartCover*>(obj))			color = color_rgba(80, 220, 255, 120);
		else if (smart_cast<CSE_ALifeObjectClimable*>(obj))	color = color_rgba(255, 200, 80, 120);

		renderSpaceObjectShape(obj, shape->shapes, color);
	}
}
