#include "stdafx.h"


#include "Blender_combine_dbg.h"

CBlender_combine_dbg::CBlender_combine_dbg	()	{	description.CLS		= 0;	}
CBlender_combine_dbg::~CBlender_combine_dbg	()	{	}
 
void	CBlender_combine_dbg::Compile(CBlender_Compile& C)
{
	IBlender::Compile		(C);

	switch (C.iElement)
	{
	case 0:	// normals only
		C.r_Pass			("null",			"combine_2_gbuf_normals",		FALSE,	FALSE,	FALSE);
		C.r_Sampler_rtf		("s_position",		r2_RT_P);
		C.r_Sampler_rtf		("s_normal",		r2_RT_N);
		C.r_Sampler_clf		("s_image",			r2_RT_generic0);
		C.r_Sampler_clf		("s_bloom",			r2_RT_bloom1);
		C.r_Sampler_clf		("s_distort",		r2_RT_generic1);
		C.r_End				();
		break;
	case 1:	// AO only
		C.r_Pass			("null",			"combine_2_gbuf_AO",		FALSE,	FALSE,	FALSE);
		C.r_Sampler_rtf		("s_position",		r2_RT_P);
		C.r_Sampler_rtf		("s_normal",		r2_RT_N);
		C.r_Sampler_clf		("s_image",			r2_RT_generic0);
		C.r_Sampler_clf		("s_bloom",			r2_RT_bloom1);
		C.r_Sampler_clf		("s_distort",		r2_RT_generic1);
		C.r_End				();
		break;
	case 2:	// Position only
		C.r_Pass			("null",			"combine_2_gbuf_position",		FALSE,	FALSE,	FALSE);
		C.r_Sampler_rtf		("s_position",		r2_RT_P);
		C.r_Sampler_rtf		("s_normal",		r2_RT_N);
		C.r_Sampler_clf		("s_image",			r2_RT_generic0);
		C.r_Sampler_clf		("s_bloom",			r2_RT_bloom1);
		C.r_Sampler_clf		("s_distort",		r2_RT_generic1);
		C.r_End				();
		break;
	case 3:	// Material only
		C.r_Pass			("null",			"combine_2_gbuf_material",		FALSE,	FALSE,	FALSE);
		C.r_Sampler_rtf		("s_position",		r2_RT_P);
		C.r_Sampler_rtf		("s_normal",		r2_RT_N);
		C.r_Sampler_clf		("s_image",			r2_RT_generic0);
		C.r_Sampler_clf		("s_bloom",			r2_RT_bloom1);
		C.r_Sampler_clf		("s_distort",		r2_RT_generic1);
		C.r_End				();
		break;
	case 4:	// Albedo
		C.r_Pass			("null",			"combine_2_gbuf_albedo",		FALSE,	FALSE,	FALSE);
		C.r_Sampler_rtf		("s_position",		r2_RT_P);
		C.r_Sampler_rtf		("s_normal",		r2_RT_N);
		C.r_Sampler_rtf		("s_diffuse",		r2_RT_albedo		);
		C.r_Sampler_clf		("s_bloom",			r2_RT_bloom1);
		C.r_Sampler_clf		("s_distort",		r2_RT_generic1);
		C.r_End				();
		break;
	case 5:	// Gloss
		C.r_Pass			("null",			"combine_2_gbuf_gloss",		FALSE,	FALSE,	FALSE);
		C.r_Sampler_rtf		("s_position",		r2_RT_P);
		C.r_Sampler_rtf		("s_normal",		r2_RT_N);
		C.r_Sampler_rtf		("s_diffuse",		r2_RT_albedo		);
		C.r_Sampler_clf		("s_bloom",			r2_RT_bloom1);
		C.r_Sampler_clf		("s_distort",		r2_RT_generic1);
		C.r_End				();
		break;
	}
}
