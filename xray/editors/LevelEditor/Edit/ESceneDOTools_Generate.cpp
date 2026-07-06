#include "stdafx.h"
#pragma hdrstop

#include <Include/cl_intersect.h>
#include <utils/ETools/ETools.h>
#include <random>
#include <execution>
#include <ranges>

#include "Scene.h"
#include "SceneObject.h"
#include "ESceneDOTools.h"
#include "../ECore/Editor/EditMesh.h"
#include "../ECore/Editor/EditObject.h"
#include "../ECore/Editor/ImageManager.h"
#include "../ECore/Editor/Library.h"
#include "../ECore/Editor/ui_main.h"
#include "../ECore/Engine/Texture.h"
#include "../../Layers/xrRender/DetailFormat.h"

static Fvector down_vec  = { 0.f, -1.f,  0.f};
static Fvector left_vec  = {-1.f,  0.f,  0.f};
static Fvector right_vec = { 1.f,  0.f,  0.f};
static Fvector fwd_vec   = { 0.f,  0.f,  1.f};
static Fvector back_vec  = { 0.f,  0.f, -1.f};

static thread_local std::mt19937 g_detail_rng(0x26111975u);
static CRandom DetailRandom(0x26111975);

DetailSlot&	EDetailManager::GetSlot(u32 sx, u32 sz){
	VERIFY(sx<dtH.size_x);
	VERIFY(sz<dtH.size_z);
	return dtSlots[sz*dtH.size_x+sx];
}

bool EDetailManager::FindClosestIndex(const Fcolor& C, SIndexDistVec& best) {

    u32				 index;
    float			 dist = flt_max;
    Fcolor			 src;
    float			 inv_a = 1.0f - C.a;
    bool			 bRes  = false;

    for (auto it = m_ColorIndices.begin(); it != m_ColorIndices.end(); ++it) {
        if (it->second.empty())
            continue;

        src.set(it->first);
        float d = inv_a + sqrtf((C.r - src.r) * (C.r - src.r) + (C.g - src.g) * (C.g - src.g) + (C.b - src.b) * (C.b - src.b));

        if (d < dist) {
            dist  = d;
            index = it->first;
            bRes  = true;
        }
    }

    if (bRes) {
        if (best.size() < 4) {
            bool bFound = false;
            for (u32 k = 0; k < best.size(); ++k) {
                if (best[k].index == index) {
                    if (dist < best[k].dist) {
                        best[k].dist  = dist;
                        best[k].index = index;
                    }
                    bFound = true;
                    break;
                }
            }
            if (!bFound) {
                best.inc();
                best[best.size() - 1].dist	= dist;
                best[best.size() - 1].index = index;
            }
        } else {
            int	  i		 = -1;
            float dd	 = flt_max;
            bool  bFound = false;
            for (int k = 0; k < 4; ++k) {
                float d = dist - best[k].dist;
                if ((d < 0) && (d < dd)) {
                    i  = k;
                    dd = d;
                }
                if (best[k].index == index) {
                    if (dist < best[k].dist) {
                        best[k].dist  = dist;
                        best[k].index = index;
                    }
                    bFound = true;
                    break;
                }
            }
            if (!bFound && (i >= 0)) {
                best[i].dist  = dist;
                best[i].index = index;
            }
        }
    }

    return bRes;
}

bool EDetailManager::Initialize()
{
	if (m_SnapObjects.empty()){
    	ELog.DlgMsg(mtError,"Snap list empty!");
    	return false;
    }
	if (!m_Base.Valid()){
    	ELog.DlgMsg(mtError,"Base texture empty!");
    	return false;
    }				
    if (!UpdateHeader())                return false;
    m_Base.CreateRMFromObjects			(m_BBox,m_SnapObjects);
    if (!UpdateSlots()) 		   		return false;
    if (!objects.empty()&&!UpdateObjects(false,false))	return false;
	return true;
}

bool EDetailManager::Reinitialize()
{
	if (m_SnapObjects.empty()){
    	ELog.DlgMsg(mtError,"Snap list empty!");
    	return false;
    }
	if (!m_Base.Valid()){
    	ELog.DlgMsg(mtError,"Base texture empty!");
    	return false;
    }				
    InvalidateCache();

    if (!UpdateHeader())            return false;
    m_Base.CreateRMFromObjects		(m_BBox,m_SnapObjects);
//.    if (!UpdateBaseTexture(0))		return false;
    if (!UpdateSlots()) 			return false;
    if (!objects.empty()&&!UpdateObjects(false,false))return false;

	return true;
}

bool EDetailManager::UpdateHeader(){
    // get bounding box
	if (!Scene->GetBox(m_BBox,m_SnapObjects)) return false;

    // fill header
    int mn_x 			= iFloor(m_BBox.min.x/DETAIL_SLOT_SIZE);
    int mn_z 			= iFloor(m_BBox.min.z/DETAIL_SLOT_SIZE);
    int mx_x 			= iFloor(m_BBox.max.x/DETAIL_SLOT_SIZE)+1;
    int mx_z 			= iFloor(m_BBox.max.z/DETAIL_SLOT_SIZE)+1;
    dtH.offs_x 	= -mn_x;
    dtH.offs_z 	= -mn_z;
	dtH.size_x 	= mx_x-mn_x;
	dtH.size_z 	= mx_z-mn_z;
    return true;
}

#define EPS_L_VAR 0.0012345f
void EDetailManager::UpdateSlotBBox(int sx, int sz, DetailSlot& slot)
{
	Fbox bbox;
    Frect rect;
    GetSlotRect			(rect,sx,sz);
    bbox.min.set		(rect.x1, m_BBox.min.y, rect.y1);
    bbox.max.set		(rect.x2, m_BBox.max.y, rect.y2);

    SBoxPickInfoVec pinf;
    ETOOLS::box_options(0);
	bool has_hit = Scene->BoxPickObjects(bbox, pinf, &m_SnapObjects);

	if (has_hit) {
		bbox.grow(EPS_L_VAR);

		Fplane frustum_planes[4];
		frustum_planes[0].build(bbox.min, left_vec);
		frustum_planes[1].build(bbox.min, back_vec);
		frustum_planes[2].build(bbox.max, right_vec);
		frustum_planes[3].build(bbox.max, fwd_vec);

		CFrustum frustum;
		frustum.CreateFromPlanes(frustum_planes, 4);

		float y_min = flt_max;
		float y_max = flt_min;

		for (auto& info : pinf) {
			for (auto& face : info.inf) {
				Fvector verts[3];
				info.e_obj->GetFaceWorld(info.s_obj->_Transform(), info.e_mesh, face.id, verts);

				sPoly src(verts, 3), dst;
				if (frustum.ClipPoly(src, dst)) {
					for (auto& v : dst) {
						y_min = std::min(y_min, v.y - 0.03f);
						y_max = std::max(y_max, v.y + 0.03f);
					}
				}
			}
		}

		slot.w_y(y_min, y_max - y_min);
		for (int i = 0; i < 4; ++i)
			slot.w_id(i, DetailSlot::ID_Empty);
	} else {
		ZeroMemory(&slot, sizeof(DetailSlot));
		for (int i = 0; i < 4; ++i)
			slot.w_id(i, DetailSlot::ID_Empty);
	}
}

bool EDetailManager::UpdateSlots()
{
	CTimer timer;
	timer.Start();

	// clear previous slots
    xr_free				(dtSlots);
    dtSlots				= xr_alloc<DetailSlot>(dtH.size_x * dtH.size_z);

	{
		SPBItem* pbPrep = UI->ProgressStart(m_SnapObjects.size(), "Preparing collision models...");
		for (ObjectIt o_it = m_SnapObjects.begin(); o_it != m_SnapObjects.end(); ++o_it) {
			CSceneObject*   S = (CSceneObject*)(*o_it);
			CEditableObject* O = S->GetReference();
			if (O) {
				for (EditMeshIt m_it = O->FirstMesh(); m_it != O->LastMesh(); ++m_it) {
					(*m_it)->GenerateCFModel();
				}
			}
			pbPrep->Inc();
		}
		UI->ProgressEnd(pbPrep);
	}

	SPBItem* pb = UI->ProgressStart(dtH.size_x * dtH.size_z, "Updating bounding boxes...");

	auto process_slot = [&](u32 idx)
	{
		u32 z = idx / dtH.size_x;
		u32 x = idx % dtH.size_x;
		UpdateSlotBBox(x, z, dtSlots[z * dtH.size_x + x]);
	};

	std::for_each(std::execution::par,
				  std::views::iota(0u, dtH.size_x * dtH.size_z).begin(),
				  std::views::iota(0u, dtH.size_x * dtH.size_z).end(),
				  process_slot);

	UI->ProgressEnd(pb);
	m_Selected.resize(dtH.size_x * dtH.size_z);

	float sec = timer.GetElapsed_sec();

	ELog.Msg(mtInformation, "=== Detail BBox Generation ===");
	ELog.Msg(mtInformation, "Slots:           %u", dtH.size_x * dtH.size_z);
	ELog.Msg(mtInformation, "Time:            %.3f sec", sec);

	if (sec > 0.0001f) {
		float rate = (dtH.size_x * dtH.size_z) / sec;
		ELog.Msg(mtInformation, "Rate:            %.0f slots/sec", rate);
	}

	return true;
}

void EDetailManager::GetSlotRect(Frect& rect, int sx, int sz){
    float x 			= fromSlotX(sx);
    float z 			= fromSlotZ(sz);
    rect.x1				= x-DETAIL_SLOT_SIZE_2+EPS_L;
    rect.y1				= z-DETAIL_SLOT_SIZE_2+EPS_L;
    rect.x2				= x+DETAIL_SLOT_SIZE_2-EPS_L;
    rect.y2				= z+DETAIL_SLOT_SIZE_2-EPS_L;
}

void EDetailManager::GetSlotTCRect(Irect& rect, int sx, int sz){
	Frect R;
	GetSlotRect			(R,sx,sz);
	rect.x1 			= m_Base.GetPixelUFromX(R.x1,m_BBox);
	rect.x2 			= m_Base.GetPixelUFromX(R.x2,m_BBox);
	rect.y2 			= m_Base.GetPixelVFromZ(R.y1,m_BBox); // v - координата флипнута
	rect.y1 			= m_Base.GetPixelVFromZ(R.y2,m_BBox);
}

void EDetailManager::CalcClosestCount(int part, const Fcolor& C, SIndexDistVec& best){
    float dist = flt_max;
    Fcolor src;
    float inv_a = 1-C.a;
    int idx = -1;

    for (u32 k=0; k<best.size(); k++){
		src.set(best[k].index);
        float d = inv_a+sqrtf((C.r-src.r)*(C.r-src.r)+(C.g-src.g)*(C.g-src.g)+(C.b-src.b)*(C.b-src.b));
        if (d<dist){
        	dist 	= d;
            idx 	= k;
        }
    }
    if (idx>=0) best[idx].cnt[part]++;
}

u8 EDetailManager::GetRandomObject(u32 color_index)
{
	ColorIndexPairIt CI=m_ColorIndices.find(color_index);
	R_ASSERT(CI!=m_ColorIndices.end());
	int k = DetailRandom.randI(0,CI->second.size());
    DetailIt it = std::find(objects.begin(),objects.end(),CI->second[k]);
    VERIFY(it!=objects.end());
	return u8(it-objects.begin());
}

u8 EDetailManager::GetObject(ColorIndexPairIt& CI, u8 id)
{
	VERIFY(CI!=m_ColorIndices.end());
    DetailIt it = std::find(objects.begin(),objects.end(),(CDetail*)CI->second[id]);
    VERIFY(it!=objects.end());
	return u8(it-objects.begin());
}

bool CompareWeightFunc(SIndexDist& d0, SIndexDist& d1){
	return d0.dist<d1.dist;
}

struct best_rand
{
	CRandom gen;
	best_rand(CRandom& A) { gen=A; }
	int operator()(int n) {return gen.randI(n);}
};

bool EDetailManager::UpdateSlotObjects(int x, int z) {
    DetailSlot* slot	= dtSlots + z * dtH.size_x + x;
    Irect		R;
    GetSlotTCRect(R, x, z);

    SIndexDistVec best;

    // find best color index
	for (int v = R.y1; v <= R.y2; v++) {
		for (int u = R.x1; u <= R.x2; u++) {
			u32 clr;
			if (m_Base.GetColor(clr, u, v)) {
				Fcolor C;
				C.set(clr);
				FindClosestIndex(C, best);
			}
		}
    }

    std::sort(best.begin(), best.end(), CompareWeightFunc);

    // пройдем по 4 частям слота и определим плотность заполнения (учесть переворот V)
    Irect P[4];
    float dx = float(R.x2 - R.x1) / 2.f;
    float dy = float(R.y2 - R.y1) / 2.f;

//	2 3
//	0 1
    P[0].x1=R.x1; 		  			P[0].y1=iFloor(R.y1+dy+0.501f); P[0].x2=iFloor(R.x1+dx+.499f);	P[0].y2=R.y2;
    P[1].x1=iFloor(R.x1+dx+0.501f);	P[1].y1=iFloor(R.y1+dy+0.501f);	P[1].x2=R.x2; 					P[1].y2=R.y2;
    P[2].x1=R.x1; 		  			P[2].y1=R.y1; 		  			P[2].x2=iFloor(R.x1+dx+.499f); 	P[2].y2=iFloor(R.y1+dy+.499f);
    P[3].x1=iFloor(R.x1+dx+0.501f); P[3].y1=R.y1;		  			P[3].x2=R.x2; 					P[3].y2=iFloor(R.y1+dx+.499f);

    for (int part = 0; part < 4; part++) {
        float   alpha = 0;
        int     cnt   = 0;

        for (int v = P[part].y1; v <= P[part].y2; v++) {
            for (int u = P[part].x1; u <= P[part].x2; u++) {
                u32 clr;
                if (m_Base.GetColor(clr, u, v)) {
                    Fcolor C;
                    C.set(clr);
                    CalcClosestCount(part, C, best);
                    alpha += C.a;
                    cnt++;
                }
            }
        }
        alpha /= (cnt ? float(cnt) : 1);
        alpha *= 0.5f;
        for (u32 i = 0; i < best.size(); i++)
            best[i].dens[part] = cnt ? (best[i].cnt[part] * alpha) / float(cnt) : 0;
    }

    // fill empty slots
    R_ASSERT(best.size());
    int  id    = -1;
    u32  o_cnt = 0;
    for (u32 i = 0; i < best.size(); i++)
        o_cnt += m_ColorIndices[best[i].index].size();

    // равномерно заполняем пустые слоты
    if (o_cnt > best.size()) {
        while (best.size() < 4) {
            do {
                id++;
                if (id > 3)		id = 0;
            } while (m_ColorIndices[best[id].index].size() <= 1);
            best.push_back(SIndexDist());
            best.back() = best[id];
            if (best.size() == o_cnt)	break;
        }
    }

    // заполним палитру и установим Random'ы
//	ELog.Msg(mtInformation, "Slot: %d %d",x,z);
    for (u32 k = 0; k < best.size(); k++) {
     	// objects
		ColorIndexPairIt CI = m_ColorIndices.find(best[k].index);	R_ASSERT(CI != m_ColorIndices.end());
        U8Vec elem;		elem.resize(CI->second.size());
        for (U8It b_it = elem.begin(); b_it != elem.end(); ++b_it)	*b_it = u8(b_it - elem.begin());

        std::shuffle(elem.begin(), elem.end(), g_detail_rng);

        for (U8It b_it = elem.begin(); b_it != elem.end(); ++b_it) {
			bool bNotFound = true;
            slot->w_id(k, GetObject(CI, *b_it));
            for (u32 j = 0; j < k; j++) {
                if (slot->r_id(j) == slot->r_id(k)) {
                	bNotFound = false;
                    break;
                }
            }
            if (bNotFound) break;
        }

        slot->color_editor();
        // density
        float f = ((EDetail*)objects[slot->r_id(k)])->m_fDensityFactor;

        slot->palette[k].a0 	= (u16)iFloor(best[k].dens[0] * f * 15.f + .5f);
        slot->palette[k].a1 	= (u16)iFloor(best[k].dens[1] * f * 15.f + .5f);
        slot->palette[k].a2 	= (u16)iFloor(best[k].dens[2] * f * 15.f + .5f);
        slot->palette[k].a3 	= (u16)iFloor(best[k].dens[3] * f * 15.f + .5f);
    }

    // определим ID незаполненных слотов как пустышки
    for (u32 k = best.size(); k < 4; k++)
        slot->w_id(k, DetailSlot::ID_Empty);

    return true;
}

bool EDetailManager::UpdateObjects(bool bUpdateTex, bool bUpdateSelectedOnly)
{
	if (bUpdateTex)		m_Base.ReloadImage();
	if (!m_Base.Valid()) {
    	ELog.DlgMsg(mtError, "Invalid base texture!");
    	return false;
    }
	if (objects.empty()) {
    	ELog.DlgMsg(mtError, "Object list empty!");
     	return false;
	}

	CTimer timer;
	timer.Start();

    // update objects
    SPBItem* pb = UI->ProgressStart(dtH.size_x * dtH.size_z, "Updating objects...");

	auto process_slot = [&](u32 idx)
	{
		u32 z = idx / dtH.size_x;
		u32 x = idx % dtH.size_x;

		if (!bUpdateSelectedOnly || m_Selected[z * dtH.size_x + x])		UpdateSlotObjects(x, z);
	};

	std::for_each(std::execution::par,
				  std::views::iota(0u, dtH.size_x * dtH.size_z).begin(),
				  std::views::iota(0u, dtH.size_x * dtH.size_z).end(),
				  process_slot);

	UI->ProgressEnd(pb);
	InvalidateCache();

	float sec = timer.GetElapsed_sec();

	ELog.Msg(mtInformation, "=== Detail Objects Generation ===");
	ELog.Msg(mtInformation, "Slots:           %u", dtH.size_x * dtH.size_z);
	ELog.Msg(mtInformation, "Time:            %.3f sec", sec);

	if (sec > 0.0001f) {
		float rate = (dtH.size_x * dtH.size_z) / sec;
		ELog.Msg(mtInformation, "Rate:            %.0f slots/sec", rate);
	}

    return true;
}

CDetailManager::DetailIt EDetailManager::FindDOByNameIt(LPCSTR name)
{
	for (DetailIt it=objects.begin(); it!=objects.end(); it++)
    	if (stricmp(((EDetail*)(*it))->GetName(),name)==0) return it;
    return objects.end();
}

EDetail* EDetailManager::FindDOByName(LPCSTR name)
{
	DetailIt it = FindDOByNameIt(name);
	return (it!=objects.end())?(EDetail*)*it:0;
}

bool EDetailManager::RemoveDO(LPCSTR name)
{
    DetailIt it = FindDOByNameIt(name);
    if (it!=objects.end()){
        xr_delete		(*it);
        objects.erase	(it);
        InvalidateSlots	();
    	return true;
    }else return false;
}

EDetail* EDetailManager::AppendDO(LPCSTR name, bool bTestUnique)
{
    EDetail* D=0;
	if (bTestUnique&&(0!=(D=FindDOByName(name)))) return D;

    D = xr_new<EDetail>();
    if (!D->Update(name)){
    	xr_delete(D);
        return 0;
    }
    objects.push_back	(D);
    InvalidateCache		();
	return D;
}

void EDetailManager::InvalidateSlots()
{
	int slot_cnt = dtH.size_x*dtH.size_z;
	for (int k=0; k<slot_cnt; k++){
    	DetailSlot* it = &dtSlots[k];
    	it->w_id(0,DetailSlot::ID_Empty);
    	it->w_id(1,DetailSlot::ID_Empty);
    	it->w_id(2,DetailSlot::ID_Empty);
    	it->w_id(3,DetailSlot::ID_Empty);
    }
    InvalidateCache();
}

int EDetailManager::RemoveDOs()
{
	int cnt=0;
    for (DetailIt it=objects.begin(); it!=objects.end(); it++)
        xr_delete(*it);
    cnt = objects.size();
    objects.clear();
    return cnt;
}

void EDetailManager::RemoveColorIndices(){
	m_ColorIndices.clear();
}

EDetail* EDetailManager::FindObjectInColorIndices(u32 index, LPCSTR name)
{
	ColorIndexPairIt CI=m_ColorIndices.find(index);
	if (CI!=m_ColorIndices.end()){
    	DOVec& lst = CI->second;
		for (DOIt it=lst.begin(); it!=lst.end(); it++)
    		if (stricmp((*it)->GetName(),name)==0) return *it;
    }
    return 0;
}

void EDetailManager::AppendIndexObject(u32 color,LPCSTR name, bool bTestUnique)
{
	if (bTestUnique){
		EDetail* DO = FindObjectInColorIndices(color,name);
        if (DO)
			m_ColorIndices[color].push_back(DO);
    }else{
		EDetail* DO = FindDOByName(name);
	    R_ASSERT(DO);
		m_ColorIndices[color].push_back(DO);
    }
}


