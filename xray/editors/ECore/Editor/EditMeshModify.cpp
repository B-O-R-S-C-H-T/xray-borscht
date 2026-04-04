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
		VMMap VM;
		VM.reserve(m_VertCount);
		VM.max_load_factor(0.92f);

		FvectorVec m_NewPoints;
		m_NewPoints.reserve(m_VertCount);

		const float CELL_SIZE = EPS_L;
		boolVec		faces_mark;
		faces_mark.resize(m_FaceCount, false);
		int i_del_face = 0;

		for (u32 k = 0; k < m_FaceCount; k++) {
			st_Face& face = m_Faces[k];
			Fvector	 points[3];
			int		 mface[3] = {-1, -1, -1};

			for (int i = 0; i < 3; i++)
				points[i].set(m_Vertices[face.pv[i].pindex]);

			for (int i = 0; i < 3; i++) {
				GridKey qkey  = {iFloor(points[i].x / CELL_SIZE), iFloor(points[i].y / CELL_SIZE), iFloor(points[i].z / CELL_SIZE)};

				bool	found = false;
				for (int dx = -1; dx <= 1 && !found; ++dx)
					for (int dy = -1; dy <= 1 && !found; ++dy)
						for (int dz = -1; dz <= 1 && !found; ++dz) {
							GridKey nk	= {qkey.x + dx, qkey.y + dy, qkey.z + dz};
							auto	mit = VM.find(nk);
							if (mit != VM.end()) {
								for (u32 idx : mit->second) {
									if (m_NewPoints[idx].similar(points[i], EPS)) {
										mface[i] = (int)idx;
										found	 = true;
										break;
									}
								}
							}
						}

				if (!found) {
					mface[i] = (int)m_NewPoints.size();
					m_NewPoints.push_back(points[i]);
					VM[qkey].push_back((u32)mface[i]);
				}
			}

			if ((mface[0] == mface[1]) || (mface[1] == mface[2]) || (mface[0] == mface[2])) {
				faces_mark[k] = true;
				i_del_face++;
			} else {
				face.pv[0].pindex = mface[0];
				face.pv[1].pindex = mface[1];
				face.pv[2].pindex = mface[2];
			}
		}

		m_VertCount = m_NewPoints.size();
		xr_free(m_Vertices);
		m_Vertices = xr_alloc<Fvector>(m_VertCount);
		Memory.mem_copy(m_Vertices, &*m_NewPoints.begin(), m_NewPoints.size() * sizeof(Fvector));

		if (i_del_face) {
			st_Face* old_faces = m_Faces;
			u32*	 old_sg	   = m_SmoothGroups;

			u32		 new_face_count = m_FaceCount - i_del_face;

			m_Faces					= xr_alloc<st_Face>(new_face_count);
			m_SmoothGroups			= xr_alloc<u32>(new_face_count);

			std::vector<u32> old_to_new(m_FaceCount, u32(-1));
			u32				 new_idx = 0;

			for (u32 old_idx = 0; old_idx < m_FaceCount; ++old_idx) {
				if (!faces_mark[old_idx]) {
					old_to_new[old_idx]		= new_idx;
					m_Faces[new_idx]		= old_faces[old_idx];
					m_SmoothGroups[new_idx] = old_sg[old_idx];
					++new_idx;
				}
			}

			for (SurfFacesPairIt plp_it = m_SurfFaces.begin(); plp_it != m_SurfFaces.end(); ++plp_it) {
				IntVec& pol_lst = plp_it->second;
				for (int i = 0; i < (int)pol_lst.size();) {
					int old_f = pol_lst[i];
					u32 new_f = old_to_new[old_f];
					if (new_f != u32(-1)) {
						pol_lst[i] = (int)new_f;
						++i;
					} else {
						pol_lst.erase(pol_lst.begin() + i);
					}
				}
			}

			m_FaceCount = new_face_count;

			xr_free(old_faces);
			xr_free(old_sg);
		}

#ifdef _EDITOR
		if (verts_before != m_VertCount || i_del_face > 0) {
			float  saved	 = verts_before > 0 ? 100.0f * float(verts_before - m_VertCount) / verts_before : 0.0f;

			LPCSTR mesh_name = m_Name.c_str() && m_Name[0] ? m_Name.c_str() : "unnamed_mesh";

			ELog.Msg(mtInformation, "Optimize %s:\n\tverts %d -> %d (%.1f%%), faces removed: %d", mesh_name, verts_before, m_VertCount, saved, i_del_face);
		}
#endif
	}
}
