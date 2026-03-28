#include "stdafx.h"

#include "editor_render.h"

#include "editor_render_ai_map.h"
#include "editor_render_spawn_element.h"
#include "editor_render_way_points.h"

extern bool isRenderAiMap;
extern bool isRenderSpawnElement;
extern bool isRenderWayPoints;

void embedded_editor_render() {
	if (isRenderAiMap)			renderAiMap();
	if (isRenderSpawnElement)	renderSpawnElements();
	if (isRenderWayPoints)		renderWayPoints();
}
