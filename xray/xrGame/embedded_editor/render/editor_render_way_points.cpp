#include "stdafx.h"

#include "editor_render_way_points.h"

#include <algorithm>

#include "../Include/xrRender/DrawUtils.h"
#include "../Include/xrRender/UIRender.h"
#include "ai_space.h"
#include "game_graph.h"
#include "level_graph.h"
#include "patrol_path.h"
#include "patrol_path_storage.h"

constexpr const float			 waypoint_size		 = 1.5f;
constexpr const float			 waypoint_radius	 = waypoint_size * 0.5f;
constexpr const float			 waypoint_lift		 = waypoint_size * 0.85f;
constexpr const float			 waypoint_link_width = 0.25f;
constexpr const float			 max_draw_dist		 = 200.f;
constexpr const float			 max_label_dist		 = 50.f;

bool							 isRenderWayPoints	 = false;
static bool						 paths_dumped		 = false;
xr_vector<WaypointLabel>		 g_waypoint_labels;
static xr_vector<shared_str>	 g_cached_paths;
static xr_vector<CachedPathInfo> g_cached_path_data;
static shared_str				 g_common_prefix;
static u16						 g_cached_level_id = u16(-1);

void renderWayPoints() {
	g_waypoint_labels.clear();
	g_waypoint_labels.reserve(4000);

	const auto& storage = ai().patrol_paths();
	if (storage.patrol_paths().empty())
		return;

	const CLevelGraph& level_graph		= ai().level_graph();
	const CGameGraph&  game_graph		= ai().game_graph();
	u16				   current_level_id = level_graph.level_id();

	// Перекэширование при смене уровня
	if (g_cached_level_id != current_level_id) {
		g_cached_paths.clear();
		g_cached_path_data.clear();
		g_common_prefix	  = "";
		g_cached_level_id = current_level_id;
	}

	// Кэширование путей, относящихся к текущему уровню
	if (g_cached_paths.empty()) {
		xr_vector<shared_str> temp_paths;
		temp_paths.reserve(storage.patrol_paths().size() / 2);
		g_cached_path_data.reserve(storage.patrol_paths().size() / 2);

		// Сбор статистики префиксов и отбор путей на текущем уровне
		xr_map<shared_str, u32> prefix_counts;
		u32						max_count = 0;

		for (const auto& path_pair : storage.patrol_paths()) {
			const CPatrolPath* path = path_pair.second;
			if (!path)
				continue;

			const auto&	   vertices				= path->vertices();
			bool		   has_valid_on_current = false;
			CachedPathInfo info{};
			info.bbox.invalidate();

			for (const auto& vertex_pair : vertices) {
				const CPatrolPoint* point = &vertex_pair.second->data();
				if (!point)
					continue;

				GameGraph::_GRAPH_ID game_vid = point->game_vertex_id();
				if (game_vid != GameGraph::_GRAPH_ID(-1) && game_graph.valid_vertex_id(game_vid)) {
					u16 point_level_id = game_graph.vertex(game_vid)->level_id();
					if (point_level_id == current_level_id) {
						has_valid_on_current   = true;

						// Выделение префикса
						const char* name_c	   = path_pair.first.c_str();
						const char* underscore = strchr(name_c, '_');
						shared_str	prefix;
						if (underscore) {
							string_path temp_prefix;
							size_t		prefix_len = underscore - name_c;
							strncpy(temp_prefix, name_c, prefix_len);
							temp_prefix[prefix_len] = '\0';
							prefix					= temp_prefix;
						} else {
							prefix = path_pair.first;
						}

						++prefix_counts[prefix];
						if (prefix_counts[prefix] > max_count) {
							max_count		= prefix_counts[prefix];
							g_common_prefix = prefix;
						}
					}
				}

				Fvector pos = point->position();
				info.bbox.modify(pos);
			}

			if (has_valid_on_current) {
				temp_paths.push_back(path_pair.first);

				Fvector c;
				info.bbox.getcenter(c);
				info.center = c;
				info.radius = info.bbox.getradius();
				g_cached_path_data.push_back(info);
			}
		}

		// Добавление путей без game vertex id с наиболее часто встречающимся префиксом
		for (const auto& path_pair : storage.patrol_paths()) {
			const CPatrolPath* path = path_pair.second;
			if (!path)
				continue;

			bool		   has_any_valid = false;
			const auto&	   vertices		 = path->vertices();
			CachedPathInfo info{};
			info.bbox.invalidate();

			for (const auto& vertex_pair : vertices) {
				const CPatrolPoint* point = &vertex_pair.second->data();
				if (!point)
					continue;

				GameGraph::_GRAPH_ID game_vid = point->game_vertex_id();
				if (game_vid != GameGraph::_GRAPH_ID(-1) && game_graph.valid_vertex_id(game_vid)) {
					has_any_valid = true;
					break;
				}

				Fvector pos = point->position();
				info.bbox.modify(pos);
			}

			if (has_any_valid)
				continue;

			const char* name_c	   = path_pair.first.c_str();
			const char* underscore = strchr(name_c, '_');
			shared_str	prefix;
			if (underscore) {
				string_path temp_prefix;
				size_t		prefix_len = underscore - name_c;
				strncpy(temp_prefix, name_c, prefix_len);
				temp_prefix[prefix_len] = '\0';
				prefix					= temp_prefix;
			} else {
				prefix = path_pair.first;
			}

			if (prefix == g_common_prefix || underscore == nullptr) {
				temp_paths.push_back(path_pair.first);

				Fvector c;
				info.bbox.getcenter(c);
				info.center = c;
				info.radius = info.bbox.getradius();
				g_cached_path_data.push_back(info);
			}
		}

		g_cached_paths = std::move(temp_paths);
	}

	const u32 normal_point_color  = color_rgba(  0, 255,   0, 255);
	const u32 normal_link_color	  = color_rgba(255, 160,  10, 255);
	const u32 normal_text_color	  = color_rgba( 10, 128, 240, 255);
	const u32 look_point_color	  = color_rgba( 10, 128, 240, 255);
	const u32 look_link_color	  = color_rgba(  0, 240, 240, 255);
	const u32 look_text_color	  = color_rgba(255, 160,  10, 255);
	const u32 invalid_point_color = color_rgba(240,   0,  10, 255);
	const u32 shadow_color		  = color_rgba(0,     0,   0, 255);

	UIRender->CacheSetCullMode(IUIRender::cmNONE);

	u32					  visible_points = 0;
	u32					  visible_lines	 = 0;

	Fvector				  zero			 = {0.f, 0.f, 0.f};
	Fmatrix				  M{};

	xr_vector<shared_str> drawn_paths;
	drawn_paths.reserve(1000);

	CFrustum frustum{};
	frustum.CreateFromMatrix(Device.mFullTransform, FRUSTUM_P_ALL);

	size_t path_index = 0;
	for (const auto& path_name : g_cached_paths) {
		const auto path_it = storage.patrol_paths().find(path_name);
		if (path_it == storage.patrol_paths().end()) {
			++path_index;
			continue;
		}

		const CPatrolPath* path = path_it->second;
		if (!path) {
			++path_index;
			continue;
		}

		const CachedPathInfo& info		  = g_cached_path_data[path_index++];
		Fvector				  center_copy = info.center;

		if (!frustum.testSphere_dirty(center_copy, info.radius))
			continue;

		const auto&		   vertices		= path->vertices();
		size_t			   num_vertices = vertices.size();

		xr_vector<Fvector> point_positions(num_vertices);
		xr_vector<u8>	   point_invalid(num_vertices, 0);

		bool			   has_visible = false;

		// Сбор позиций точек и валидация
		for (const auto& vertex_pair : vertices) {
			u32					vertex_id = vertex_pair.first;
			const CPatrolPoint* point	  = &vertex_pair.second->data();
			if (!point)
				continue;

			GameGraph::_GRAPH_ID game_vid	   = point->game_vertex_id();
			bool				 is_valid_game = (game_vid != GameGraph::_GRAPH_ID(-1) && game_graph.valid_vertex_id(game_vid));
			if (!is_valid_game)
				point_invalid[vertex_id] = 1;

			Fvector pos = point->position();

			// Довольно бесполезная проверка, т.к. bbox'ы уровней в ЧН почти всегда переналагаются друг на друга
			Fbox	box = level_graph.header().box();
			if (!(box.min.x <= pos.x && pos.x <= box.max.x && box.min.z <= pos.z && pos.z <= box.max.z)) {
				if (!paths_dumped)
					Msg("~ Invalid vertex %u in path [%s]: skipped (outside level bbox)", vertex_id, path_name.c_str());
				continue;
			}

			pos.y += waypoint_lift;

			if (pos.distance_to(Device.vCameraPosition) > max_draw_dist)
				continue;

			has_visible				   = true;
			point_positions[vertex_id] = pos;
		}

		if (!has_visible)
			continue;

		drawn_paths.push_back(path_name);

		bool is_look_path	  = (path_name.size() >= 5 && strcmp(path_name.c_str() + path_name.size() - 5, "_look") == 0);
		u32	 point_color_base = is_look_path ? look_point_color : normal_point_color;
		u32	 line_color		  = is_look_path ? look_link_color : normal_link_color;
		u32	 text_color		  = is_look_path ? look_text_color : normal_text_color;

		// Отрисовка маркеров точек
		for (u32 id = 0; id < num_vertices; ++id) {
			const Fvector& pos_ref = point_positions[id];
			if (pos_ref.x == 0.f && pos_ref.y == 0.f && pos_ref.z == 0.f)
				continue;

			Fvector pos				 = pos_ref;
			u32		this_point_color = point_invalid[id] ? invalid_point_color : point_color_base;

			M.translate(pos);
			UIRender->CacheSetXformWorld(M);
			DU->DrawCross(zero,
						  waypoint_radius,
						  waypoint_lift,
						  waypoint_radius,
						  waypoint_radius,
						  waypoint_size * 0.15f,
						  waypoint_radius,
						  this_point_color,
						  false);

			++visible_points;
		}

		Fidentity.identity();
		UIRender->CacheSetXformWorld(Fidentity);

		// Отрисовка связей и меток весов путей
		for (const auto& vertex_pair : vertices) {
			u32			   current_id	   = vertex_pair.first;
			const Fvector& current_pos_ref = point_positions[current_id];
			if (current_pos_ref.x == 0.f && current_pos_ref.y == 0.f && current_pos_ref.z == 0.f)
				continue;

			Fvector		current_pos = current_pos_ref;

			const auto& edges		= vertex_pair.second->edges();
			for (const auto& edge : edges) {
				u32 target_id = edge.vertex_id();
				if (target_id >= num_vertices)
					continue;

				const Fvector& target_pos_ref = point_positions[target_id];
				if (target_pos_ref.x == 0.f && target_pos_ref.y == 0.f && target_pos_ref.z == 0.f)
					continue;

				Fvector target_pos = target_pos_ref;

				Fvector mid{};
				mid.add(current_pos, target_pos);
				mid.mul(0.5f);

				if (mid.distance_to(Device.vCameraPosition) > max_draw_dist)
					continue;

				DU->DrawLink(current_pos, target_pos, waypoint_link_width, line_color);
				++visible_lines;

				mid.y += 0.5f;
				if (mid.distance_to(Device.vCameraPosition) < max_label_dist) {
					char buf[32];
					sprintf(buf, "P: %1.2f", edge.weight());
					g_waypoint_labels.push_back({mid, text_color, shadow_color, buf});
				}
			}
		}

		// Метка имени над первой точкой патрульного пути
		for (u32 id = 0; id < num_vertices; ++id) {
			const Fvector& pos_ref = point_positions[id];
			if (pos_ref.x != 0.f || pos_ref.y != 0.f || pos_ref.z != 0.f) {
				Fvector first_pos = pos_ref;
				first_pos.y += 1.0f;

				if (first_pos.distance_to(Device.vCameraPosition) < max_label_dist) {
					string128 hint;
					sprintf(hint, "%s", path_name.c_str());
					g_waypoint_labels.push_back({first_pos, text_color, shadow_color, hint});
				}
				break;
			}
		}
	}

	// Сброс отрисованных путей в файл при первом рендеринге. Вдруг ещё пригодится.
/*	if (!paths_dumped && !drawn_paths.empty()) {
		std::sort(drawn_paths.begin(), drawn_paths.end(), [](const shared_str& a, const shared_str& b) { return xr_strcmp(a.c_str(), b.c_str()) < 0; });

		FILE* f = fopen("patrol_paths_dump.txt", "w");
		if (f) {
			for (const auto& name : drawn_paths)
				fprintf(f, "%s\n", name.c_str());

			fclose(f);
			Msg("~ Drawn patrol paths dumped to patrol_paths_dump.txt (%u paths)", (u32)drawn_paths.size());
		} else {
			Msg("! Failed to open patrol_paths_dump.txt for writing");
		}

		paths_dumped = true;
	}
	*/
	UIRender->CacheSetCullMode(IUIRender::cmCCW);
}
