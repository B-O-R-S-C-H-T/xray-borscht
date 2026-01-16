#pragma once

void renderWayPoints();

struct WaypointLabel {
	Fvector	  world_pos;
	u32		  color;
	u32		  shadow_color;
	xr_string text;
};

extern xr_vector<WaypointLabel> g_waypoint_labels;

struct CachedPathInfo {
	Fbox	bbox;
	Fvector center;
	float	radius;
};

extern xr_vector<shared_str>	 g_cached_paths;
extern xr_vector<CachedPathInfo> g_cached_path_data;
