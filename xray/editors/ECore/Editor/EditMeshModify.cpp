//----------------------------------------------------
// file: EditMeshModify.cpp
//----------------------------------------------------

#include "stdafx.h"
#pragma hdrstop

#include <ankerl/unordered_dense.h>

#include "EditMesh.h"
#include "EditObject.h"

void CEditableMesh::Transform(const Fmatrix& parent)
{
	// transform position
	for(u32 k=0; k<m_VertCount; ++k)
		parent.transform_tiny(m_Vertices[k]);

    // RecomputeBBox
	RecomputeBBox	();

    // update normals & cform
#ifdef _EDITOR
	UnloadRenderBuffers	();
	UnloadCForm		();
#endif
    UnloadFNormals	(true);
    UnloadVNormals	(true);
    UnloadSVertices	(true);
}

int CEditableMesh::FindSimilarUV(st_VMap* vmap, Fvector2& _uv)
{
	int sz			= vmap->size();
	for (int k=0; k<sz; ++k)
	{
		const Fvector2& uv = vmap->getUV(k);
		if (uv.similar(_uv)) 
			return k;
	}
	return -1;
}

int CEditableMesh::FindSimilarWeight(st_VMap* vmap, float _w)
{
	int sz			= vmap->size();
	for (int k=0; k<sz; ++k)
	{
		float w		= vmap->getW(k);
		if (fsimilar(w,_w)) return k;
	}
	return -1;
}

void CEditableMesh::RebuildVMaps()
{
//.	Log			("Rebuilding VMaps...");
	IntVec		m_VertVMap;
	m_VertVMap.resize(m_VertCount,-1);
	VMapVec		nVMaps;
	VMRefsVec	nVMRefs;
	// refs copy to new
	{
		nVMRefs.resize(m_VMRefs.size());
		for (VMRefsIt o_it=m_VMRefs.begin(),n_it=nVMRefs.begin(); o_it!=m_VMRefs.end(); o_it++,n_it++){
			n_it->count	= o_it->count;
			n_it->pts	= xr_alloc<st_VMapPt>(n_it->count);
		}
	}

	for (u32 f_id=0; f_id<m_FaceCount; f_id++){
		st_Face& F=m_Faces[f_id];
		for (int k=0; k<3; k++){
			u32 pts_cnt			= m_VMRefs[F.pv[k].vmref].count;
			for (u32 pt_id=0; pt_id<pts_cnt; pt_id++){
                st_VMapPt* n_pt_it	= &nVMRefs[F.pv[k].vmref].pts[pt_id];
                st_VMapPt* o_pt_it	= &m_VMRefs[F.pv[k].vmref].pts[pt_id];
				st_VMap* vmap=m_VMaps[o_pt_it->vmap_index];
				switch (vmap->type){
				case vmtUV:{
					int& pm=m_VertVMap[F.pv[k].pindex];
					if (-1==pm){ // point map
						pm=F.pv[k].vmref;
						int vm_idx=FindVMapByName(nVMaps,vmap->name.c_str(),vmap->type,false);
						if (-1==vm_idx){
							nVMaps.push_back(xr_new<st_VMap>(vmap->name.c_str(),(u8)vmap->type,false));
							vm_idx=nVMaps.size()-1;
						}
						st_VMap* nVMap=nVMaps[vm_idx];

//						int uv_idx = FindSimilarUV(nVMap,vmap->getUV(pt_it->index));
//						if (uv_idx==-1){
//							uv_idx	= nVMap->size();
//							nVMap->appendUV(vmap->getUV(pt_it->index));
//							nVMap->appendVI(F.pv[k].pindex);
//						}

						nVMap->appendUV(vmap->getUV(o_pt_it->index));
						nVMap->appendVI(F.pv[k].pindex);
						n_pt_it->index = nVMap->size()-1;
						n_pt_it->vmap_index=vm_idx;
					}
					else{ // poly map
						int vm_idx=FindVMapByName(nVMaps,vmap->name.c_str(),vmap->type,true);
						if (-1==vm_idx){
							nVMaps.push_back(xr_new<st_VMap>(vmap->name.c_str(),(u8)vmap->type,true));
							vm_idx=nVMaps.size()-1;
						}
						st_VMap* nVMapPM=nVMaps[vm_idx];

//						int uv_idx = FindSimilarUV(nVMapPM,vmap->getUV(pt_it->index));
//						if (uv_idx==-1){
//							uv_idx	= nVMapPM->size();
//							nVMapPM->appendUV(vmap->getUV(pt_it->index));
//							nVMapPM->appendVI(F.pv[k].pindex);
//							nVMapPM->appendPI(f_it-m_Faces.begin());
//						}
//						n_pt_it->index = uv_idx;

						nVMapPM->appendUV(vmap->getUV(o_pt_it->index));
						nVMapPM->appendVI(F.pv[k].pindex);
						nVMapPM->appendPI(f_id);
						n_pt_it->index = nVMapPM->size()-1;
						n_pt_it->vmap_index=vm_idx;
					}
				}break;
				case vmtWeight:{
					int vm_idx=FindVMapByName(nVMaps,vmap->name.c_str(),vmap->type,false);
					if (-1==vm_idx){
						nVMaps.push_back(xr_new<st_VMap>(vmap->name.c_str(),(u8)vmap->type,false));
						vm_idx=nVMaps.size()-1;
					}
					st_VMap* nWMap=nVMaps[vm_idx];
					nWMap->appendW	(vmap->getW(o_pt_it->index));
					nWMap->appendVI	(F.pv[k].pindex);
					n_pt_it->index	= nWMap->size()-1;
					n_pt_it->vmap_index=vm_idx;
				}break;
				}
			}
		}
	}
	for (VMapIt vm_it=m_VMaps.begin(); vm_it!=m_VMaps.end(); vm_it++)
		xr_delete(*vm_it);

	m_VMaps.clear();
	m_VMaps=nVMaps;
	// clear refs
	for (VMRefsIt ref_it=m_VMRefs.begin(); ref_it!=m_VMRefs.end(); ref_it++)
		xr_free			(ref_it->pts);
	m_VMRefs.clear();
	m_VMRefs = nVMRefs;
}

struct GridKey {
	int	 x, y, z;
	bool operator==(const GridKey& other) const { return x == other.x && y == other.y && z == other.z; }
};

namespace ankerl::unordered_dense {
	template <>
	struct hash<GridKey> {
		using is_avalanching = void;
		auto operator()(GridKey const& k) const noexcept -> std::uint64_t { return hash<std::tuple<int, int, int>>()(std::make_tuple(k.x, k.y, k.z)); }
	};
} // namespace ankerl::unordered_dense

void CEditableMesh::OptimizeMesh(BOOL NoOpt) {
	if (!NoOpt) {
#ifdef _EDITOR
		UnloadRenderBuffers();
		UnloadCForm();
#endif
		UnloadFNormals(true);
		UnloadVNormals(true);
		UnloadSVertices(true);
		UnloadAdjacency(true);

#ifdef _EDITOR
		u32 verts_before = m_VertCount;
		u32 faces_before = m_FaceCount;
#endif

		using VMMap = ankerl::unordered_dense::map<GridKey, U32Vec>;
		VMMap vertex_grid;
		vertex_grid.reserve(m_VertCount);
		vertex_grid.max_load_factor(0.92f);

		FvectorVec new_vertices;
		new_vertices.reserve(m_VertCount);

		const float CELL_SIZE = EPS_L;

		boolVec		faces_to_remove;
		faces_to_remove.resize(m_FaceCount, false);
		int removed_face_count = 0;

		// Удаление дубликатов вершин
		for (u32 face_idx = 0; face_idx < m_FaceCount; face_idx++) {
			st_Face& current_face = m_Faces[face_idx];

			Fvector	 face_points[3];
			int		 new_vertex_indices[3] = {-1, -1, -1};

			for (int i = 0; i < 3; i++) {
				face_points[i].set(m_Vertices[current_face.pv[i].pindex]);
			}

			for (int i = 0; i < 3; i++) {
				GridKey grid_key	 = {iFloor(face_points[i].x / CELL_SIZE), iFloor(face_points[i].y / CELL_SIZE), iFloor(face_points[i].z / CELL_SIZE)};

				bool vertex_found = false;

				for (int dx = -1; dx <= 1 && !vertex_found; ++dx) {
					for (int dy = -1; dy <= 1 && !vertex_found; ++dy) {
						for (int dz = -1; dz <= 1 && !vertex_found; ++dz) {
							GridKey neighbor_key = {grid_key.x + dx, grid_key.y + dy, grid_key.z + dz};

							auto it = vertex_grid.find(neighbor_key);
							if (it != vertex_grid.end()) {
								for (u32 existing_idx : it->second) {
									if (new_vertices[existing_idx].similar(face_points[i], EPS)) {
										new_vertex_indices[i] = (int)existing_idx;
										vertex_found		  = true;
										break;
									}
								}
							}
						}
					}
				}

				if (!vertex_found) {
					new_vertex_indices[i] = (int)new_vertices.size();
					new_vertices.push_back(face_points[i]);
					vertex_grid[grid_key].push_back((u32)new_vertex_indices[i]);
				}
			}

			// Не стала ли грань вырожденной после удаления дубликатов вершин?
			if ((new_vertex_indices[0] == new_vertex_indices[1]) || (new_vertex_indices[1] == new_vertex_indices[2]) || (new_vertex_indices[0] == new_vertex_indices[2])) {
				faces_to_remove[face_idx] = true;
				removed_face_count++;
			} else {
				current_face.pv[0].pindex = new_vertex_indices[0];
				current_face.pv[1].pindex = new_vertex_indices[1];
				current_face.pv[2].pindex = new_vertex_indices[2];
			}
		}

		// Обновление массива вершин
		m_VertCount = new_vertices.size();
		xr_free(m_Vertices);
		m_Vertices = xr_alloc<Fvector>(m_VertCount);
		Memory.mem_copy(m_Vertices, &*new_vertices.begin(), new_vertices.size() * sizeof(Fvector));

		// Удаление вырожденных граней
		if (removed_face_count > 0) {
			st_Face* old_faces		   = m_Faces;
			u32*	 old_smooth_groups = m_SmoothGroups;

			u32		 new_face_count	   = m_FaceCount - removed_face_count;

			m_Faces					   = xr_alloc<st_Face>(new_face_count);
			m_SmoothGroups			   = xr_alloc<u32>(new_face_count);

			// старый_индекс_грани -> новый_индекс_грани
			std::vector<u32> old_to_new_index_map(m_FaceCount, u32(-1));

			u32				 new_face_index = 0;
			for (u32 old_face_index = 0; old_face_index < m_FaceCount; ++old_face_index) {
				if (!faces_to_remove[old_face_index]) {
					old_to_new_index_map[old_face_index] = new_face_index;
					m_Faces[new_face_index]				 = old_faces[old_face_index];
					m_SmoothGroups[new_face_index]		 = old_smooth_groups[old_face_index];
					++new_face_index;
				}
			}

			// Обновление поверхностей
			for (SurfFacesPairIt surf_it = m_SurfFaces.begin(); surf_it != m_SurfFaces.end(); ++surf_it) {
				IntVec& surface_face_list = surf_it->second;
				for (int i = 0; i < (int)surface_face_list.size();) {
					int old_face_id = surface_face_list[i];
					u32 new_face_id = old_to_new_index_map[old_face_id];

					if (new_face_id != u32(-1)) {
						surface_face_list[i] = (int)new_face_id;
						++i;
					} else {
						surface_face_list.erase(surface_face_list.begin() + i);
					}
				}
			}

			m_FaceCount = new_face_count;

			xr_free(old_faces);
			xr_free(old_smooth_groups);
		}

#ifdef _EDITOR
		LPCSTR source_file_name = (m_Parent && m_Parent->m_LibName.length() > 0) ? m_Parent->m_LibName.c_str() : "unnamed_file";

		if (verts_before != m_VertCount || removed_face_count > 0) {
			float saved_percent = verts_before > 0 ? 100.0f * float(verts_before - m_VertCount) / verts_before : 0.0f;
			ELog.Msg(mtInformation,
					 "Optimize %s:\n\tverts %d -> %d (%.1f%%), faces removed: %d",
					 source_file_name,
					 verts_before,
					 m_VertCount,
					 saved_percent,
					 removed_face_count);
		}
#endif
	}
}
