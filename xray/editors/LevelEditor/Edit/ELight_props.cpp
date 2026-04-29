//----------------------------------------------------
// file: ELight.cpp
//----------------------------------------------------
#include "stdafx.h"
#pragma hdrstop

#include "ELight.h"
#include "escenelighttools.h"
#include <xrEngine/LightAnimLibrary.h>
#include "../ECore/Editor/ui_main.h"
#include <addons/implot/implot.h>

//----------------------------------------------------
void 	CLight::OnAutoClick(ButtonValue* value, bool& bModif, bool& bSafe)
{
	ButtonValue* B = dynamic_cast<ButtonValue*>(value); R_ASSERT(B);
	switch(B->btn_num){
    case 0:{
        float P = 0.1f;
		m_Attenuation0 = 1.f;
        m_Attenuation1 = (m_Brightness-P-P*m_Range*m_Range*m_Attenuation2)/(P*m_Range);
        clamp(m_Attenuation1,0.f,1.f);
    }break;
	case 1:{
        float P = 0.1f;
        m_Attenuation0 = 1.f;
        m_Attenuation2 = (m_Brightness-P-P*m_Range*m_Attenuation1)/(P*m_Range*m_Range);
        clamp(m_Attenuation2,0.f,1.f);
    }
    }
    bModif = true;
	OnPointDataChange(value);
}

void 	CLight::OnFuzzyGenerateClick(ButtonValue* value, bool& bModif, bool& bSafe)
{
	ButtonValue* B = dynamic_cast<ButtonValue*>(value); R_ASSERT(B);
	switch(B->btn_num){
    case 0:{
    	OnFuzzyDataChange(value);
    }break;
    }
    bModif = true;
}

void 	CLight::OnFuzzyFlagChange(PropValue* value)
{
	if (m_Flags.is(ELight::flPointFuzzy)){ 
    	m_FuzzyData		= xr_new<SFuzzyData>();
	    OnFuzzyTypeChange	(value);
    }else{
    	xr_delete		(m_FuzzyData);
        OnTypeChange 	(value);
    }
}

void 	CLight::OnFuzzyTypeChange(PropValue* value)
{
	OnTypeChange		(value);
    OnFuzzyDataChange   (value);
}

void 	CLight::OnFuzzyDataChange(PropValue* value)
{
    VERIFY(m_FuzzyData);
	m_FuzzyData->m_Positions.resize	(m_FuzzyData->m_PointCount);
    for (FvectorIt it=m_FuzzyData->m_Positions.begin(); it!=m_FuzzyData->m_Positions.end(); it++)
    	m_FuzzyData->Generate(*it);
}

void CLight::OnPlotDraw(PlotValue* sender, void* /*user_data*/) {
	if (ImPlot::BeginPlot("##AttenuationPlot", ImVec2(-1, 220), ImPlotFlags_CanvasOnly | ImPlotFlags_NoInputs | ImPlotFlags_NoFrame | ImPlotFlags_NoMenus)) {
		ImPlot::SetupAxis(ImAxis_X1, "distance", ImPlotAxisFlags_NoLabel);
		ImPlot::SetupAxis(ImAxis_Y1, "intensity", ImPlotAxisFlags_NoLabel);

		ImPlot::SetupAxisLimits(ImAxis_X1, 0.0, m_Range * 1.1f, ImPlotCond_Always);
		ImPlot::SetupAxisLimits(ImAxis_Y1, 0.0, 1.1f, ImPlotCond_Always);

		const int			points = 300;
		std::vector<double> x(points), y(points);

		float				d_cost = m_Range / (points - 1);

		for (int i = 0; i < points; ++i) {
			float R = i * d_cost;
			float b = m_Brightness / (m_Attenuation0 + m_Attenuation1 * R + m_Attenuation2 * R * R);

			if (!fis_zero(m_Range))
				b -= m_Brightness * R / (m_Range * (m_Attenuation0 + m_Attenuation1 * m_Range + m_Attenuation2 * m_Range * m_Range));

			b	 = std::clamp(b, 0.0f, 1.0f);

			x[i] = R;
			y[i] = b;
		}

		ImPlot::PlotLine("Attenuation", x.data(), y.data(), points, ImPlotSpec());

		ImPlot::EndPlot();
	}
}

void 	CLight::OnPointDataChange(PropValue* value)
{
	ExecCommand(COMMAND_UPDATE_PROPERTIES);
}

void 	CLight::OnPointDataTestEqual(CanvasValue* a, CanvasValue* b, bool& res)
{
	CLight* A = a->GetTag<CLight*>(); VERIFY(A);
	CLight* B = b->GetTag<CLight*>(); VERIFY(B);
	res =	(fsimilar(A->m_Range,B->m_Range)&&
    		fsimilar(A->m_Attenuation0,B->m_Attenuation0)&&
    		fsimilar(A->m_Attenuation1,B->m_Attenuation1)&&
    		fsimilar(A->m_Attenuation2,B->m_Attenuation2)&&
    		fsimilar(A->m_Brightness,B->m_Brightness));
}

void CLight::FillAttProp(LPCSTR pref, PropItemVec& items) {
	PropValue* V;

	V = PHelper().CreateFloat	(items, PrepareKey(pref, "Range"),					&m_Range,		 0.1f, 1000.f);
	V->OnChangeEvent.bind		(this, &CLight::OnPointDataChange);

	V = PHelper().CreateFloat	(items, PrepareKey(pref, "Attenuation\\Constant"),	&m_Attenuation0, 0.f, 1.f, 0.0001f, 6);
	V->OnChangeEvent.bind		(this, &CLight::OnPointDataChange);

	V = PHelper().CreateFloat	(items, PrepareKey(pref, "Attenuation\\Linear"),	&m_Attenuation1, 0.f, 1.f, 0.0001f, 6);
	V->OnChangeEvent.bind		(this, &CLight::OnPointDataChange);

	V = PHelper().CreateFloat	(items, PrepareKey(pref, "Attenuation\\Quadratic"),	&m_Attenuation2, 0.f, 1.f, 0.0001f, 6);
	V->OnChangeEvent.bind		(this, &CLight::OnPointDataChange);

	ButtonValue* B = nullptr;
	B			   = PHelper().CreateButton (items, PrepareKey(pref, "Attenuation\\Auto Fit"), "Linear,Quadratic", 0);
	B->OnBtnClickEvent.bind					(this, &CLight::OnAutoClick);

	PlotValue* PV = PHelper().CreatePlot	(items, PrepareKey(pref, "Attenuation\\Graphic"), fastdelegate::MakeDelegate(this, &CLight::OnPlotDraw), "");
}

xr_token fuzzy_shape_types[]={
	{ "Sphere",			CLight::SFuzzyData::fstSphere	},
	{ "Box",			CLight::SFuzzyData::fstBox		},
	{ 0,				0				}
};
void CLight::FillPointProp(LPCSTR pref, PropItemVec& items)
{
	// flags
    PHelper().CreateFlag32(items,	PrepareKey(pref,"Usage\\LightMap"),	&m_Flags,	ELight::flAffectStatic);
    PHelper().CreateFlag32(items,	PrepareKey(pref,"Usage\\Dynamic"),	&m_Flags,	ELight::flAffectDynamic);
    PHelper().CreateFlag32(items,	PrepareKey(pref,"Usage\\Animated"),	&m_Flags,	ELight::flProcedural);
//    PHelper().CreateFlag32	(items,	PrepareKey(pref,"Flags\\Breakable"),&m_Flags,	CLight::flBreaking);

	FillAttProp			(pref,items);
    PropValue* 			P=0;
    ButtonValue* 		B=0;
    P=PHelper().CreateFlag32(items,	PrepareKey(pref, "Fuzzy"),			&m_Flags,	ELight::flPointFuzzy);
    P->OnChangeEvent.bind	(this,&CLight::OnFuzzyFlagChange);
	if (m_Flags.is(ELight::flPointFuzzy)){
        VERIFY				(m_FuzzyData);
        P=PHelper().CreateS16		(items,	PrepareKey(pref, "Fuzzy\\Points Count"),	&m_FuzzyData->m_PointCount,1,100);
        P->OnChangeEvent.bind		(this,&CLight::OnFuzzyDataChange);
	    B=PHelper().CreateButton	(items,	PrepareKey(pref, "Fuzzy\\Generate"),"Randomize",0);
    	B->OnBtnClickEvent.bind		(this,&CLight::OnFuzzyGenerateClick);
        P=PHelper().CreateToken8	(items,	PrepareKey(pref, "Fuzzy\\Shape"),			(u8*)&m_FuzzyData->m_ShapeType,	fuzzy_shape_types);
        P->OnChangeEvent.bind		(this,&CLight::OnFuzzyTypeChange);
        switch (m_FuzzyData->m_ShapeType){
        case CLight::SFuzzyData::fstSphere: 
            P=PHelper().CreateFloat(items,	PrepareKey(pref, "Fuzzy\\Radius"),			&m_FuzzyData->m_SphereRadius,0.01f,100.f,0.01f,2);
            P->OnChangeEvent.bind	(this,&CLight::OnFuzzyDataChange);
        break;
        case CLight::SFuzzyData::fstBox: 
            P=PHelper().CreateVector(items,	PrepareKey(pref, "Fuzzy\\Half Size"),		&m_FuzzyData->m_BoxDimension,0.01f,100.f,0.01f,2);
            P->OnChangeEvent.bind	(this,&CLight::OnFuzzyDataChange);
        break;
        }
    }
}
//----------------------------------------------------

void CLight::FillSpotProp(LPCSTR pref, PropItemVec& items)
{
	// flags
    PHelper().CreateFlag32(items,	PrepareKey(pref,"Usage\\LightMap"),	&m_Flags,	ELight::flAffectStatic);
    PHelper().CreateFlag32(items,	PrepareKey(pref,"Usage\\Dynamic"),	&m_Flags,	ELight::flAffectDynamic);
    PHelper().CreateFlag32(items,	PrepareKey(pref,"Usage\\Animated"),	&m_Flags,	ELight::flProcedural);
//    PHelper().CreateFlag32	(items,	PHelper().PrepareKey(pref,"Flags\\Breakable"),&m_Flags,	CLight::flBreaking);

	FillAttProp				(pref,items);
	PHelper().CreateAngle  	(items,	PrepareKey(pref, "Spot R1\\Cone Angle"),	&m_Cone,	0.1f,deg2rad(120.f),0.01f,2);
	PHelper().CreateChoose	(items,	PrepareKey(pref, "Spot R1\\Falloff Texture"),  	&m_FalloffTex, 	smTexture);
}
//----------------------------------------------------

xr_token			token_light_type[ ]	=	{
    { "Point",		ELight::ltPoint			},
//. andy    { "Spot",		ELight::ltSpot			},
    { 0,			0	  					}
};

void CLight::FillProp(LPCSTR pref, PropItemVec& items)
{
	inherited::FillProp(pref,items);

    PropValue* V=0;

//    V=PHelper().CreateToken32	(items,	PrepareKey(pref,"Type"),		(u32*)&m_Type,token_light_type);
//    V->OnChangeEvent.bind	(this,&CLight::OnTypeChange);
    V=PHelper().CreateFColor(items,	PrepareKey(pref,"Color"),			&m_Color);
	V->OnChangeEvent.bind	(this,&CLight::OnNeedUpdate);
    V=PHelper().CreateFloat	(items,	PrepareKey(pref,"Brightness"),		&m_Brightness,-3.f,3.f,0.1f,2);
    V->OnChangeEvent.bind	(this,&CLight::OnPointDataChange);

    ESceneLightTool* lt	= dynamic_cast<ESceneLightTool*>(ParentTool); VERIFY(lt);
	PHelper().CreateRToken32(items,PrepareKey(pref,"Light Control"),	&m_LControl, &*lt->lcontrols.begin(), lt->lcontrols.size());

    switch(m_Type){
    case ELight::ltPoint:	FillPointProp	(pref, items);	break;
    case ELight::ltSpot: 	FillSpotProp 	(pref, items);	break;
    default: THROW;
    }
    PHelper().CreateBOOL		(items,	PrepareKey(pref,"Use In D3D"),		&m_UseInD3D);
}

void 	CLight::OnTypeChange(PropValue* value)
{
	ExecCommand		(COMMAND_UPDATE_PROPERTIES);

	Update			();
}
//----------------------------------------------------

void CLight::OnShowHint(AStringVec& dest){
    CCustomObject::OnShowHint(dest);
    AnsiString temp("Type:  ");
    switch(m_Type){
    case ELight::ltPoint:	        temp+="point"; break;
    case ELight::ltSpot:			temp+="spot"; break;
    default: temp+="undef";
    }
    dest.push_back(temp);
    temp = "Flags: ";
    if (m_Flags.is(ELight::flAffectStatic))  	temp+="Stat ";
    if (m_Flags.is(ELight::flAffectDynamic)) 	temp+="Dyn ";
    if (m_Flags.is(ELight::flProcedural))		temp+="Proc ";
    dest.push_back(temp);
    string64 s;
    sprintf(s, "Pos:   %3.2f, %3.2f, %3.2f",GetPosition().x,GetPosition().y,GetPosition().z);
    temp = s;
    dest.push_back(temp);
}


